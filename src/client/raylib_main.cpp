#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include <raylib.h>

#include "lecon/simulation.hpp"

namespace {

constexpr int kWindowW = 1366;
constexpr int kWindowH = 768;
constexpr int kBaseTileSize = 24;
constexpr int kAtlasCell = 16;
constexpr float kStepIntervalSeconds = 0.12F;

struct BiomeWeights {
    std::array<float, 3> w{0.0F, 0.0F, 0.0F};
};

struct Particle {
    Vector2 worldPos{};
    Vector2 vel{};
    Color color{255, 255, 255, 255};
    float life = 0.0F;
    float maxLife = 1.0F;
    float size = 1.0F;
};

struct CrackInfo {
    float strength = 0.0F;
    float ttl = 0.0F;
};

enum class SpriteId : int {
    FloorACold = 0,
    FloorBCold = 1,
    WallACold = 2,
    WallBCold = 3,

    FloorAWarm = 4,
    FloorBWarm = 5,
    WallAWarm = 6,
    WallBWarm = 7,

    FloorAMoss = 8,
    FloorBMoss = 9,
    WallAMoss = 10,
    WallBMoss = 11,

    Ore = 12,
    Exit = 13,
    Player = 14,
    Mob = 15
};

enum class ScreenState {
    Menu,
    Playing
};

std::uint32_t hashU32(int x, int y, std::uint32_t salt) {
    std::uint32_t value = static_cast<std::uint32_t>(x) * 374761393u;
    value ^= static_cast<std::uint32_t>(y) * 668265263u;
    value ^= salt * 2246822519u;
    value = (value ^ (value >> 13)) * 1274126177u;
    return value ^ (value >> 16);
}

float hash01(int x, int y, std::uint32_t salt) {
    const std::uint32_t h = hashU32(x, y, salt);
    return static_cast<float>(h & 0x00FFFFFFu) / 16777215.0F;
}

std::int64_t tileKey(int x, int y) {
    const auto ux = static_cast<std::uint64_t>(static_cast<std::uint32_t>(x));
    const auto uy = static_cast<std::uint64_t>(static_cast<std::uint32_t>(y));
    return static_cast<std::int64_t>((ux << 32) | uy);
}

void putPixel(Image& image, int x, int y, Color color) {
    if(x < 0 || y < 0 || x >= image.width || y >= image.height) {
        return;
    }
    ImageDrawPixel(&image, x, y, color);
}

void fillRect(Image& image, int x, int y, int w, int h, Color color) {
    for(int py = y; py < y + h; ++py) {
        for(int px = x; px < x + w; ++px) {
            putPixel(image, px, py, color);
        }
    }
}

Rectangle spriteSource(SpriteId id) {
    const int idx = static_cast<int>(id);
    const int cols = 4;
    const int x = (idx % cols) * kAtlasCell;
    const int y = (idx / cols) * kAtlasCell;
    return Rectangle{static_cast<float>(x), static_cast<float>(y), static_cast<float>(kAtlasCell), static_cast<float>(kAtlasCell)};
}

Texture2D buildSpriteAtlas() {
    Image atlas = GenImageColor(kAtlasCell * 4, kAtlasCell * 4, BLANK);

    auto paintFloor = [&](int sx, int sy, Color baseA, Color speckA, Color speckB) {
        fillRect(atlas, sx, sy, kAtlasCell, kAtlasCell, baseA);
        for(int y = 0; y < kAtlasCell; ++y) {
            for(int x = 0; x < kAtlasCell; ++x) {
                const float n = hash01(sx + x, sy + y, 501);
                if(n > 0.90F) {
                    putPixel(atlas, sx + x, sy + y, speckA);
                } else if(n < 0.08F) {
                    putPixel(atlas, sx + x, sy + y, speckB);
                }
            }
        }
    };

    auto paintWall = [&](int sx, int sy, Color base, Color topEdge, Color bottomEdge, Color grout) {
        fillRect(atlas, sx, sy, kAtlasCell, kAtlasCell, base);

        for(int x = 0; x < kAtlasCell; ++x) {
            putPixel(atlas, sx + x, sy, topEdge);
            putPixel(atlas, sx + x, sy + kAtlasCell - 1, bottomEdge);
        }
        for(int y = 0; y < kAtlasCell; ++y) {
            putPixel(atlas, sx, sy + y, topEdge);
            putPixel(atlas, sx + kAtlasCell - 1, sy + y, bottomEdge);
        }

        const int row1 = sy + 5;
        const int row2 = sy + 10;
        for(int x = 1; x < kAtlasCell - 1; ++x) {
            putPixel(atlas, sx + x, row1, grout);
            putPixel(atlas, sx + x, row2, grout);
        }

        const int splitA = sx + 4;
        const int splitB = sx + 8;
        const int splitC = sx + 12;
        putPixel(atlas, splitA, sy + 1, grout);
        putPixel(atlas, splitA, row1 - 1, grout);
        putPixel(atlas, splitB, row1 + 1, grout);
        putPixel(atlas, splitB, row2 - 1, grout);
        putPixel(atlas, splitC, row2 + 1, grout);
        putPixel(atlas, splitC, sy + kAtlasCell - 2, grout);
    };

    paintFloor(0, 0, Color{56, 66, 86, 255}, Color{93, 109, 136, 255}, Color{39, 47, 63, 255});
    paintFloor(kAtlasCell, 0, Color{62, 73, 95, 255}, Color{97, 116, 144, 255}, Color{43, 52, 68, 255});
    paintWall(2 * kAtlasCell, 0, Color{101, 107, 124, 255}, Color{144, 151, 169, 255}, Color{70, 75, 90, 255}, Color{78, 84, 101, 255});
    paintWall(3 * kAtlasCell, 0, Color{106, 112, 129, 255}, Color{149, 157, 173, 255}, Color{74, 79, 94, 255}, Color{83, 89, 106, 255});

    paintFloor(0, kAtlasCell, Color{95, 80, 57, 255}, Color{131, 111, 77, 255}, Color{74, 60, 43, 255});
    paintFloor(kAtlasCell, kAtlasCell, Color{103, 86, 61, 255}, Color{138, 116, 81, 255}, Color{78, 64, 46, 255});
    paintWall(2 * kAtlasCell, kAtlasCell, Color{126, 96, 68, 255}, Color{170, 129, 88, 255}, Color{89, 68, 49, 255}, Color{101, 77, 55, 255});
    paintWall(3 * kAtlasCell, kAtlasCell, Color{132, 101, 72, 255}, Color{176, 134, 93, 255}, Color{94, 72, 52, 255}, Color{105, 81, 58, 255});

    paintFloor(0, 2 * kAtlasCell, Color{62, 89, 64, 255}, Color{96, 132, 98, 255}, Color{44, 65, 46, 255});
    paintFloor(kAtlasCell, 2 * kAtlasCell, Color{67, 96, 70, 255}, Color{103, 139, 106, 255}, Color{47, 70, 50, 255});
    paintWall(2 * kAtlasCell, 2 * kAtlasCell, Color{86, 116, 86, 255}, Color{121, 159, 123, 255}, Color{61, 84, 62, 255}, Color{72, 97, 73, 255});
    paintWall(3 * kAtlasCell, 2 * kAtlasCell, Color{90, 121, 90, 255}, Color{127, 165, 129, 255}, Color{65, 88, 66, 255}, Color{76, 102, 77, 255});

    paintWall(0, 3 * kAtlasCell, Color{99, 100, 110, 255}, Color{142, 144, 157, 255}, Color{70, 72, 80, 255}, Color{76, 80, 90, 255});
    for(int i = 0; i < 11; ++i) {
        const int x = 1 + static_cast<int>(hash01(i, 17, 901) * 13.0F);
        const int y = 2 + static_cast<int>(hash01(77, i, 903) * 11.0F);
        putPixel(atlas, x, 3 * kAtlasCell + y, Color{242, 186, 77, 255});
        putPixel(atlas, x + 1, 3 * kAtlasCell + y, Color{255, 224, 145, 255});
    }

    fillRect(atlas, kAtlasCell, 3 * kAtlasCell, kAtlasCell, kAtlasCell, Color{25, 66, 61, 255});
    for(int x = 0; x < kAtlasCell; ++x) {
        putPixel(atlas, kAtlasCell + x, 3 * kAtlasCell, Color{54, 108, 97, 255});
        putPixel(atlas, kAtlasCell + x, 3 * kAtlasCell + kAtlasCell - 1, Color{16, 44, 39, 255});
    }
    const int cx = kAtlasCell + 8;
    const int cy = 3 * kAtlasCell + 8;
    for(int y = -5; y <= 5; ++y) {
        for(int x = -5; x <= 5; ++x) {
            const float d = std::sqrt(static_cast<float>(x * x + y * y));
            if(d < 4.8F) {
                putPixel(atlas, cx + x, cy + y, Color{75, 231, 176, 255});
            }
            if(d < 3.2F) {
                putPixel(atlas, cx + x, cy + y, Color{152, 255, 228, 255});
            }
        }
    }

    fillRect(atlas, 2 * kAtlasCell, 3 * kAtlasCell, kAtlasCell, kAtlasCell, BLANK);
    fillRect(atlas, 2 * kAtlasCell + 6, 3 * kAtlasCell + 3, 4, 3, Color{86, 57, 36, 255});
    fillRect(atlas, 2 * kAtlasCell + 5, 3 * kAtlasCell + 6, 6, 4, Color{236, 198, 156, 255});
    fillRect(atlas, 2 * kAtlasCell + 5, 3 * kAtlasCell + 10, 6, 4, Color{79, 164, 247, 255});
    fillRect(atlas, 2 * kAtlasCell + 4, 3 * kAtlasCell + 11, 1, 2, Color{53, 122, 198, 255});
    fillRect(atlas, 2 * kAtlasCell + 11, 3 * kAtlasCell + 11, 1, 2, Color{53, 122, 198, 255});
    fillRect(atlas, 2 * kAtlasCell + 5, 3 * kAtlasCell + 14, 2, 2, Color{45, 89, 148, 255});
    fillRect(atlas, 2 * kAtlasCell + 9, 3 * kAtlasCell + 14, 2, 2, Color{45, 89, 148, 255});
    putPixel(atlas, 2 * kAtlasCell + 6, 3 * kAtlasCell + 7, BLACK);
    putPixel(atlas, 2 * kAtlasCell + 9, 3 * kAtlasCell + 7, BLACK);

    fillRect(atlas, 3 * kAtlasCell, 3 * kAtlasCell, kAtlasCell, kAtlasCell, BLANK);
    for(int y = 3; y <= 13; ++y) {
        for(int x = 2; x <= 13; ++x) {
            const float dx = static_cast<float>(x - 8) / 5.7F;
            const float dy = static_cast<float>(y - 8) / 4.7F;
            if(dx * dx + dy * dy <= 1.0F) {
                putPixel(atlas, 3 * kAtlasCell + x, 3 * kAtlasCell + y, Color{230, 103, 95, 255});
                if(y < 7) {
                    putPixel(atlas, 3 * kAtlasCell + x, 3 * kAtlasCell + y, Color{250, 142, 129, 255});
                }
            }
        }
    }
    putPixel(atlas, 3 * kAtlasCell + 6, 3 * kAtlasCell + 7, WHITE);
    putPixel(atlas, 3 * kAtlasCell + 10, 3 * kAtlasCell + 7, WHITE);
    putPixel(atlas, 3 * kAtlasCell + 6, 3 * kAtlasCell + 8, BLACK);
    putPixel(atlas, 3 * kAtlasCell + 10, 3 * kAtlasCell + 8, BLACK);

    Texture2D atlasTexture = LoadTextureFromImage(atlas);
    UnloadImage(atlas);
    SetTextureFilter(atlasTexture, TEXTURE_FILTER_POINT);
    return atlasTexture;
}

SpriteId floorVariant(int biome, int wx, int wy) {
    const bool alt = hash01(wx, wy, 1401) > 0.5F;
    if(biome == 0) {
        return alt ? SpriteId::FloorBCold : SpriteId::FloorACold;
    }
    if(biome == 1) {
        return alt ? SpriteId::FloorBWarm : SpriteId::FloorAWarm;
    }
    return alt ? SpriteId::FloorBMoss : SpriteId::FloorAMoss;
}

SpriteId wallVariant(int biome, int wx, int wy) {
    const bool alt = hash01(wx, wy, 1409) > 0.5F;
    if(biome == 0) {
        return alt ? SpriteId::WallBCold : SpriteId::WallACold;
    }
    if(biome == 1) {
        return alt ? SpriteId::WallBWarm : SpriteId::WallAWarm;
    }
    return alt ? SpriteId::WallBMoss : SpriteId::WallAMoss;
}

void drawSpriteTile(const Texture2D& atlas, SpriteId id, int px, int py, int size, Color tint) {
    const Rectangle src = spriteSource(id);
    const Rectangle dst = Rectangle{static_cast<float>(px), static_cast<float>(py), static_cast<float>(size), static_cast<float>(size)};
    DrawTexturePro(atlas, src, dst, Vector2{0.0F, 0.0F}, 0.0F, tint);
}

BiomeWeights biomeWeights(int wx, int wy) {
    const float n = hash01(wx / 8, wy / 8, 303);
    const float c0 = 0.18F;
    const float c1 = 0.50F;
    const float c2 = 0.82F;
    const float span = 0.34F;

    auto weight = [&](float c) {
        const float d = std::fabs(n - c);
        return std::max(0.0F, 1.0F - d / span);
    };

    BiomeWeights out{};
    out.w[0] = weight(c0);
    out.w[1] = weight(c1);
    out.w[2] = weight(c2);

    const float sum = out.w[0] + out.w[1] + out.w[2];
    if(sum > 0.0001F) {
        out.w[0] /= sum;
        out.w[1] /= sum;
        out.w[2] /= sum;
    } else {
        out.w = {1.0F, 0.0F, 0.0F};
    }

    return out;
}

void dominantBiomes(const BiomeWeights& bw, int& primary, int& secondary, float& secondaryWeight) {
    primary = 0;
    secondary = 1;
    float best = -1.0F;
    float second = -1.0F;

    for(int i = 0; i < 3; ++i) {
        if(bw.w[i] > best) {
            second = best;
            secondary = primary;
            best = bw.w[i];
            primary = i;
        } else if(bw.w[i] > second) {
            second = bw.w[i];
            secondary = i;
        }
    }

    secondaryWeight = std::max(0.0F, second);
}

void drawParallaxBackground(int screenW, int screenH, float t) {
    DrawRectangleGradientV(0, 0, screenW, screenH, Color{24, 30, 42, 255}, Color{10, 13, 19, 255});

    for(int i = 0; i < 6; ++i) {
        const float wave = std::sin(t * 0.3F + static_cast<float>(i) * 0.7F) * 22.0F;
        const int y = static_cast<int>(screenH * 0.24F + i * 48 + wave);
        const int h = 120 + i * 16;
        const int alpha = 24 - i * 2;
        DrawRectangle(-80, y, screenW + 160, h, Fade(Color{70, 90, 120, 255}, static_cast<float>(alpha) / 255.0F));
    }

    for(int i = 0; i < 90; ++i) {
        const int x = static_cast<int>(hash01(i, 17, 11) * static_cast<float>(screenW));
        const int y = static_cast<int>(hash01(i, 73, 11) * static_cast<float>(screenH) * 0.42F);
        const float twinkle = 0.5F + 0.5F * std::sin(t * 1.7F + static_cast<float>(i));
        DrawCircle(x, y, 1.0F + twinkle, Fade(Color{188, 210, 255, 255}, 0.25F + twinkle * 0.35F));
    }
}

void drawStyledTile(const Texture2D& atlas, lecon::TileType type, int wx, int wy, int px, int py, int tileSize, float t) {
    const BiomeWeights bw = biomeWeights(wx, wy);

    int primary = 0;
    int secondary = 1;
    float secondaryWeight = 0.0F;
    dominantBiomes(bw, primary, secondary, secondaryWeight);

    if(type == lecon::TileType::Empty) {
        drawSpriteTile(atlas, floorVariant(primary, wx, wy), px, py, tileSize, WHITE);
        const float blend = std::clamp(secondaryWeight * 1.2F - 0.15F, 0.0F, 0.65F);
        if(blend > 0.02F) {
            drawSpriteTile(atlas, floorVariant(secondary, wx + 13, wy + 7), px, py, tileSize, Fade(WHITE, blend));
        }
        return;
    }

    if(type == lecon::TileType::Wall) {
        drawSpriteTile(atlas, wallVariant(primary, wx, wy), px, py, tileSize, WHITE);
        const float blend = std::clamp(secondaryWeight * 1.2F - 0.12F, 0.0F, 0.60F);
        if(blend > 0.02F) {
            drawSpriteTile(atlas, wallVariant(secondary, wx + 3, wy + 11), px, py, tileSize, Fade(WHITE, blend));
        }
        return;
    }

    if(type == lecon::TileType::Resource) {
        drawSpriteTile(atlas, SpriteId::Ore, px, py, tileSize, WHITE);
        return;
    }

    drawSpriteTile(atlas, SpriteId::Exit, px, py, tileSize, WHITE);
    const float pulse = 0.5F + 0.5F * std::sin(t * 3.0F + static_cast<float>((wx + wy) % 9));
    const int cx = px + tileSize / 2;
    const int cy = py + tileSize / 2;
    DrawCircleGradient(
        cx,
        cy,
        static_cast<float>(tileSize) * (0.45F + pulse * 0.12F),
        Fade(Color{151, 255, 219, 220}, 0.45F + pulse * 0.4F),
        Fade(Color{151, 255, 219, 0}, 0.0F)
    );
}

lecon::Action actionFromInput() {
    if(IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        return lecon::Action::MoveUp;
    }
    if(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        return lecon::Action::MoveDown;
    }
    if(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        return lecon::Action::MoveLeft;
    }
    if(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        return lecon::Action::MoveRight;
    }
    if(IsKeyDown(KEY_Z)) {
        return lecon::Action::Mine;
    }
    if(IsKeyDown(KEY_X)) {
        return lecon::Action::Place;
    }
    if(IsKeyDown(KEY_C)) {
        return lecon::Action::Use;
    }

    return lecon::Action::Wait;
}

void spawnParticles(
    std::vector<Particle>& particles,
    Vector2 world,
    int count,
    Color color,
    float speed,
    float life,
    float size,
    int salt
) {
    for(int i = 0; i < count; ++i) {
        const float ang = hash01(i, salt, 1701) * 6.2831853F;
        const float mag = speed * (0.35F + hash01(salt, i, 1703) * 0.9F);
        Particle p;
        p.worldPos = world;
        p.vel = Vector2{std::cos(ang) * mag, std::sin(ang) * mag};
        p.color = color;
        p.maxLife = life * (0.8F + hash01(i, salt, 1709) * 0.45F);
        p.life = p.maxLife;
        p.size = size * (0.75F + hash01(salt, i, 1717) * 0.5F);
        particles.push_back(p);
    }
}

void updateParticles(std::vector<Particle>& particles, float dt) {
    for(auto& p : particles) {
        p.life -= dt;
        p.worldPos.x += p.vel.x * dt;
        p.worldPos.y += p.vel.y * dt;
        p.vel.y += 0.8F * dt;
        p.vel.x *= 0.98F;
    }

    particles.erase(
        std::remove_if(
            particles.begin(),
            particles.end(),
            [](const Particle& p) { return p.life <= 0.0F; }
        ),
        particles.end()
    );
}

void drawParticles(const std::vector<Particle>& particles, const lecon::Vec2i& player, int centerX, int centerY, int tileSize) {
    for(const auto& p : particles) {
        const float lifeNorm = std::clamp(p.life / std::max(0.001F, p.maxLife), 0.0F, 1.0F);
        const int sx = centerX + static_cast<int>((p.worldPos.x - static_cast<float>(player.x)) * static_cast<float>(tileSize));
        const int sy = centerY + static_cast<int>((p.worldPos.y - static_cast<float>(player.y)) * static_cast<float>(tileSize));
        const float r = std::max(1.0F, p.size * (static_cast<float>(tileSize) * 0.12F) * lifeNorm);
        DrawCircle(sx, sy, r, Fade(p.color, 0.1F + lifeNorm * 0.9F));
    }
}

void updateCracks(std::unordered_map<std::int64_t, CrackInfo>& cracks, float dt) {
    for(auto it = cracks.begin(); it != cracks.end();) {
        it->second.ttl -= dt;
        it->second.strength = std::max(0.0F, it->second.strength - dt * 0.25F);
        if(it->second.ttl <= 0.0F || it->second.strength <= 0.01F) {
            it = cracks.erase(it);
        } else {
            ++it;
        }
    }
}

void drawCracks(
    const std::unordered_map<std::int64_t, CrackInfo>& cracks,
    const lecon::Vec2i& player,
    int centerX,
    int centerY,
    int tileSize,
    int viewRadiusX,
    int viewRadiusY,
    float t
) {
    for(const auto& [key, info] : cracks) {
        const int wx = static_cast<int>(static_cast<std::uint32_t>((static_cast<std::uint64_t>(key) >> 32) & 0xFFFFFFFFu));
        const int wy = static_cast<int>(static_cast<std::uint32_t>(static_cast<std::uint64_t>(key) & 0xFFFFFFFFu));

        const int dx = wx - player.x;
        const int dy = wy - player.y;
        if(dx < -viewRadiusX - 1 || dx > viewRadiusX + 1 || dy < -viewRadiusY - 1 || dy > viewRadiusY + 1) {
            continue;
        }

        const int px = centerX + dx * tileSize;
        const int py = centerY + dy * tileSize;

        const float alpha = std::clamp(info.strength, 0.0F, 1.0F);
        const Color c = Fade(Color{33, 26, 26, 255}, 0.22F + alpha * 0.65F);

        DrawLine(px + tileSize / 3, py + 2, px + tileSize / 2, py + tileSize - 3, c);
        DrawLine(px + tileSize / 2, py + tileSize / 2, px + tileSize - 3, py + tileSize - 4, c);
        DrawLine(px + 3, py + tileSize / 2, px + tileSize / 2, py + tileSize / 3, c);

        const int jitter = static_cast<int>(std::sin(t * 9.0F + static_cast<float>(wx + wy)) * 1.2F);
        DrawLine(px + 5, py + 3 + jitter, px + tileSize - 6, py + tileSize - 6 - jitter, Fade(c, 0.65F));
    }
}

void drawMobSprite(const Texture2D& atlas, int px, int py, int tileSize, float t, int idx) {
    const float wobble = std::sin(t * 5.2F + static_cast<float>(idx)) * 1.5F;
    DrawEllipse(px + tileSize / 2, py + static_cast<int>(tileSize * 0.88F), tileSize * 0.25F, tileSize * 0.10F, Fade(BLACK, 0.3F));
    drawSpriteTile(atlas, SpriteId::Mob, px, py + static_cast<int>(wobble), tileSize, WHITE);
}

void drawPlayerSprite(const Texture2D& atlas, int px, int py, int tileSize, float t) {
    const int bob = static_cast<int>(std::sin(t * 7.0F) * 1.6F);
    DrawEllipse(px + tileSize / 2, py + static_cast<int>(tileSize * 0.90F), tileSize * 0.28F, tileSize * 0.11F, Fade(BLACK, 0.35F));
    drawSpriteTile(atlas, SpriteId::Player, px, py + bob, tileSize, WHITE);
}

bool drawButton(Rectangle rect, const char* text, bool enabled) {
    const Vector2 mouse = GetMousePosition();
    const bool hover = enabled && CheckCollisionPointRec(mouse, rect);
    const Color bg = !enabled ? Color{52, 56, 66, 255} : (hover ? Color{82, 133, 207, 255} : Color{66, 84, 115, 255});

    DrawRectangleRounded(rect, 0.25F, 8, bg);
    DrawRectangleRoundedLinesEx(rect, 0.25F, 8, 2.0F, enabled ? Color{178, 205, 243, 255} : Color{102, 112, 130, 255});

    const int tw = MeasureText(text, 24);
    const int tx = static_cast<int>(rect.x + (rect.width - static_cast<float>(tw)) * 0.5F);
    const int ty = static_cast<int>(rect.y + (rect.height - 24.0F) * 0.5F);
    DrawText(text, tx, ty, 24, enabled ? WHITE : Color{140, 146, 160, 255});

    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void drawHud(const lecon::Simulation& sim, int screenH, int tileSize) {
    const Rectangle panel = {18.0F, 18.0F, 420.0F, 124.0F};
    DrawRectangleRounded(panel, 0.2F, 8, Fade(Color{18, 21, 27, 255}, 0.85F));
    DrawRectangleRoundedLinesEx(panel, 0.2F, 8, 2.0F, Color{60, 68, 82, 255});

    DrawText(TextFormat("HP: %d", sim.hp()), 34, 36, 24, Color{225, 80, 80, 255});
    DrawText(TextFormat("Energy: %d", sim.energy()), 34, 66, 24, Color{95, 179, 255, 255});
    DrawText(TextFormat("Inventory: %d", sim.inventory()), 34, 96, 24, Color{236, 198, 102, 255});

    DrawText(TextFormat("Tile: %dpx", tileSize), 298, 36, 20, Color{210, 220, 235, 255});
    DrawText("Biome transitions + FX", 298, 66, 18, Color{172, 225, 196, 255});

    const std::string info = "WASD/Arrows move | Z mine | X place | C use | R reset | ESC menu | Mouse wheel zoom";
    DrawText(info.c_str(), 18, screenH - 34, 18, Color{196, 206, 220, 255});
}

std::uint64_t parseSeed(const std::string& text, std::uint64_t fallback) {
    if(text.empty()) {
        return fallback;
    }

    std::uint64_t value = 0;
    for(char c : text) {
        if(c < '0' || c > '9') {
            return fallback;
        }
        const std::uint64_t digit = static_cast<std::uint64_t>(c - '0');
        if(value > (std::numeric_limits<std::uint64_t>::max() - digit) / 10ULL) {
            return fallback;
        }
        value = value * 10ULL + digit;
    }
    return value;
}

}  // namespace

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(kWindowW, kWindowH, "Lecon 2D - raylib biome client");
    SetTargetFPS(60);

