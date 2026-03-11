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
    Mob = 15,
    Tree = 16,
    Workbench = 17
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
    Image atlas = GenImageColor(kAtlasCell * 4, kAtlasCell * 5, BLANK);

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

    fillRect(atlas, 0, 4 * kAtlasCell, kAtlasCell, kAtlasCell, BLANK);
    fillRect(atlas, 6, 4 * kAtlasCell + 9, 4, 6, Color{121, 83, 49, 255});
    fillRect(atlas, 5, 4 * kAtlasCell + 8, 6, 1, Color{142, 98, 56, 255});
    fillRect(atlas, 7, 4 * kAtlasCell + 6, 2, 2, Color{154, 107, 63, 255});

    for(int y = 1; y <= 10; ++y) {
        for(int x = 1; x <= 14; ++x) {
            const float dx = static_cast<float>(x - 8) / 6.0F;
            const float dy = static_cast<float>(y - 5) / 4.8F;
            if(dx * dx + dy * dy <= 1.0F) {
                const bool highlight = y < 4;
                putPixel(atlas, x, 4 * kAtlasCell + y, highlight ? Color{121, 179, 109, 255} : Color{86, 143, 76, 255});
            }
        }
    }
    for(int i = 0; i < 14; ++i) {
        const int tx = static_cast<int>(hash01(i, 99, 1901) * 14.0F) + 1;
        const int ty = static_cast<int>(hash01(77, i, 1903) * 9.0F) + 1;
        putPixel(atlas, tx, 4 * kAtlasCell + ty, Color{67, 110, 59, 255});
    }

    fillRect(atlas, kAtlasCell, 4 * kAtlasCell, kAtlasCell, kAtlasCell, BLANK);
    fillRect(atlas, kAtlasCell + 2, 4 * kAtlasCell + 4, 12, 9, Color{116, 79, 52, 255});
    fillRect(atlas, kAtlasCell + 1, 4 * kAtlasCell + 3, 14, 2, Color{149, 103, 67, 255});
    fillRect(atlas, kAtlasCell + 3, 4 * kAtlasCell + 6, 10, 1, Color{88, 58, 38, 255});
    fillRect(atlas, kAtlasCell + 3, 4 * kAtlasCell + 9, 10, 1, Color{88, 58, 38, 255});
    fillRect(atlas, kAtlasCell + 3, 4 * kAtlasCell + 13, 2, 3, Color{92, 61, 41, 255});
    fillRect(atlas, kAtlasCell + 11, 4 * kAtlasCell + 13, 2, 3, Color{92, 61, 41, 255});

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

    if(type == lecon::TileType::Tree) {
        drawSpriteTile(atlas, floorVariant(primary, wx, wy), px, py, tileSize, WHITE);
        const Color treeTint = (primary == 1)
                                   ? Color{236, 229, 207, 255}
                                   : (primary == 2 ? Color{210, 245, 214, 255} : Color{214, 228, 244, 255});
        drawSpriteTile(atlas, SpriteId::Tree, px, py, tileSize, treeTint);
        return;
    }

    if(type == lecon::TileType::Workbench) {
        drawSpriteTile(atlas, floorVariant(primary, wx, wy), px, py, tileSize, WHITE);
        drawSpriteTile(atlas, SpriteId::Workbench, px, py, tileSize, WHITE);
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

const char* itemGlyph(lecon::ItemId item);
Color itemTint(lecon::ItemId item);

void drawMobSprite(const Texture2D& atlas, int px, int py, int tileSize, float t, int idx) {
    const float wobble = std::sin(t * 5.2F + static_cast<float>(idx)) * 1.5F;
    DrawEllipse(px + tileSize / 2, py + static_cast<int>(tileSize * 0.88F), tileSize * 0.25F, tileSize * 0.10F, Fade(BLACK, 0.3F));
    drawSpriteTile(atlas, SpriteId::Mob, px, py + static_cast<int>(wobble), tileSize, WHITE);
}

void drawPlayerSprite(const Texture2D& atlas, int px, int py, int tileSize, float t, const lecon::Vec2i& facing, lecon::ItemId heldItem) {
    const int bob = static_cast<int>(std::sin(t * 7.0F) * 1.6F);
    DrawEllipse(px + tileSize / 2, py + static_cast<int>(tileSize * 0.90F), tileSize * 0.28F, tileSize * 0.11F, Fade(BLACK, 0.35F));
    drawSpriteTile(atlas, SpriteId::Player, px, py + bob, tileSize, WHITE);

    if(heldItem != lecon::ItemId::None) {
        const int handOffsetX = facing.x * (tileSize / 3);
        const int handOffsetY = facing.y * (tileSize / 3);
        const int hx = px + tileSize / 2 + handOffsetX;
        const int hy = py + tileSize / 2 + bob + handOffsetY;
        const int s = std::max(8, tileSize / 3);
        DrawRectangleRounded(
            Rectangle{static_cast<float>(hx - s / 2), static_cast<float>(hy - s / 2), static_cast<float>(s), static_cast<float>(s)},
            0.25F,
            4,
            itemTint(heldItem)
        );
        const char* glyph = itemGlyph(heldItem);
        DrawText(glyph, hx - 4, hy - 6, 12, WHITE);
    }
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

struct RecipeEntry {
    lecon::RecipeId recipe;
    const char* keyHint;
    const char* name;
    const char* cost;
    bool requiresWorkbench;
};

const char* itemShortLabel(lecon::ItemId item) {
    switch(item) {
        case lecon::ItemId::Wood:
            return "Wood";
        case lecon::ItemId::Planks:
            return "Plank";
        case lecon::ItemId::Sticks:
            return "Stick";
        case lecon::ItemId::Ore:
            return "Ore";
        case lecon::ItemId::WorkbenchKit:
            return "Bench";
        default:
            return "-";
    }
}

const char* itemGlyph(lecon::ItemId item) {
    switch(item) {
        case lecon::ItemId::Wood:
            return "W";
        case lecon::ItemId::Planks:
            return "P";
        case lecon::ItemId::Sticks:
            return "S";
        case lecon::ItemId::Ore:
            return "O";
        case lecon::ItemId::WorkbenchKit:
            return "B";
        default:
            return "";
    }
}

Color itemTint(lecon::ItemId item) {
    switch(item) {
        case lecon::ItemId::Wood:
            return Color{156, 109, 71, 255};
        case lecon::ItemId::Planks:
            return Color{187, 141, 97, 255};
        case lecon::ItemId::Sticks:
            return Color{136, 99, 72, 255};
        case lecon::ItemId::Ore:
            return Color{185, 162, 93, 255};
        case lecon::ItemId::WorkbenchKit:
            return Color{130, 95, 70, 255};
        default:
            return Color{80, 86, 100, 255};
    }
}

constexpr std::array<RecipeEntry, 7> kRecipes{ {
    {lecon::RecipeId::Planks, "[F1]", "Planks x4", "1 Wood", false},
    {lecon::RecipeId::Sticks, "[F2]", "Sticks x4", "2 Planks", false},
    {lecon::RecipeId::Workbench, "[F3]", "Workbench Kit x1", "10 Planks", false},
    {lecon::RecipeId::AxeTier1, "[F4]", "Axe Lv1", "3 Planks + 2 Sticks", true},
    {lecon::RecipeId::PickaxeTier1, "[F5]", "Pickaxe Lv1", "3 Planks + 2 Sticks", true},
    {lecon::RecipeId::AxeTier2, "[F6]", "Axe Lv2", "3 Ore + 2 Sticks", true},
    {lecon::RecipeId::PickaxeTier2, "[F7]", "Pickaxe Lv2", "3 Ore + 2 Sticks", true},
} };

void drawHud(const lecon::Simulation& sim, int screenH, int tileSize, bool inventoryOpen, bool nearWorkbench) {
    const Rectangle panel = {18.0F, 18.0F, 560.0F, 188.0F};
    DrawRectangleRounded(panel, 0.2F, 8, Fade(Color{18, 21, 27, 255}, 0.85F));
    DrawRectangleRoundedLinesEx(panel, 0.2F, 8, 2.0F, Color{60, 68, 82, 255});

    DrawText(TextFormat("HP: %d", sim.hp()), 34, 36, 24, Color{225, 80, 80, 255});
    DrawText(TextFormat("Energy: %d", sim.energy()), 34, 66, 24, Color{95, 179, 255, 255});
    DrawText(TextFormat("Wood: %d", sim.wood()), 34, 96, 20, Color{201, 156, 94, 255});
    DrawText(TextFormat("Planks: %d", sim.planks()), 34, 120, 20, Color{216, 179, 129, 255});
    DrawText(TextFormat("Sticks: %d", sim.sticks()), 34, 144, 20, Color{190, 150, 108, 255});
    DrawText(TextFormat("Ore: %d", sim.ore()), 220, 96, 20, Color{236, 198, 102, 255});
    DrawText(TextFormat("Workbench Kits: %d", sim.workbenches()), 220, 120, 20, Color{190, 165, 127, 255});
    DrawText(TextFormat("Inventory Total: %d", sim.inventory()), 220, 144, 20, Color{216, 226, 241, 255});

    DrawText(TextFormat("Tile: %dpx", tileSize), 402, 36, 20, Color{210, 220, 235, 255});
    DrawText(TextFormat("Axe Lvl: %d", sim.axeLevel()), 402, 66, 20, Color{201, 156, 94, 255});
    DrawText(TextFormat("Pickaxe Lvl: %d", sim.pickaxeLevel()), 402, 92, 20, Color{236, 198, 102, 255});
    DrawText(TextFormat("Mine Range: %.2f", sim.miningRangeTiles()), 402, 118, 18, Color{178, 209, 245, 255});
    DrawText(nearWorkbench ? "Workbench nearby: YES" : "Workbench nearby: NO", 402, 142, 18, nearWorkbench ? Color{149, 232, 166, 255} : Color{233, 145, 136, 255});
    DrawText(inventoryOpen ? "Inventory: OPEN (TAB)" : "Inventory: CLOSED (TAB)", 402, 164, 16, Color{190, 212, 241, 255});
    DrawText(TextFormat("Hotbar Slot: %d", sim.hotbarSelection() + 1), 402, 182, 16, Color{244, 223, 156, 255});

    const std::string info = "WASD move | Mouse mine | RMB tile: context place/use | 1-9 hotbar | X/C use selected slot | F1-F7 craft | TAB inventory";
    DrawText(info.c_str(), 18, screenH - 56, 18, Color{196, 206, 220, 255});
}

void drawHotbar(const lecon::Simulation& sim, int screenW, int screenH) {
    constexpr int kSlotSize = 52;
    constexpr int kGap = 8;
    const int totalW = lecon::Simulation::kHotbarSlotCount * kSlotSize + (lecon::Simulation::kHotbarSlotCount - 1) * kGap;
    const int startX = (screenW - totalW) / 2;
    const int y = screenH - 108;

    DrawRectangleRounded(
        Rectangle{static_cast<float>(startX - 14), static_cast<float>(y - 10), static_cast<float>(totalW + 28), 72.0F},
        0.2F,
        8,
        Fade(Color{13, 17, 24, 255}, 0.88F)
    );

    for(int i = 0; i < lecon::Simulation::kHotbarSlotCount; ++i) {
        const int x = startX + i * (kSlotSize + kGap);
        const Rectangle rect{static_cast<float>(x), static_cast<float>(y), static_cast<float>(kSlotSize), static_cast<float>(kSlotSize)};

        const bool active = i == sim.hotbarSelection();
        DrawRectangleRounded(rect, 0.18F, 6, active ? Color{60, 82, 109, 255} : Color{34, 44, 58, 255});
        DrawRectangleRoundedLinesEx(rect, 0.18F, 6, active ? 3.0F : 2.0F, active ? Color{244, 223, 156, 255} : Color{90, 105, 126, 255});

        const auto slot = sim.hotbarSlot(i);
        if(slot.item != lecon::ItemId::None && slot.count > 0) {
            const Rectangle inner{rect.x + 6.0F, rect.y + 6.0F, rect.width - 12.0F, rect.height - 12.0F};
            DrawRectangleRounded(inner, 0.18F, 4, itemTint(slot.item));
            const char* glyph = itemGlyph(slot.item);
            const int glyphW = MeasureText(glyph, 24);
            DrawText(glyph, static_cast<int>(rect.x + (rect.width - static_cast<float>(glyphW)) * 0.5F), static_cast<int>(rect.y + 10.0F), 24, WHITE);
            DrawText(TextFormat("%d", slot.count), static_cast<int>(rect.x + 7.0F), static_cast<int>(rect.y + rect.height - 17.0F), 15, Color{241, 247, 255, 255});
        }

        DrawText(TextFormat("%d", i + 1), static_cast<int>(rect.x + rect.width - 14.0F), static_cast<int>(rect.y + 4.0F), 12, Color{180, 192, 210, 255});
    }
}

void drawInventoryPanel(lecon::Simulation& sim, int screenW, bool nearWorkbench, int selectedHotbarSlot, int& dragSourceSlot, bool& dragSplitMode) {
    const Rectangle panel = {static_cast<float>(screenW - 496), 18.0F, 474.0F, 560.0F};
    DrawRectangleRounded(panel, 0.16F, 8, Fade(Color{14, 18, 23, 255}, 0.92F));
    DrawRectangleRoundedLinesEx(panel, 0.16F, 8, 2.0F, Color{72, 84, 100, 255});

    DrawText("Inventory + Crafting", static_cast<int>(panel.x) + 18, static_cast<int>(panel.y) + 14, 28, Color{224, 237, 255, 255});
    DrawText(nearWorkbench ? "Workbench in range" : "No workbench nearby", static_cast<int>(panel.x) + 18, static_cast<int>(panel.y) + 50, 18, nearWorkbench ? Color{149, 232, 166, 255} : Color{233, 145, 136, 255});

    DrawText(TextFormat("Wood: %d", sim.wood()), static_cast<int>(panel.x) + 18, static_cast<int>(panel.y) + 78, 18, Color{201, 156, 94, 255});
    DrawText(TextFormat("Planks: %d", sim.planks()), static_cast<int>(panel.x) + 18, static_cast<int>(panel.y) + 100, 18, Color{216, 179, 129, 255});
    DrawText(TextFormat("Sticks: %d", sim.sticks()), static_cast<int>(panel.x) + 18, static_cast<int>(panel.y) + 122, 18, Color{190, 150, 108, 255});
    DrawText(TextFormat("Ore: %d", sim.ore()), static_cast<int>(panel.x) + 160, static_cast<int>(panel.y) + 78, 18, Color{236, 198, 102, 255});
    DrawText(TextFormat("Bench Kits: %d", sim.workbenches()), static_cast<int>(panel.x) + 160, static_cast<int>(panel.y) + 100, 18, Color{190, 165, 127, 255});
    DrawText(TextFormat("Slots: %d", sim.inventorySlotCount()), static_cast<int>(panel.x) + 160, static_cast<int>(panel.y) + 122, 18, Color{170, 183, 202, 255});

    constexpr int kCols = 8;
    constexpr int kRows = 3;
    constexpr int kSlotSize = 46;
    constexpr int kSlotGap = 8;

    const int gridX = static_cast<int>(panel.x) + 18;
    const int gridY = static_cast<int>(panel.y) + 154;

    std::array<Rectangle, lecon::Simulation::kInventorySlotCount> slotRects{};
    int hoveredSlot = -1;
    const Vector2 mouse = GetMousePosition();

    for(int row = 0; row < kRows; ++row) {
        for(int col = 0; col < kCols; ++col) {
            const int idx = row * kCols + col;
            const float sx = static_cast<float>(gridX + col * (kSlotSize + kSlotGap));
            const float sy = static_cast<float>(gridY + row * (kSlotSize + kSlotGap));
            const Rectangle rect{sx, sy, static_cast<float>(kSlotSize), static_cast<float>(kSlotSize)};
            slotRects[static_cast<std::size_t>(idx)] = rect;
            if(CheckCollisionPointRec(mouse, rect)) {
                hoveredSlot = idx;
            }
        }
    }

    if(dragSourceSlot < 0) {
        if(hoveredSlot >= 0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            const auto slot = sim.inventorySlot(hoveredSlot);
            if(slot.item != lecon::ItemId::None && slot.count > 0) {
                dragSourceSlot = hoveredSlot;
                dragSplitMode = false;
            }
        } else if(hoveredSlot >= 0 && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            const auto slot = sim.inventorySlot(hoveredSlot);
            if(slot.item != lecon::ItemId::None && slot.count > 1) {
                dragSourceSlot = hoveredSlot;
                dragSplitMode = true;
            }
        }
    } else {
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
            const bool splitDrop = dragSplitMode || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT);
            if(hoveredSlot >= 0 && hoveredSlot != dragSourceSlot) {
                if(splitDrop) {
                    (void)sim.splitInventoryStack(dragSourceSlot, hoveredSlot);
                } else {
                    (void)sim.moveInventoryStack(dragSourceSlot, hoveredSlot);
                }
            }
            dragSourceSlot = -1;
            dragSplitMode = false;
        }
        if(IsKeyPressed(KEY_Q)) {
            dragSourceSlot = -1;
            dragSplitMode = false;
        }
    }

    for(int idx = 0; idx < sim.inventorySlotCount(); ++idx) {
        const auto rect = slotRects[static_cast<std::size_t>(idx)];
        const auto slot = sim.inventorySlot(idx);
        const bool hovered = idx == hoveredSlot;
        const bool selected = idx == dragSourceSlot;
        const bool hotbarSlot = idx < lecon::Simulation::kHotbarSlotCount;
        const bool activeHotbarSlot = hotbarSlot && idx == selectedHotbarSlot;

        DrawRectangleRounded(rect, 0.18F, 6, hovered ? Color{50, 64, 84, 255} : Color{36, 44, 58, 255});
        Color border = selected ? Color{132, 205, 240, 255} : Color{83, 98, 121, 255};
        float borderW = 2.0F;
        if(hotbarSlot) {
            border = Color{141, 154, 177, 255};
        }
        if(activeHotbarSlot) {
            border = Color{244, 223, 156, 255};
            borderW = 3.0F;
        }
        DrawRectangleRoundedLinesEx(rect, 0.18F, 6, borderW, border);

        if(slot.item != lecon::ItemId::None && slot.count > 0) {
            const Rectangle inner{rect.x + 5.0F, rect.y + 5.0F, rect.width - 10.0F, rect.height - 10.0F};
            DrawRectangleRounded(inner, 0.18F, 4, itemTint(slot.item));

            const char* glyph = itemGlyph(slot.item);
            const int glyphW = MeasureText(glyph, 24);
            DrawText(glyph, static_cast<int>(rect.x + (rect.width - static_cast<float>(glyphW)) * 0.5F), static_cast<int>(rect.y + 9.0F), 24, Color{242, 247, 255, 255});

            DrawText(TextFormat("%d", slot.count), static_cast<int>(rect.x + 8.0F), static_cast<int>(rect.y + rect.height - 18.0F), 16, Color{242, 247, 255, 255});
            if(slot.count >= lecon::Simulation::kInventoryStackLimit) {
                DrawText("MAX", static_cast<int>(rect.x + rect.width - 32.0F), static_cast<int>(rect.y + rect.height - 18.0F), 12, Color{255, 230, 172, 255});
            }
        }
    }

    DrawText("Drag LMB: move/swap | Drag RMB: split", static_cast<int>(panel.x) + 18, static_cast<int>(panel.y) + 318, 16, Color{176, 200, 226, 255});
    DrawText(TextFormat("Stack limit: %d", lecon::Simulation::kInventoryStackLimit), static_cast<int>(panel.x) + 18, static_cast<int>(panel.y) + 338, 16, Color{164, 174, 191, 255});

    if(dragSourceSlot >= 0) {
        const auto src = sim.inventorySlot(dragSourceSlot);
        if(src.item != lecon::ItemId::None && src.count > 0) {
            const Rectangle ghost{mouse.x + 12.0F, mouse.y + 12.0F, 96.0F, 28.0F};
            DrawRectangleRounded(ghost, 0.2F, 6, Fade(Color{13, 18, 24, 255}, 0.9F));
            DrawRectangleRoundedLinesEx(ghost, 0.2F, 6, 2.0F, Color{118, 171, 220, 255});
            DrawText(TextFormat("%s x%d", itemShortLabel(src.item), src.count), static_cast<int>(ghost.x) + 8, static_cast<int>(ghost.y) + 6, 16, Color{230, 238, 251, 255});
        }
    }

    DrawText("Recipes", static_cast<int>(panel.x) + 18, static_cast<int>(panel.y) + 362, 22, Color{208, 223, 246, 255});

    for(int i = 0; i < static_cast<int>(kRecipes.size()); ++i) {
        const auto& recipe = kRecipes[static_cast<std::size_t>(i)];
        const bool craftable = sim.canCraft(recipe.recipe);
        const bool needsBenchNow = recipe.requiresWorkbench && !nearWorkbench;
        Color textColor = craftable ? Color{158, 239, 177, 255} : Color{175, 183, 197, 255};
        if(needsBenchNow) {
            textColor = Color{232, 185, 115, 255};
        }

        const int y = static_cast<int>(panel.y) + 390 + i * 16;
        DrawText(TextFormat("%s %s", recipe.keyHint, recipe.name), static_cast<int>(panel.x) + 18, y, 16, textColor);
        DrawText(recipe.cost, static_cast<int>(panel.x) + 258, y, 16, Color{170, 180, 194, 255});
        if(recipe.requiresWorkbench) {
            DrawText("WB", static_cast<int>(panel.x) + 432, y, 16, Color{202, 162, 118, 255});
        }
    }
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
    bool inventoryOpen = false;
    int dragSourceSlot = -1;
    bool dragSplitMode = false;

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
                inventoryOpen = false;
                dragSourceSlot = -1;
                dragSplitMode = false;
                hasRun = true;
                screenState = ScreenState::Playing;
            } else if(resume) {
                screenState = ScreenState::Playing;
            }

            continue;
        }

        if(IsKeyPressed(KEY_ESCAPE)) {
            screenState = ScreenState::Menu;
            dragSourceSlot = -1;
            dragSplitMode = false;
        }

        if(IsKeyPressed(KEY_TAB)) {
            inventoryOpen = !inventoryOpen;
            if(!inventoryOpen) {
                dragSourceSlot = -1;
                dragSplitMode = false;
            }
        }

        if(IsKeyPressed(KEY_R)) {
            sim.reset(currentSeed);
            particles.clear();
            cracks.clear();
            facing = {1, 0};
            inventoryOpen = false;
            dragSourceSlot = -1;
            dragSplitMode = false;
        }

        if(IsKeyPressed(KEY_V)) {
            sim.placeWorkbenchForward();
        }

        if(IsKeyPressed(KEY_ONE)) {
            sim.setHotbarSelection(0);
        }
        if(IsKeyPressed(KEY_TWO)) {
            sim.setHotbarSelection(1);
        }
        if(IsKeyPressed(KEY_THREE)) {
            sim.setHotbarSelection(2);
        }
        if(IsKeyPressed(KEY_FOUR)) {
            sim.setHotbarSelection(3);
        }
        if(IsKeyPressed(KEY_FIVE)) {
            sim.setHotbarSelection(4);
        }
        if(IsKeyPressed(KEY_SIX)) {
            sim.setHotbarSelection(5);
        }
        if(IsKeyPressed(KEY_SEVEN)) {
            sim.setHotbarSelection(6);
        }
        if(IsKeyPressed(KEY_EIGHT)) {
            sim.setHotbarSelection(7);
        }
        if(IsKeyPressed(KEY_NINE)) {
            sim.setHotbarSelection(8);
        }

        if(IsKeyPressed(KEY_F1)) {
            sim.craft(lecon::RecipeId::Planks);
        }
        if(IsKeyPressed(KEY_F2)) {
            sim.craft(lecon::RecipeId::Sticks);
        }
        if(IsKeyPressed(KEY_F3)) {
            sim.craft(lecon::RecipeId::Workbench);
        }
        if(IsKeyPressed(KEY_F4)) {
            sim.craft(lecon::RecipeId::AxeTier1);
        }
        if(IsKeyPressed(KEY_F5)) {
            sim.craft(lecon::RecipeId::PickaxeTier1);
        }
        if(IsKeyPressed(KEY_F6)) {
            sim.craft(lecon::RecipeId::AxeTier2);
        }
        if(IsKeyPressed(KEY_F7)) {
            sim.craft(lecon::RecipeId::PickaxeTier2);
        }

        const lecon::Vec2i playerInput = sim.playerPos();
        const int centerXInput = screenW / 2;
        const int centerYInput = screenH / 2;

        const Vector2 mouse = GetMousePosition();
        const int hoverDx = static_cast<int>(std::floor((mouse.x - static_cast<float>(centerXInput)) / static_cast<float>(tileSize)));
        const int hoverDy = static_cast<int>(std::floor((mouse.y - static_cast<float>(centerYInput)) / static_cast<float>(tileSize)));
        const lecon::Vec2i hoverTile{playerInput.x + hoverDx, playerInput.y + hoverDy};
        const lecon::TileType hoverType = sim.tileAt(hoverTile.x, hoverTile.y);
        const bool hoverMineable = hoverType == lecon::TileType::Resource || hoverType == lecon::TileType::Tree || hoverType == lecon::TileType::Workbench;
        const float miningRangeTiles = sim.miningRangeTiles();
        const float hoverDist2 = static_cast<float>(hoverDx * hoverDx + hoverDy * hoverDy);
        const bool hoverInRange = hoverDist2 <= (miningRangeTiles * miningRangeTiles);
        const bool hoverLineOfSight = hoverMineable ? sim.hasLineOfSightTo(hoverTile) : false;
        const bool hoverCanMine = hoverMineable ? sim.canMineTarget(hoverTile) : false;
        const bool mouseMineActive = !inventoryOpen && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && hoverCanMine;

        if(mouseMineActive) {
            sim.setMiningTargetOverride(hoverTile);
        } else {
            sim.clearMiningTargetOverride();
        }

        if(!inventoryOpen && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            if(hoverDx != 0 || hoverDy != 0) {
                if(std::abs(hoverDx) >= std::abs(hoverDy)) {
                    facing = {hoverDx > 0 ? 1 : -1, 0};
                } else {
                    facing = {0, hoverDy > 0 ? 1 : -1};
                }
            }
            (void)sim.contextUseAt(hoverTile);
        }

        stepTimer += dt;
        if(stepTimer >= kStepIntervalSeconds) {
            stepTimer = 0.0F;
            if(!sim.done()) {
                lecon::Action action = actionFromInput();
                if(mouseMineActive) {
                    action = lecon::Action::Mine;
                }
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
                    if(mouseMineActive) {
                        mineTarget = hoverTile;
                    }
                    beforeMineTile = sim.tileAt(mineTarget.x, mineTarget.y);
                }

                sim.step(action);

                const bool minedResource = beforeMineTile == lecon::TileType::Resource;
                const bool minedTree = beforeMineTile == lecon::TileType::Tree;
                const bool minedWorkbench = beforeMineTile == lecon::TileType::Workbench;
                if(action == lecon::Action::Mine && (minedResource || minedTree || minedWorkbench)) {
                    const lecon::TileType afterMineTile = sim.tileAt(mineTarget.x, mineTarget.y);
                    const Vector2 dustPos{
                        static_cast<float>(mineTarget.x) + 0.5F,
                        static_cast<float>(mineTarget.y) + 0.5F
                    };

                    const Color chipColor = minedTree
                                                ? Color{183, 133, 82, 255}
                                                : (minedWorkbench ? Color{166, 116, 82, 255} : Color{238, 194, 109, 255});
                    const Color burstColor = minedTree
                                                 ? Color{220, 176, 132, 255}
                                                 : (minedWorkbench ? Color{215, 169, 134, 255} : Color{255, 223, 145, 255});

                    auto& crack = cracks[tileKey(mineTarget.x, mineTarget.y)];
                    crack.strength = std::min(1.0F, crack.strength + 0.32F);
                    crack.ttl = 1.2F;

                    spawnParticles(particles, dustPos, 6, chipColor, 1.3F, 0.45F, 0.9F, mineTarget.x ^ mineTarget.y);

                    if(afterMineTile != beforeMineTile) {
                        cracks.erase(tileKey(mineTarget.x, mineTarget.y));
                        spawnParticles(particles, dustPos, 18, burstColor, 2.4F, 0.7F, 1.15F, mineTarget.x * 31 + mineTarget.y * 17);
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
                            " Wood=" + std::to_string(sim.wood()) +
                            " Planks=" + std::to_string(sim.planks()) +
                            " Ore=" + std::to_string(sim.ore()) +
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

        const int hoverPx = centerX + (hoverTile.x - player.x) * tileSize;
        const int hoverPy = centerY + (hoverTile.y - player.y) * tileSize;
        if(hoverMineable) {
            Color hoverColor = Color{236, 116, 116, 220};
            if(hoverCanMine) {
                hoverColor = Color{126, 236, 156, 220};
            } else if(hoverInRange && !hoverLineOfSight) {
                hoverColor = Color{246, 186, 92, 220};
            }
            DrawRectangleLinesEx(
                Rectangle{static_cast<float>(hoverPx), static_cast<float>(hoverPy), static_cast<float>(tileSize), static_cast<float>(tileSize)},
                2.0F,
                hoverColor
            );
        }

        DrawCircleLines(
            centerX + tileSize / 2,
            centerY + tileSize / 2,
            static_cast<float>(tileSize) * miningRangeTiles,
            Fade(Color{170, 200, 255, 255}, 0.20F)
        );

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

        drawPlayerSprite(atlas, centerX, centerY, tileSize, t, facing, sim.hotbarSlot(sim.hotbarSelection()).item);
        drawParticles(particles, player, centerX, centerY, tileSize);

        if(hitFlash > 0.01F) {
            DrawRectangle(0, 0, screenW, screenH, Fade(Color{255, 60, 60, 255}, std::clamp(hitFlash, 0.0F, 0.35F)));
        }

        const bool nearWorkbench = sim.isNearWorkbench();
        drawHud(sim, screenH, tileSize, inventoryOpen, nearWorkbench);
        drawHotbar(sim, screenW, screenH);
        if(inventoryOpen) {
            drawInventoryPanel(sim, screenW, nearWorkbench, sim.selectedHotbarSlotIndex(), dragSourceSlot, dragSplitMode);
        }

        EndDrawing();
    }

    UnloadTexture(atlas);
    CloseWindow();
    return 0;
}