    lecon::Simulation sim;
    const Texture2D atlas = buildSpriteAtlas();

    ScreenState screenState = ScreenState::Menu;

    std::string seedInput = "42";
    std::uint64_t currentSeed = 42;
    bool hasRun = false;

    float stepTimer = 0.0F;
    float zoom = 1.0F;

    std::vector<Particle> particles;
    std::unordered_map<std::int64_t, CrackInfo> cracks;

    lecon::Vec2i facing{1, 0};
    float hitFlash = 0.0F;

    while(!WindowShouldClose()) {
        const int screenW = GetScreenWidth();
        const int screenH = GetScreenHeight();
        const float t = static_cast<float>(GetTime());
        const float dt = GetFrameTime();

        updateParticles(particles, dt);
        updateCracks(cracks, dt);
        hitFlash = std::max(0.0F, hitFlash - dt * 2.0F);

        zoom += GetMouseWheelMove() * 0.08F;
        zoom = std::clamp(zoom, 0.70F, 1.70F);
        const int tileSize = std::clamp(static_cast<int>(std::round(static_cast<float>(kBaseTileSize) * zoom)), 16, 48);

        if(screenState == ScreenState::Menu) {
            int ch = GetCharPressed();
            while(ch > 0) {
                if(ch >= '0' && ch <= '9' && seedInput.size() < 20) {
                    seedInput.push_back(static_cast<char>(ch));
                }
                ch = GetCharPressed();
            }
            if(IsKeyPressed(KEY_BACKSPACE) && !seedInput.empty()) {
                seedInput.pop_back();
            }

            const Rectangle newRunBtn = {static_cast<float>(screenW / 2 - 150), static_cast<float>(screenH / 2 + 38), 300.0F, 52.0F};
            const Rectangle continueBtn = {static_cast<float>(screenW / 2 - 150), static_cast<float>(screenH / 2 + 102), 300.0F, 52.0F};

            BeginDrawing();
            drawParallaxBackground(screenW, screenH, t);

            DrawRectangle(0, 0, screenW, screenH, Fade(Color{8, 10, 14, 255}, 0.5F));

            DrawText("LECON 2D", screenW / 2 - 130, screenH / 2 - 190, 58, Color{220, 235, 255, 255});
            DrawText("Biome transitions | RL-ready world", screenW / 2 - 194, screenH / 2 - 132, 26, Color{170, 214, 186, 255});

            const Rectangle inputBox = {static_cast<float>(screenW / 2 - 150), static_cast<float>(screenH / 2 - 34), 300.0F, 52.0F};
            DrawRectangleRounded(inputBox, 0.18F, 6, Color{24, 29, 40, 255});
            DrawRectangleRoundedLinesEx(inputBox, 0.18F, 6, 2.0F, Color{92, 118, 166, 255});

            DrawText("Seed", screenW / 2 - 150, screenH / 2 - 68, 22, Color{188, 200, 220, 255});
            DrawText(seedInput.empty() ? "0" : seedInput.c_str(), screenW / 2 - 132, screenH / 2 - 20, 30, WHITE);

            const bool clickNew = drawButton(newRunBtn, "New Run", true);
            const bool clickContinue = drawButton(continueBtn, "Continue", hasRun);

            DrawText("Enter = New Run | C = Continue", screenW / 2 - 188, screenH / 2 + 170, 22, Color{198, 206, 218, 255});
            DrawText("Use only digits for seed", screenW / 2 - 128, screenH / 2 + 200, 18, Color{154, 164, 180, 255});

            EndDrawing();

            const bool startNew = clickNew || IsKeyPressed(KEY_ENTER);
            const bool resume = clickContinue || (hasRun && IsKeyPressed(KEY_C));

            if(startNew) {
                currentSeed = parseSeed(seedInput, currentSeed);
                sim.reset(currentSeed);
                particles.clear();
                cracks.clear();
                facing = {1, 0};
                hasRun = true;
                screenState = ScreenState::Playing;
            } else if(resume) {
                screenState = ScreenState::Playing;
            }

            continue;
        }

        if(IsKeyPressed(KEY_ESCAPE)) {
            screenState = ScreenState::Menu;
        }

        if(IsKeyPressed(KEY_R)) {
            sim.reset(currentSeed);
            particles.clear();
            cracks.clear();
            facing = {1, 0};
        }

        stepTimer += dt;
        if(stepTimer >= kStepIntervalSeconds) {
            stepTimer = 0.0F;
            if(!sim.done()) {
                const lecon::Action action = actionFromInput();
                const lecon::Vec2i playerBefore = sim.playerPos();
                const int hpBefore = sim.hp();

                if(action == lecon::Action::MoveUp) {
                    facing = {0, -1};
                } else if(action == lecon::Action::MoveDown) {
                    facing = {0, 1};
                } else if(action == lecon::Action::MoveLeft) {
                    facing = {-1, 0};
                } else if(action == lecon::Action::MoveRight) {
                    facing = {1, 0};
                }

                lecon::TileType beforeMineTile = lecon::TileType::Empty;
                lecon::Vec2i mineTarget{playerBefore.x + facing.x, playerBefore.y + facing.y};
                if(action == lecon::Action::Mine) {
                    beforeMineTile = sim.tileAt(mineTarget.x, mineTarget.y);
                }

                sim.step(action);

                if(action == lecon::Action::Mine && beforeMineTile == lecon::TileType::Resource) {
                    const lecon::TileType afterMineTile = sim.tileAt(mineTarget.x, mineTarget.y);
                    const Vector2 dustPos{
                        static_cast<float>(mineTarget.x) + 0.5F,
                        static_cast<float>(mineTarget.y) + 0.5F
                    };

                    auto& crack = cracks[tileKey(mineTarget.x, mineTarget.y)];
                    crack.strength = std::min(1.0F, crack.strength + 0.32F);
                    crack.ttl = 1.2F;

                    spawnParticles(particles, dustPos, 6, Color{238, 194, 109, 255}, 1.3F, 0.45F, 0.9F, mineTarget.x ^ mineTarget.y);

                    if(afterMineTile != lecon::TileType::Resource) {
                        cracks.erase(tileKey(mineTarget.x, mineTarget.y));
                        spawnParticles(particles, dustPos, 18, Color{255, 223, 145, 255}, 2.4F, 0.7F, 1.15F, mineTarget.x * 31 + mineTarget.y * 17);
                    }
                }

                const int damage = hpBefore - sim.hp();
                if(damage > 0) {
                    const Vector2 hitPos{
                        static_cast<float>(sim.playerPos().x) + 0.5F,
                        static_cast<float>(sim.playerPos().y) + 0.45F
                    };
                    spawnParticles(particles, hitPos, 10 + damage * 4, Color{250, 93, 90, 255}, 2.0F, 0.55F, 1.0F, hpBefore * 13 + sim.steps());
                    hitFlash = std::min(1.0F, hitFlash + static_cast<float>(damage) * 0.35F);
                 }
             }
         }

        std::string title = "Lecon 2D | Seed=" + std::to_string(currentSeed) +
                            " HP=" + std::to_string(sim.hp()) +
                            " Energy=" + std::to_string(sim.energy()) +
                            " Inv=" + std::to_string(sim.inventory()) +
                            " Steps=" + std::to_string(sim.steps());
        if(sim.done()) {
            title += " | Episode done (R reset, ESC menu)";
        }
        SetWindowTitle(title.c_str());

        const lecon::Vec2i player = sim.playerPos();
        const lecon::Vec2i exit = sim.exitPos();

        const int viewRadiusX = std::max(8, screenW / (2 * tileSize) - 1);
        const int viewRadiusY = std::max(6, screenH / (2 * tileSize) - 2);

        const int centerX = screenW / 2;
        const int centerY = screenH / 2;

        BeginDrawing();
        drawParallaxBackground(screenW, screenH, t);

        for(int vy = -viewRadiusY; vy <= viewRadiusY; ++vy) {
            for(int vx = -viewRadiusX; vx <= viewRadiusX; ++vx) {
                const int wx = player.x + vx;
                const int wy = player.y + vy;

                const int px = centerX + vx * tileSize;
                const int py = centerY + vy * tileSize;

                drawStyledTile(atlas, sim.tileAt(wx, wy), wx, wy, px, py, tileSize, t);
            }
        }

        drawCracks(cracks, player, centerX, centerY, tileSize, viewRadiusX, viewRadiusY, t);

        const float dayNight = 0.75F + 0.25F * std::sin(t * 0.11F);
        DrawRectangle(0, 0, screenW, screenH, Fade(Color{12, 15, 24, 255}, 1.0F - dayNight));

        const int ex = centerX + (exit.x - player.x) * tileSize + tileSize / 2;
        const int ey = centerY + (exit.y - player.y) * tileSize + tileSize / 2;
        DrawCircleGradient(ex, ey, static_cast<float>(tileSize) * 1.2F, Fade(Color{90, 255, 180, 255}, 0.4F), Fade(Color{90, 255, 180, 0}, 0.0F));

        int mobIdx = 0;
        for(const auto& mob : sim.mobs()) {
            const int mx = centerX + (mob.pos.x - player.x) * tileSize;
            const int my = centerY + (mob.pos.y - player.y) * tileSize;
            drawMobSprite(atlas, mx, my, tileSize, t, mobIdx);
            ++mobIdx;
        }

        drawPlayerSprite(atlas, centerX, centerY, tileSize, t);
        drawParticles(particles, player, centerX, centerY, tileSize);

        if(hitFlash > 0.01F) {
            DrawRectangle(0, 0, screenW, screenH, Fade(Color{255, 60, 60, 255}, std::clamp(hitFlash, 0.0F, 0.35F)));
        }

        drawHud(sim, screenH, tileSize);

        EndDrawing();
    }

    UnloadTexture(atlas);
    CloseWindow();
    return 0;
}
