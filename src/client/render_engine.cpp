#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <deque>
#include <limits>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <raylib.h>

#include "stoneforge/client/render_engine.hpp"
#include "stoneforge/client/render_fx.hpp"
#include "stoneforge/client/command_registry.hpp"
#include "stoneforge/client/render_ui.hpp"
#include "stoneforge/mod/asset_manager.hpp"
#include "stoneforge/mod/content_registry.hpp"
#include "stoneforge/mod/mod_loader.hpp"
#include "stoneforge/mod/object_factory.hpp"
#include "stoneforge/mod/runtime_registry.hpp"
#include "stoneforge/mod/script_runtime.hpp"
#include "stoneforge/game_config.hpp"
#include "stoneforge/item.hpp"
#include "stoneforge/simulation.hpp"

using stoneforge::client::CrackInfo;
using stoneforge::client::Particle;
using stoneforge::client::drawBottomVitals;
using stoneforge::client::drawButton;
using stoneforge::client::drawCracks;
using stoneforge::client::drawHotbar;
using stoneforge::client::drawHud;
using stoneforge::client::drawInventoryPanel;
using stoneforge::client::drawParticles;
using stoneforge::client::clearCraftingInputs;
using stoneforge::client::consumeCraftingInputs;
using stoneforge::client::itemGlyph;
using stoneforge::client::itemTint;
using stoneforge::client::spawnParticles;
using stoneforge::client::tileKey;
using stoneforge::client::updateCracks;
using stoneforge::client::updateParticles;

namespace {

constexpr int kWindowW = 1366;
constexpr int kWindowH = 768;
constexpr int kBaseTileSize = 30;
constexpr int kAtlasCell = 16;
struct BiomeWeights {
    std::array<float, 3> w{0.0F, 0.0F, 0.0F};
    int count = 0;
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
    Workbench = 17,
    WoodWall = 18,
    WoodLog = 19
};

enum class ScreenState {
    Menu,
    Playing
};

struct RuntimeBiome {
    std::string id;
    float center = 0.5F;
    float span = 0.34F;
    SpriteId floorA = SpriteId::FloorACold;
    SpriteId floorB = SpriteId::FloorBCold;
    SpriteId wallA = SpriteId::WallACold;
    SpriteId wallB = SpriteId::WallBCold;
    int paletteHint = 0;
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

bool spriteIdFromSlotName(const std::string& slotName, SpriteId& out) {
    if(slotName == "FloorACold") {
        out = SpriteId::FloorACold;
    } else if(slotName == "FloorBCold") {
        out = SpriteId::FloorBCold;
    } else if(slotName == "WallACold") {
        out = SpriteId::WallACold;
    } else if(slotName == "WallBCold") {
        out = SpriteId::WallBCold;
    } else if(slotName == "FloorAWarm") {
        out = SpriteId::FloorAWarm;
    } else if(slotName == "FloorBWarm") {
        out = SpriteId::FloorBWarm;
    } else if(slotName == "WallAWarm") {
        out = SpriteId::WallAWarm;
    } else if(slotName == "WallBWarm") {
        out = SpriteId::WallBWarm;
    } else if(slotName == "FloorAMoss") {
        out = SpriteId::FloorAMoss;
    } else if(slotName == "FloorBMoss") {
        out = SpriteId::FloorBMoss;
    } else if(slotName == "WallAMoss") {
        out = SpriteId::WallAMoss;
    } else if(slotName == "WallBMoss") {
        out = SpriteId::WallBMoss;
    } else if(slotName == "Ore") {
        out = SpriteId::Ore;
    } else if(slotName == "Exit") {
        out = SpriteId::Exit;
    } else if(slotName == "Player") {
        out = SpriteId::Player;
    } else if(slotName == "Mob") {
        out = SpriteId::Mob;
    } else if(slotName == "Tree") {
        out = SpriteId::Tree;
    } else if(slotName == "Workbench") {
        out = SpriteId::Workbench;
    } else if(slotName == "WoodWall") {
        out = SpriteId::WoodWall;
    } else if(slotName == "WoodLog") {
        out = SpriteId::WoodLog;
    } else {
        return false;
    }
    return true;
}

int paletteHintFromBiomeId(const std::string& biomeId) {
    if(biomeId.find("warm") != std::string::npos || biomeId.find("desert") != std::string::npos) {
        return 1;
    }
    if(biomeId.find("moss") != std::string::npos || biomeId.find("forest") != std::string::npos) {
        return 2;
    }
    return 0;
}

std::vector<RuntimeBiome> defaultRuntimeBiomes() {
    return {
        RuntimeBiome{"stoneforge:cold", 0.18F, 0.34F, SpriteId::FloorACold, SpriteId::FloorBCold, SpriteId::WallACold, SpriteId::WallBCold, 0},
        RuntimeBiome{"stoneforge:warm", 0.50F, 0.34F, SpriteId::FloorAWarm, SpriteId::FloorBWarm, SpriteId::WallAWarm, SpriteId::WallBWarm, 1},
        RuntimeBiome{"stoneforge:moss", 0.82F, 0.34F, SpriteId::FloorAMoss, SpriteId::FloorBMoss, SpriteId::WallAMoss, SpriteId::WallBMoss, 2}
    };
}

std::vector<RuntimeBiome> buildRuntimeBiomes(const stoneforge::mod::ContentRegistry& registry) {
    std::vector<RuntimeBiome> out;
    out.reserve(registry.biomes().size());

    for(const auto& [id, def] : registry.biomes()) {
        SpriteId floorA = SpriteId::FloorACold;
        SpriteId floorB = SpriteId::FloorBCold;
        SpriteId wallA = SpriteId::WallACold;
        SpriteId wallB = SpriteId::WallBCold;
        if(!spriteIdFromSlotName(def.floorA, floorA) || !spriteIdFromSlotName(def.floorB, floorB) || !spriteIdFromSlotName(def.wallA, wallA) || !spriteIdFromSlotName(def.wallB, wallB)) {
            continue;
        }

        RuntimeBiome biome;
        biome.id = id;
        biome.center = std::clamp(def.center, 0.0F, 1.0F);
        biome.span = std::max(0.05F, def.span);
        biome.floorA = floorA;
        biome.floorB = floorB;
        biome.wallA = wallA;
        biome.wallB = wallB;
        biome.paletteHint = paletteHintFromBiomeId(id);
        out.push_back(std::move(biome));
    }

    if(out.empty()) {
        return defaultRuntimeBiomes();
    }

    std::sort(out.begin(), out.end(), [](const RuntimeBiome& a, const RuntimeBiome& b) {
        return a.center < b.center;
    });
    if(static_cast<int>(out.size()) > 3) {
        out.resize(3);
    }
    return out;
}

void applySpriteTextureOverrides(
    Texture2D& atlas,
    const stoneforge::mod::ContentRegistry& registry,
    const stoneforge::mod::AssetManager& assets
) {
    for(const auto& [id, def] : registry.sprites()) {
        (void)id;
        SpriteId spriteId = SpriteId::FloorACold;
        if(!spriteIdFromSlotName(def.slot, spriteId)) {
            continue;
        }

        auto texturePathOpt = assets.resolve(def.sourceMod, def.texture);
        if(!texturePathOpt) {
            continue;
        }

        Image image = LoadImage(texturePathOpt->string().c_str());
        if(!IsImageValid(image)) {
            continue;
        }

        ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        if(image.width != kAtlasCell || image.height != kAtlasCell) {
            ImageResizeNN(&image, kAtlasCell, kAtlasCell);
        }

        const Rectangle dst = spriteSource(spriteId);
        UpdateTextureRec(atlas, dst, image.data);
        UnloadImage(image);
    }
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

    fillRect(atlas, 2 * kAtlasCell, 4 * kAtlasCell, kAtlasCell, kAtlasCell, Color{116, 82, 56, 255});
    for(int y = 1; y < kAtlasCell - 1; y += 4) {
        for(int x = 1; x < kAtlasCell - 1; ++x) {
            putPixel(atlas, 2 * kAtlasCell + x, 4 * kAtlasCell + y, Color{147, 106, 74, 255});
        }
    }
    for(int y = 2; y < kAtlasCell - 2; y += 4) {
        for(int x = 1; x < kAtlasCell - 1; ++x) {
            if((x + y) % 5 == 0) {
                putPixel(atlas, 2 * kAtlasCell + x, 4 * kAtlasCell + y, Color{131, 94, 66, 255});
            }
        }
    }
    for(int i = 0; i < 6; ++i) {
        const int knotX = 2 * kAtlasCell + 2 + static_cast<int>(hash01(i, 12, 2301) * 11.0F);
        const int knotY = 4 * kAtlasCell + 2 + static_cast<int>(hash01(12, i, 2303) * 11.0F);
        putPixel(atlas, knotX, knotY, Color{98, 67, 47, 255});
        putPixel(atlas, knotX + 1, knotY, Color{172, 128, 91, 255});
    }
    for(int x = 0; x < kAtlasCell; ++x) {
        putPixel(atlas, 2 * kAtlasCell + x, 4 * kAtlasCell, Color{171, 126, 86, 255});
        putPixel(atlas, 2 * kAtlasCell + x, 4 * kAtlasCell + kAtlasCell - 1, Color{84, 60, 40, 255});
    }
    for(int y = 0; y < kAtlasCell; ++y) {
        putPixel(atlas, 2 * kAtlasCell, 4 * kAtlasCell + y, Color{159, 116, 80, 255});
        putPixel(atlas, 2 * kAtlasCell + kAtlasCell - 1, 4 * kAtlasCell + y, Color{88, 63, 42, 255});
    }

    fillRect(atlas, 3 * kAtlasCell, 4 * kAtlasCell, kAtlasCell, kAtlasCell, Color{102, 72, 49, 255});
    for(int x = 1; x < kAtlasCell - 1; ++x) {
        if(x % 3 == 0) {
            for(int y = 1; y < kAtlasCell - 1; ++y) {
                putPixel(atlas, 3 * kAtlasCell + x, 4 * kAtlasCell + y, Color{133, 95, 65, 255});
            }
        }
    }
    for(int i = 0; i < 14; ++i) {
        const int barkX = 3 * kAtlasCell + 1 + static_cast<int>(hash01(i, 31, 2401) * 14.0F);
        const int barkY = 4 * kAtlasCell + 1 + static_cast<int>(hash01(31, i, 2403) * 14.0F);
        putPixel(atlas, barkX, barkY, Color{119, 85, 58, 255});
    }
    const int ringCx = 3 * kAtlasCell + 8;
    const int ringCy = 4 * kAtlasCell + 8;
    for(int y = -4; y <= 4; ++y) {
        for(int x = -4; x <= 4; ++x) {
            const float d = std::sqrt(static_cast<float>(x * x + y * y));
            if(d < 4.2F) {
                putPixel(atlas, ringCx + x, ringCy + y, Color{165, 120, 82, 255});
            }
            if(d < 2.6F) {
                putPixel(atlas, ringCx + x, ringCy + y, Color{132, 93, 64, 255});
            }
            if(d > 2.8F && d < 3.2F && ((x + y) % 2 == 0)) {
                putPixel(atlas, ringCx + x, ringCy + y, Color{183, 138, 98, 255});
            }
        }
    }

    Texture2D atlasTexture = LoadTextureFromImage(atlas);
    UnloadImage(atlas);
    SetTextureFilter(atlasTexture, TEXTURE_FILTER_POINT);
    return atlasTexture;
}

SpriteId floorVariant(const std::vector<RuntimeBiome>& biomes, int biome, int wx, int wy) {
    if(biomes.empty()) {
        const bool altFallback = hash01(wx, wy, 1401) > 0.5F;
        return altFallback ? SpriteId::FloorBCold : SpriteId::FloorACold;
    }
    const bool alt = hash01(wx, wy, 1401) > 0.5F;
    const int idx = std::clamp(biome, 0, static_cast<int>(biomes.size()) - 1);
    return alt ? biomes[static_cast<std::size_t>(idx)].floorB : biomes[static_cast<std::size_t>(idx)].floorA;
}

SpriteId wallVariant(const std::vector<RuntimeBiome>& biomes, int biome, int wx, int wy) {
    if(biomes.empty()) {
        const bool altFallback = hash01(wx, wy, 1409) > 0.5F;
        return altFallback ? SpriteId::WallBCold : SpriteId::WallACold;
    }
    const bool alt = hash01(wx, wy, 1409) > 0.5F;
    const int idx = std::clamp(biome, 0, static_cast<int>(biomes.size()) - 1);
    return alt ? biomes[static_cast<std::size_t>(idx)].wallB : biomes[static_cast<std::size_t>(idx)].wallA;
}

void drawSpriteTile(const Texture2D& atlas, SpriteId id, int px, int py, int size, Color tint) {
    const Rectangle src = spriteSource(id);
    const Rectangle dst = Rectangle{static_cast<float>(px), static_cast<float>(py), static_cast<float>(size), static_cast<float>(size)};
    DrawTexturePro(atlas, src, dst, Vector2{0.0F, 0.0F}, 0.0F, tint);
}

void drawSpriteTileRotated(const Texture2D& atlas, SpriteId id, int px, int py, int size, float rotationDeg, Color tint) {
    const Rectangle src = spriteSource(id);
    const Rectangle dst = Rectangle{static_cast<float>(px), static_cast<float>(py), static_cast<float>(size), static_cast<float>(size)};
    const Vector2 origin{dst.width * 0.5F, dst.height * 0.5F};
    DrawTexturePro(atlas, src, dst, origin, rotationDeg, tint);
}

void drawOrganicTree(int wx, int wy, int px, int py, int tileSize, int biome, float t) {
    const float phase = hash01(wx, wy, 4901) * 6.2831853F;
    const int sway = static_cast<int>(std::sin(t * 1.15F + phase) * static_cast<float>(std::max(1, tileSize / 14)));
    const int treeType = static_cast<int>(hashU32(wx, wy, 4919) % 3u);

    Color leafA = Color{84, 145, 80, 255};
    Color leafB = Color{114, 176, 106, 255};
    Color leafC = Color{62, 114, 60, 255};
    if(biome == 1) {
        leafA = Color{141, 145, 84, 255};
        leafB = Color{186, 176, 106, 255};
        leafC = Color{115, 114, 60, 255};
    } else if(biome == 0) {
        leafA = Color{90, 124, 101, 255};
        leafB = Color{118, 154, 130, 255};
        leafC = Color{70, 100, 82, 255};
    }

    const int trunkW = std::max(2, tileSize / (treeType == 0 ? 7 : 5));
    const int trunkH = std::max(6, tileSize / (treeType == 2 ? 2 : 3));
    const int trunkLean = (treeType == 2) ? (static_cast<int>(hash01(wx, wy, 4921) * 3.0F) - 1) : 0;
    const int trunkX = px + tileSize / 2 - trunkW / 2 + sway / 2 + trunkLean;
    const int trunkY = py + tileSize - trunkH - 1;

    DrawRectangle(trunkX, trunkY, trunkW, trunkH, Color{110, 77, 49, 255});
    DrawRectangle(trunkX, trunkY, std::max(1, trunkW / 3), trunkH, Color{142, 102, 65, 255});
    DrawRectangle(trunkX + trunkW - std::max(1, trunkW / 3), trunkY, std::max(1, trunkW / 3), trunkH, Color{79, 53, 35, 255});

    const int crownX = px + tileSize / 2 + sway + trunkLean;
    const int crownY = py + tileSize / 3;

    if(treeType == 0) {
        const int baseW = std::max(7, tileSize - tileSize / 4);
        const int tierH = std::max(4, tileSize / 4);

        DrawTriangle(
            Vector2{static_cast<float>(crownX), static_cast<float>(crownY - tierH - 1)},
            Vector2{static_cast<float>(crownX - baseW / 2), static_cast<float>(crownY + tierH)},
            Vector2{static_cast<float>(crownX + baseW / 2), static_cast<float>(crownY + tierH)},
            leafB
        );
        DrawTriangle(
            Vector2{static_cast<float>(crownX), static_cast<float>(crownY - 1)},
            Vector2{static_cast<float>(crownX - (baseW / 2 - 2)), static_cast<float>(crownY + tierH + 4)},
            Vector2{static_cast<float>(crownX + (baseW / 2 - 2)), static_cast<float>(crownY + tierH + 4)},
            leafA
        );
        DrawTriangle(
            Vector2{static_cast<float>(crownX), static_cast<float>(crownY + tierH - 1)},
            Vector2{static_cast<float>(crownX - (baseW / 2 - 3)), static_cast<float>(crownY + tierH + 8)},
            Vector2{static_cast<float>(crownX + (baseW / 2 - 3)), static_cast<float>(crownY + tierH + 8)},
            leafC
        );

        DrawLine(crownX, crownY - tierH - 1, crownX - baseW / 2, crownY + tierH, Fade(Color{28, 52, 30, 255}, 0.5F));
        DrawLine(crownX, crownY - tierH - 1, crownX + baseW / 2, crownY + tierH, Fade(Color{28, 52, 30, 255}, 0.5F));
    } else if(treeType == 1) {
        const float rMain = static_cast<float>(std::max(4, tileSize / 3));
        const float rSide = static_cast<float>(std::max(3, tileSize / 4));
        const float rTop = static_cast<float>(std::max(3, tileSize / 5));

        DrawCircle(crownX, crownY + tileSize / 14, rMain, leafA);
        DrawCircle(crownX - tileSize / 5, crownY + tileSize / 9, rSide, leafB);
        DrawCircle(crownX + tileSize / 5, crownY + tileSize / 10, rSide, leafB);
        DrawCircle(crownX, crownY - tileSize / 7, rTop, leafB);
        DrawCircle(crownX - tileSize / 10, crownY, rTop, leafC);

        DrawCircleLines(crownX, crownY + tileSize / 14, rMain, Fade(Color{32, 58, 34, 255}, 0.45F));
        DrawCircleLines(crownX - tileSize / 5, crownY + tileSize / 9, rSide, Fade(Color{32, 58, 34, 255}, 0.35F));
        DrawCircleLines(crownX + tileSize / 5, crownY + tileSize / 10, rSide, Fade(Color{32, 58, 34, 255}, 0.35F));
    } else {
        const int skew = static_cast<int>(hash01(wx, wy, 4927) * 5.0F) - 2;
        DrawCircle(crownX - tileSize / 6, crownY + tileSize / 8, static_cast<float>(std::max(4, tileSize / 4)), leafA);
        DrawCircle(crownX + tileSize / 5 + skew, crownY, static_cast<float>(std::max(3, tileSize / 5)), leafB);
        DrawCircle(crownX + skew, crownY - tileSize / 6, static_cast<float>(std::max(3, tileSize / 6)), leafC);
        DrawCircle(crownX - tileSize / 4, crownY - tileSize / 10, static_cast<float>(std::max(2, tileSize / 7)), leafB);

        const int branchY = trunkY + trunkH / 3;
        DrawLine(trunkX + trunkW / 2, branchY, trunkX + trunkW / 2 + tileSize / 5, branchY - tileSize / 8, Color{95, 67, 42, 255});
        DrawLine(trunkX + trunkW / 2, branchY + 2, trunkX + trunkW / 2 - tileSize / 6, branchY - 1, Color{95, 67, 42, 255});
    }

    const int sparkleCount = treeType == 0 ? 3 : 5;
    for(int i = 0; i < sparkleCount; ++i) {
        const int ox = static_cast<int>((hash01(wx + i, wy, 4931) - 0.5F) * static_cast<float>(tileSize * 3 / 5));
        const int oy = static_cast<int>((hash01(wx, wy + i, 4933) - 0.5F) * static_cast<float>(tileSize * 2 / 5));
        if(hash01(wx + i, wy - i, 4937) > 0.64F) {
            DrawPixel(crownX + ox, crownY + oy, Fade(Color{216, 243, 177, 255}, 0.55F));
        }
    }

    DrawEllipse(
        px + tileSize / 2,
        py + tileSize - std::max(2, tileSize / 10),
        static_cast<float>(tileSize) * (treeType == 0 ? 0.20F : 0.23F),
        static_cast<float>(tileSize) * 0.08F,
        Fade(BLACK, 0.22F)
    );
}

BiomeWeights biomeWeights(int wx, int wy, const std::vector<RuntimeBiome>& biomes) {
    auto floorDivLocal = [](int value, int divisor) {
        const int q = value / divisor;
        const int r = value % divisor;
        return (r != 0 && ((r > 0) != (divisor > 0))) ? q - 1 : q;
    };
    auto positiveModLocal = [](int value, int divisor) {
        int result = value % divisor;
        if(result < 0) {
            result += divisor;
        }
        return result;
    };
    auto smoothstep = [](float t) {
        t = std::clamp(t, 0.0F, 1.0F);
        return t * t * (3.0F - 2.0F * t);
    };
    auto lerp = [](float a, float b, float t) {
        return a + (b - a) * t;
    };
    auto chunkNoise = [&](int cx, int cy) {
        // Low-frequency value noise on chunk grid creates coherent biome clusters.
        const float fx = static_cast<float>(cx) * 0.23F;
        const float fy = static_cast<float>(cy) * 0.23F;
        const int x0 = static_cast<int>(std::floor(fx));
        const int y0 = static_cast<int>(std::floor(fy));
        const int x1 = x0 + 1;
        const int y1 = y0 + 1;
        const float tx = smoothstep(fx - static_cast<float>(x0));
        const float ty = smoothstep(fy - static_cast<float>(y0));

        const float v00 = hash01(x0, y0, 3301);
        const float v10 = hash01(x1, y0, 3301);
        const float v01 = hash01(x0, y1, 3301);
        const float v11 = hash01(x1, y1, 3301);
        return lerp(lerp(v00, v10, tx), lerp(v01, v11, tx), ty);
    };
    auto chunkBiomeTag = [&](int cx, int cy) {
        if(biomes.empty()) {
            return 0;
        }

        const float n = chunkNoise(cx, cy);
        int best = 0;
        float bestDist = std::numeric_limits<float>::max();
        for(int i = 0; i < static_cast<int>(biomes.size()); ++i) {
            const float d = std::fabs(n - biomes[static_cast<std::size_t>(i)].center);
            if(d < bestDist) {
                bestDist = d;
                best = i;
            }
        }
        return best;
    };

    BiomeWeights out{};
    out.count = std::min(3, static_cast<int>(biomes.size()));
    if(out.count <= 0) {
        out.w[0] = 1.0F;
        out.count = 1;
        return out;
    }

    const int chunkSize = stoneforge::World::kChunkSize;
    const int cx = floorDivLocal(wx, chunkSize);
    const int cy = floorDivLocal(wy, chunkSize);
    const int lx = positiveModLocal(wx, chunkSize);
    const int ly = positiveModLocal(wy, chunkSize);

    const int baseTag = std::clamp(chunkBiomeTag(cx, cy), 0, out.count - 1);
    out.w[static_cast<std::size_t>(baseTag)] += 1.0F;

    const int blendWidth = 5;
    if(lx < blendWidth) {
        const float t = static_cast<float>(blendWidth - lx) / static_cast<float>(blendWidth);
        const int neighbor = std::clamp(chunkBiomeTag(cx - 1, cy), 0, out.count - 1);
        out.w[static_cast<std::size_t>(neighbor)] += t;
    }
    if(lx >= chunkSize - blendWidth) {
        const float t = static_cast<float>(lx - (chunkSize - blendWidth - 1)) / static_cast<float>(blendWidth);
        const int neighbor = std::clamp(chunkBiomeTag(cx + 1, cy), 0, out.count - 1);
        out.w[static_cast<std::size_t>(neighbor)] += t;
    }
    if(ly < blendWidth) {
        const float t = static_cast<float>(blendWidth - ly) / static_cast<float>(blendWidth);
        const int neighbor = std::clamp(chunkBiomeTag(cx, cy - 1), 0, out.count - 1);
        out.w[static_cast<std::size_t>(neighbor)] += t;
    }
    if(ly >= chunkSize - blendWidth) {
        const float t = static_cast<float>(ly - (chunkSize - blendWidth - 1)) / static_cast<float>(blendWidth);
        const int neighbor = std::clamp(chunkBiomeTag(cx, cy + 1), 0, out.count - 1);
        out.w[static_cast<std::size_t>(neighbor)] += t;
    }

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
    secondary = bw.count > 1 ? 1 : 0;
    float best = -1.0F;
    float second = -1.0F;

    for(int i = 0; i < std::max(1, bw.count); ++i) {
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

void drawStyledTile(
    const Texture2D& atlas,
    stoneforge::TileType type,
    int wx,
    int wy,
    int px,
    int py,
    int tileSize,
    float t,
    const std::vector<RuntimeBiome>& biomes
) {
    const BiomeWeights bw = biomeWeights(wx, wy, biomes);

    int primary = 0;
    int secondary = 1;
    float secondaryWeight = 0.0F;
    dominantBiomes(bw, primary, secondary, secondaryWeight);
    const int paletteHint = biomes.empty() ? 0 : biomes[static_cast<std::size_t>(std::clamp(primary, 0, static_cast<int>(biomes.size()) - 1))].paletteHint;

    if(type == stoneforge::TileType::Empty) {
        drawSpriteTile(atlas, floorVariant(biomes, primary, wx, wy), px, py, tileSize, WHITE);
        const float blend = std::clamp(secondaryWeight * 1.2F - 0.15F, 0.0F, 0.65F);
        if(blend > 0.02F) {
            drawSpriteTile(atlas, floorVariant(biomes, secondary, wx + 13, wy + 7), px, py, tileSize, Fade(WHITE, blend));
        }
        return;
    }

    if(type == stoneforge::TileType::Wall) {
        drawSpriteTile(atlas, wallVariant(biomes, primary, wx, wy), px, py, tileSize, WHITE);
        const float blend = std::clamp(secondaryWeight * 1.2F - 0.12F, 0.0F, 0.60F);
        if(blend > 0.02F) {
            drawSpriteTile(atlas, wallVariant(biomes, secondary, wx + 3, wy + 11), px, py, tileSize, Fade(WHITE, blend));
        }
        return;
    }

    if(type == stoneforge::TileType::Resource) {
        drawSpriteTile(atlas, SpriteId::Ore, px, py, tileSize, WHITE);
        return;
    }

    if(type == stoneforge::TileType::Tree) {
        drawSpriteTile(atlas, floorVariant(biomes, primary, wx, wy), px, py, tileSize, WHITE);
        drawOrganicTree(wx, wy, px, py, tileSize, paletteHint, t);
        return;
    }

    if(type == stoneforge::TileType::Workbench) {
        drawSpriteTile(atlas, floorVariant(biomes, primary, wx, wy), px, py, tileSize, WHITE);
        drawSpriteTile(atlas, SpriteId::Workbench, px, py, tileSize, WHITE);
        return;
    }

    if(type == stoneforge::TileType::WoodWall) {
        drawSpriteTile(atlas, SpriteId::WoodWall, px, py, tileSize, WHITE);
        return;
    }

    if(type == stoneforge::TileType::WoodLog) {
        const int rotVariant = static_cast<int>(hashU32(wx, wy, 2241) & 3u);
        const float rot = static_cast<float>(rotVariant) * 90.0F;
        const unsigned char tintC = static_cast<unsigned char>(220 + static_cast<int>(hash01(wx, wy, 2243) * 32.0F));
        drawSpriteTileRotated(atlas, SpriteId::WoodLog, px, py, tileSize, rot, Color{tintC, tintC, tintC, 255});

        const int pattern = static_cast<int>(hashU32(wx, wy, 2249) % 4u);
        const Color dark = Fade(Color{56, 38, 26, 255}, 0.35F);
        const Color light = Fade(Color{178, 137, 96, 255}, 0.30F);
        if(pattern == 0) {
            const int x = px + tileSize / 3;
            DrawLine(x, py + 3, x, py + tileSize - 3, dark);
            DrawLine(x + tileSize / 4, py + 4, x + tileSize / 4, py + tileSize - 4, light);
        } else if(pattern == 1) {
            const int y = py + tileSize / 3;
            DrawLine(px + 3, y, px + tileSize - 3, y, dark);
            DrawLine(px + 4, y + tileSize / 4, px + tileSize - 4, y + tileSize / 4, light);
        } else if(pattern == 2) {
            const int kx = px + tileSize / 2;
            const int ky = py + tileSize / 2;
            DrawCircleLines(kx, ky, static_cast<float>(std::max(2, tileSize / 6)), dark);
            DrawCircleLines(kx, ky, static_cast<float>(std::max(1, tileSize / 9)), light);
        } else {
            DrawLine(px + 4, py + tileSize - 5, px + tileSize - 5, py + 4, dark);
            DrawLine(px + 5, py + tileSize - 4, px + tileSize - 4, py + 5, light);
        }
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

bool isSolidTileForDetail(stoneforge::TileType type) {
    return type == stoneforge::TileType::Wall || type == stoneforge::TileType::Resource || type == stoneforge::TileType::Tree ||
           type == stoneforge::TileType::Workbench || type == stoneforge::TileType::WoodWall || type == stoneforge::TileType::WoodLog;
}

void drawTileDetailPass(
    const stoneforge::Simulation& sim,
    const stoneforge::Vec2i& player,
    int centerX,
    int centerY,
    int tileSize,
    int viewRadiusX,
    int viewRadiusY,
    float t
) {
    const int edgeThin = std::max(1, tileSize / 10);
    const int edgeThick = std::max(1, tileSize / 7);

    for(int vy = -viewRadiusY; vy <= viewRadiusY; ++vy) {
        for(int vx = -viewRadiusX; vx <= viewRadiusX; ++vx) {
            const int wx = player.x + vx;
            const int wy = player.y + vy;
            const stoneforge::TileType type = sim.tileAt(wx, wy);

            const bool solid = isSolidTileForDetail(type);
            const bool northSolid = isSolidTileForDetail(sim.tileAt(wx, wy - 1));
            const bool southSolid = isSolidTileForDetail(sim.tileAt(wx, wy + 1));
            const bool westSolid = isSolidTileForDetail(sim.tileAt(wx - 1, wy));
            const bool eastSolid = isSolidTileForDetail(sim.tileAt(wx + 1, wy));

            const int px = centerX + vx * tileSize;
            const int py = centerY + vy * tileSize;

            if(solid) {
                if(!northSolid) {
                    DrawRectangle(px + 1, py + 1, tileSize - 2, edgeThin, Fade(Color{241, 234, 222, 255}, 0.20F));
                }
                if(!westSolid) {
                    DrawRectangle(px + 1, py + 1, edgeThin, tileSize - 2, Fade(Color{237, 229, 216, 255}, 0.12F));
                }
                if(!southSolid) {
                    DrawRectangle(px + 1, py + tileSize - edgeThick - 1, tileSize - 2, edgeThick, Fade(BLACK, 0.25F));
                }
                if(!eastSolid) {
                    DrawRectangle(px + tileSize - edgeThick - 1, py + 1, edgeThick, tileSize - 2, Fade(BLACK, 0.20F));
                }

                if(type == stoneforge::TileType::Tree) {
                    const float leafWave = 0.45F + 0.55F * std::sin(t * 1.1F + hash01(wx, wy, 3701) * 6.2831853F);
                    DrawCircle(
                        px + tileSize / 2,
                        py + tileSize / 4,
                        static_cast<float>(std::max(2, tileSize / 8)),
                        Fade(Color{194, 244, 168, 255}, 0.10F + leafWave * 0.10F)
                    );
                } else if(type == stoneforge::TileType::Resource) {
                    const float spark = hash01(wx, wy, 3801);
                    if(spark > 0.80F) {
                        DrawCircle(
                            px + tileSize / 2 + static_cast<int>(hash01(wx, wy, 3803) * 5.0F) - 2,
                            py + tileSize / 2 + static_cast<int>(hash01(wx, wy, 3805) * 5.0F) - 2,
                            static_cast<float>(std::max(1, tileSize / 12)),
                            Fade(Color{255, 236, 186, 255}, 0.16F)
                        );
                    }
                }
            } else {
                if(northSolid) {
                    DrawRectangle(px, py, tileSize, edgeThin, Fade(BLACK, 0.11F));
                }
                if(westSolid) {
                    DrawRectangle(px, py, edgeThin, tileSize, Fade(BLACK, 0.09F));
                }
                if(southSolid) {
                    DrawRectangle(px, py + tileSize - edgeThin, tileSize, edgeThin, Fade(Color{255, 236, 196, 255}, 0.06F));
                }
                if(eastSolid) {
                    DrawRectangle(px + tileSize - edgeThin, py, edgeThin, tileSize, Fade(Color{255, 236, 196, 255}, 0.05F));
                }
            }
        }
    }
}

stoneforge::Action actionFromInput() {
    if(IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        return stoneforge::Action::MoveUp;
    }
    if(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        return stoneforge::Action::MoveDown;
    }
    if(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        return stoneforge::Action::MoveLeft;
    }
    if(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        return stoneforge::Action::MoveRight;
    }
    if(IsKeyDown(KEY_Z)) {
        return stoneforge::Action::Mine;
    }
    if(IsKeyDown(KEY_X)) {
        return stoneforge::Action::Place;
    }
    if(IsKeyDown(KEY_C)) {
        return stoneforge::Action::Use;
    }

    return stoneforge::Action::Wait;
}

std::int64_t posKey(int x, int y) {
    const std::uint64_t hi = static_cast<std::uint64_t>(static_cast<std::uint32_t>(x));
    const std::uint64_t lo = static_cast<std::uint64_t>(static_cast<std::uint32_t>(y));
    return static_cast<std::int64_t>((hi << 32) | lo);
}

stoneforge::Vec2i keyToPos(std::int64_t key) {
    const std::uint64_t u = static_cast<std::uint64_t>(key);
    return stoneforge::Vec2i{
        static_cast<int>(static_cast<std::uint32_t>(u >> 32)),
        static_cast<int>(static_cast<std::uint32_t>(u & 0xFFFFFFFFULL))
    };
}

bool isWalkableTile(stoneforge::TileType tile) {
    return tile == stoneforge::TileType::Empty || tile == stoneforge::TileType::Exit;
}

Color forcefieldColor(float value) {
    const float clamped = std::clamp(value, 0.0F, 1.0F);
    const unsigned char red = static_cast<unsigned char>(35.0F + clamped * 220.0F);
    const unsigned char green = static_cast<unsigned char>(64.0F + (1.0F - clamped) * 48.0F);
    const unsigned char blue = static_cast<unsigned char>(232.0F - clamped * 172.0F);
    return Color{red, green, blue, 92};
}

Color potentialGoalAreaColor() {
    return Color{245, 224, 92, 86};
}

float parseThresholdValue(const std::string& text, float fallback) {
    if(text.empty()) {
        return fallback;
    }

    try {
        return std::clamp(std::stof(text), 0.0F, 1.0F);
    } catch(...) {
        return fallback;
    }
}

void trimThresholdText(std::string& text) {
    while(text.size() > 1 && text.front() == '0' && text[1] != '.') {
        text.erase(text.begin());
    }
}

std::optional<stoneforge::Action> actionFromDelta(const stoneforge::Vec2i& delta) {
    if(delta.x == 1 && delta.y == 0) {
        return stoneforge::Action::MoveRight;
    }
    if(delta.x == -1 && delta.y == 0) {
        return stoneforge::Action::MoveLeft;
    }
    if(delta.x == 0 && delta.y == 1) {
        return stoneforge::Action::MoveDown;
    }
    if(delta.x == 0 && delta.y == -1) {
        return stoneforge::Action::MoveUp;
    }
    return std::nullopt;
}

std::optional<stoneforge::Action> autoWalkNextAction(
    const stoneforge::Simulation& sim,
    const stoneforge::Vec2i& start,
    const stoneforge::Vec2i& goal
) {
    if(start == goal) {
        return stoneforge::Action::Wait;
    }

    const int margin = 128;
    const int minX = std::min(start.x, goal.x) - margin;
    const int maxX = std::max(start.x, goal.x) + margin;
    const int minY = std::min(start.y, goal.y) - margin;
    const int maxY = std::max(start.y, goal.y) + margin;

    std::deque<stoneforge::Vec2i> queue;
    std::unordered_map<std::int64_t, std::int64_t> parent;

    const std::int64_t startKey = posKey(start.x, start.y);
    const std::int64_t goalKey = posKey(goal.x, goal.y);
    queue.push_back(start);
    parent[startKey] = startKey;

    constexpr std::array<stoneforge::Vec2i, 4> kDirs = {
        stoneforge::Vec2i{1, 0},
        stoneforge::Vec2i{-1, 0},
        stoneforge::Vec2i{0, 1},
        stoneforge::Vec2i{0, -1},
    };

    bool found = false;
    while(!queue.empty()) {
        const stoneforge::Vec2i cur = queue.front();
        queue.pop_front();

        if(cur == goal) {
            found = true;
            break;
        }

        for(const stoneforge::Vec2i dir : kDirs) {
            const int nx = cur.x + dir.x;
            const int ny = cur.y + dir.y;
            if(nx < minX || nx > maxX || ny < minY || ny > maxY) {
                continue;
            }

            const std::int64_t key = posKey(nx, ny);
            if(parent.find(key) != parent.end()) {
                continue;
            }

            const bool isGoal = (nx == goal.x && ny == goal.y);
            const bool walkable = isGoal || isWalkableTile(sim.tileAt(nx, ny));
            if(!walkable) {
                continue;
            }

            if(!isGoal && sim.hasMobAt(nx, ny)) {
                continue;
            }

            parent[key] = posKey(cur.x, cur.y);
            queue.push_back(stoneforge::Vec2i{nx, ny});
            if(isGoal) {
                found = true;
                queue.clear();
                break;
            }
        }
    }

    if(!found || parent.find(goalKey) == parent.end()) {
        return std::nullopt;
    }

    std::int64_t stepKey = goalKey;
    while(parent[stepKey] != startKey) {
        stepKey = parent[stepKey];
        if(stepKey == startKey) {
            break;
        }
    }

    const stoneforge::Vec2i stepPos = keyToPos(stepKey);
    const stoneforge::Vec2i delta{stepPos.x - start.x, stepPos.y - start.y};
    return actionFromDelta(delta);
}

void drawMobSprite(const Texture2D& atlas, const stoneforge::Mob& mob, int px, int py, int tileSize, float t, int idx) {
    const float wobble = std::sin(t * 5.2F + static_cast<float>(idx)) * 1.5F;
    DrawEllipse(px + tileSize / 2, py + static_cast<int>(tileSize * 0.88F), tileSize * 0.25F, tileSize * 0.10F, Fade(BLACK, 0.3F));

    Color tint = WHITE;
    if(mob.variant == "alpha") {
        tint = Color{206, 235, 255, 255};
    } else if(mob.variant == "boss") {
        tint = Color{255, 198, 142, 255};
    }
    if(mob.aggro) {
        tint = Color{255, 165, 165, 255};
    }

    drawSpriteTile(atlas, SpriteId::Mob, px, py + static_cast<int>(wobble), tileSize, tint);
}

void drawPlayerSprite(const Texture2D& atlas, int px, int py, int tileSize, float t, const stoneforge::Vec2i& facing, const std::string& heldItemId) {
    const int bob = static_cast<int>(std::sin(t * 7.0F) * 1.6F);
    DrawEllipse(px + tileSize / 2, py + static_cast<int>(tileSize * 0.90F), tileSize * 0.28F, tileSize * 0.11F, Fade(BLACK, 0.35F));
    drawSpriteTile(atlas, SpriteId::Player, px, py + bob, tileSize, WHITE);

    if(!heldItemId.empty()) {
        const int handOffsetX = facing.x * (tileSize / 3);
        const int handOffsetY = facing.y * (tileSize / 3);
        const int hx = px + tileSize / 2 + handOffsetX;
        const int hy = py + tileSize / 2 + bob + handOffsetY;
        const int s = std::max(8, tileSize / 3);
        DrawRectangleRounded(
            Rectangle{static_cast<float>(hx - s / 2), static_cast<float>(hy - s / 2), static_cast<float>(s), static_cast<float>(s)},
            0.25F,
            4,
            itemTint(heldItemId)
        );
        const std::string glyph = itemGlyph(heldItemId);
        DrawText(glyph.c_str(), hx - 4, hy - 6, 12, WHITE);
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

int stoneforge::client::RenderEngine::run() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(kWindowW, kWindowH, "Stoneforge 2D - raylib biome client");
    SetTargetFPS(60);

    std::string gameConfigError;
    if(!stoneforge::loadGameConfigFile("assets/base/game_config.json", &gameConfigError)) {
        TraceLog(LOG_WARNING, "Game config load failed: %s", gameConfigError.c_str());
    }
    const float stepIntervalSeconds = std::max(0.01F, stoneforge::gameConfig().render.stepIntervalSeconds);

    stoneforge::mod::ContentRegistry contentRegistry;
    stoneforge::mod::AssetManager assetManager;
    stoneforge::mod::ModLoader modLoader;
    std::vector<stoneforge::mod::LoadedModInfo> loadedMods;
    std::string modError;
    if(!modLoader.loadAll("assets/base", "mods", contentRegistry, assetManager, loadedMods, &modError)) {
        TraceLog(LOG_WARNING, "Mod loading failed: %s", modError.c_str());
    }
    stoneforge::setItemRegistry(&contentRegistry);

    stoneforge::mod::ObjectFactory objectFactory;
    objectFactory.buildFromContent(contentRegistry);
    stoneforge::mod::RuntimeRegistry runtimeRegistry;
    runtimeRegistry.build(contentRegistry, objectFactory);
    stoneforge::client::CommandRegistry commandRegistry;
    std::vector<RuntimeBiome> runtimeBiomes = buildRuntimeBiomes(contentRegistry);
    TraceLog(LOG_INFO, "Object archetypes loaded: %d", static_cast<int>(objectFactory.objects().size()));
    TraceLog(LOG_INFO, "Biome registry loaded: %d", static_cast<int>(contentRegistry.biomes().size()));
    TraceLog(LOG_INFO, "Entity registry loaded: %d", static_cast<int>(contentRegistry.entities().size()));

    stoneforge::mod::ScriptRuntime scriptRuntime;
    if(!scriptRuntime.initialize()) {
        TraceLog(LOG_WARNING, "Script runtime disabled: %s", scriptRuntime.lastError().c_str());
    }
    scriptRuntime.loadScripts(loadedMods);

    stoneforge::Simulation sim;
    Texture2D atlas = buildSpriteAtlas();
    applySpriteTextureOverrides(atlas, contentRegistry, assetManager);

    ScreenState screenState = ScreenState::Menu;

    std::string seedInput = "42";
    std::uint64_t currentSeed = 42;
    bool hasRun = false;

    float stepTimer = 0.0F;
    float zoom = 1.0F;

    std::vector<Particle> particles;
    std::unordered_map<std::int64_t, CrackInfo> cracks;

    stoneforge::Vec2i facing{1, 0};
    float hitFlash = 0.0F;
    bool inventoryOpen = false;
    int dragSourceSlot = -1;
    bool dragSplitMode = false;
    stoneforge::client::CraftingPanelState craftingPanel;
    bool commandMode = false;
    std::string commandInput;
    std::string commandFeedback;
    float commandFeedbackTtl = 0.0F;
    bool autoWalkEnabled = false;
    bool forcefieldEnabled = false;
    bool showPotentialGoalSpawnArea = false;
    bool thresholdInputActive = false;
    std::string thresholdInput = "0.1";

    while(!WindowShouldClose()) {
        const int screenW = GetScreenWidth();
        const int screenH = GetScreenHeight();
        const float t = static_cast<float>(GetTime());
        const float dt = GetFrameTime();
        const float wheelMove = GetMouseWheelMove();

        updateParticles(particles, dt);
        updateCracks(cracks, dt);
        hitFlash = std::max(0.0F, hitFlash - dt * 2.0F);

        if(IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
            zoom += wheelMove * 0.08F;
        }
        zoom = std::clamp(zoom, 0.70F, 1.70F);
        const int tileSize = std::clamp(static_cast<int>(std::round(static_cast<float>(kBaseTileSize) * zoom)), 16, 48);
        commandFeedbackTtl = std::max(0.0F, commandFeedbackTtl - dt);

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

            DrawText("STONEFORGE 2D", screenW / 2 - 130, screenH / 2 - 190, 58, Color{220, 235, 255, 255});
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
                clearCraftingInputs(craftingPanel.slots);
                commandMode = false;
                commandInput.clear();
                autoWalkEnabled = false;
                forcefieldEnabled = false;
                showPotentialGoalSpawnArea = false;
                thresholdInputActive = false;
                hasRun = true;
                screenState = ScreenState::Playing;
            } else if(resume) {
                commandMode = false;
                commandInput.clear();
                screenState = ScreenState::Playing;
            }

            continue;
        }

        if(!commandMode && IsKeyPressed(KEY_ESCAPE)) {
            screenState = ScreenState::Menu;
            dragSourceSlot = -1;
            dragSplitMode = false;
            commandMode = false;
            commandInput.clear();
        }

        if(!commandMode && IsKeyPressed(KEY_SLASH)) {
            commandMode = true;
            commandInput = "/";
        }

        if(thresholdInputActive) {
            int ch = GetCharPressed();
            while(ch > 0) {
                const char c = static_cast<char>(ch);
                if((c >= '0' && c <= '9') || c == '.') {
                    if(thresholdInput.size() < 8) {
                        thresholdInput.push_back(c);
                        trimThresholdText(thresholdInput);
                    }
                }
                ch = GetCharPressed();
            }

            if(IsKeyPressed(KEY_BACKSPACE) && !thresholdInput.empty()) {
                thresholdInput.pop_back();
            }

            if(IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                thresholdInputActive = false;
            }
        }

        if(commandMode) {
            const stoneforge::client::CommandExecutionContext commandCtx{sim, contentRegistry, objectFactory, runtimeRegistry};
            int ch = GetCharPressed();
            while(ch > 0) {
                const char c = static_cast<char>(ch);
                const bool allowed = std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == ':' || c == '-' || c == ' ' || c == '/' || c == '=' || c == '#';
                if(allowed && commandInput.size() < 160) {
                    if(!(c == '/' && commandInput == "/")) {
                        commandInput.push_back(c);
                    }
                }
                ch = GetCharPressed();
            }

            if(IsKeyPressed(KEY_BACKSPACE) && !commandInput.empty()) {
                commandInput.pop_back();
            }

            if(IsKeyPressed(KEY_TAB)) {
                const auto suggestions = commandRegistry.autocomplete(commandInput, commandCtx);
                if(suggestions.size() == 1) {
                    commandInput = suggestions.front();
                } else if(!suggestions.empty()) {
                    std::string s = "SUGGEST: ";
                    for(std::size_t i = 0; i < suggestions.size() && i < 6; ++i) {
                        if(i > 0) {
                            s += " | ";
                        }
                        s += suggestions[i];
                    }
                    if(suggestions.size() > 6) {
                        s += " | ...";
                    }
                    commandFeedback = s;
                    commandFeedbackTtl = 4.0F;
                }
            }

            if(IsKeyPressed(KEY_ENTER)) {
                const auto result = commandRegistry.execute(commandInput, commandCtx);
                commandFeedback = result.success ? "OK: " + result.message : "ERR: " + result.message;
                commandFeedbackTtl = 5.5F;
                commandMode = false;
                commandInput.clear();
            } else if(IsKeyPressed(KEY_ESCAPE)) {
                commandMode = false;
                commandInput.clear();
            }
        }

        const bool gameplayInputBlocked = commandMode;

        if(!gameplayInputBlocked && IsKeyPressed(KEY_TAB)) {
            inventoryOpen = !inventoryOpen;
            if(!inventoryOpen) {
                dragSourceSlot = -1;
                dragSplitMode = false;
            }
        }

        if(!gameplayInputBlocked && IsKeyPressed(KEY_R)) {
            currentSeed += 1ULL;
            sim.reset(currentSeed);
            particles.clear();
            cracks.clear();
            facing = {1, 0};
            inventoryOpen = false;
            dragSourceSlot = -1;
            dragSplitMode = false;
            clearCraftingInputs(craftingPanel.slots);
            commandMode = false;
            commandInput.clear();
            autoWalkEnabled = false;
            forcefieldEnabled = false;
            showPotentialGoalSpawnArea = false;
            thresholdInputActive = false;
        }

        if(!gameplayInputBlocked && IsKeyPressed(KEY_G) && !sim.done()) {
            autoWalkEnabled = !autoWalkEnabled;
        }

        if(!gameplayInputBlocked && IsKeyPressed(KEY_F) && !sim.done()) {
            forcefieldEnabled = !forcefieldEnabled;
        }

        if(!gameplayInputBlocked && IsKeyPressed(KEY_P) && !sim.done()) {
            showPotentialGoalSpawnArea = !showPotentialGoalSpawnArea;
        }

        if(!gameplayInputBlocked && IsKeyPressed(KEY_V)) {
            sim.placeWorkbenchForward();
        }

        if(!gameplayInputBlocked && !inventoryOpen && std::fabs(wheelMove) > 0.01F && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_RIGHT_CONTROL)) {
            int nextSlot = sim.hotbarSelection();
            const int hotbarSlots = sim.hotbarSlotCount();
            if(wheelMove > 0.0F) {
                nextSlot -= 1;
            } else {
                nextSlot += 1;
            }

            if(nextSlot < 0) {
                nextSlot = hotbarSlots - 1;
            } else if(nextSlot >= hotbarSlots) {
                nextSlot = 0;
            }

            sim.setHotbarSelection(nextSlot);
        }

        if(!gameplayInputBlocked && IsKeyPressed(KEY_ONE)) {
            sim.setHotbarSelection(0);
        }
        if(!gameplayInputBlocked && IsKeyPressed(KEY_TWO)) {
            sim.setHotbarSelection(1);
        }
        if(!gameplayInputBlocked && IsKeyPressed(KEY_THREE)) {
            sim.setHotbarSelection(2);
        }
        if(!gameplayInputBlocked && IsKeyPressed(KEY_FOUR)) {
            sim.setHotbarSelection(3);
        }
        if(!gameplayInputBlocked && IsKeyPressed(KEY_FIVE)) {
            sim.setHotbarSelection(4);
        }
        if(!gameplayInputBlocked && IsKeyPressed(KEY_SIX)) {
            sim.setHotbarSelection(5);
        }
        if(!gameplayInputBlocked && IsKeyPressed(KEY_SEVEN)) {
            sim.setHotbarSelection(6);
        }
        if(!gameplayInputBlocked && IsKeyPressed(KEY_EIGHT)) {
            sim.setHotbarSelection(7);
        }
        if(!gameplayInputBlocked && IsKeyPressed(KEY_NINE)) {
            sim.setHotbarSelection(8);
        }

        const stoneforge::Vec2i playerInput = sim.playerPos();
        const int centerXInput = screenW / 2;
        const int centerYInput = screenH / 2;

        const Vector2 mouse = GetMousePosition();
        const int hoverDx = static_cast<int>(std::floor((mouse.x - static_cast<float>(centerXInput)) / static_cast<float>(tileSize)));
        const int hoverDy = static_cast<int>(std::floor((mouse.y - static_cast<float>(centerYInput)) / static_cast<float>(tileSize)));
        const stoneforge::Vec2i hoverTile{playerInput.x + hoverDx, playerInput.y + hoverDy};
        const stoneforge::TileType hoverType = sim.tileAt(hoverTile.x, hoverTile.y);
        const bool hoverMineable = hoverType == stoneforge::TileType::Resource || hoverType == stoneforge::TileType::Tree || hoverType == stoneforge::TileType::Workbench;
        const stoneforge::TileType previewPlaceTile = sim.previewPlacementTileForSelectedHotbar();
        const bool hasPlacePreview = previewPlaceTile != stoneforge::TileType::Empty;
        const float miningRangeTiles = sim.miningRangeTiles();
        const float hoverDist2 = static_cast<float>(hoverDx * hoverDx + hoverDy * hoverDy);
        const bool hoverInRange = hoverDist2 <= (miningRangeTiles * miningRangeTiles);
        const bool hoverLineOfSight = hoverMineable ? sim.hasLineOfSightTo(hoverTile) : false;
        const bool hoverCanPlace = hasPlacePreview ? sim.canPlaceFromHotbarAt(hoverTile) : false;
        const bool hoverCanMine = hoverMineable ? sim.canMineTarget(hoverTile) : false;
        const bool mouseMineActive = !gameplayInputBlocked && !inventoryOpen && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && hoverCanMine;

        if(mouseMineActive) {
            sim.setMiningTargetOverride(hoverTile);
        } else {
            sim.clearMiningTargetOverride();
        }

        if(!gameplayInputBlocked && !inventoryOpen && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            const stoneforge::TileType beforeContext = sim.tileAt(hoverTile.x, hoverTile.y);
            if(hoverDx != 0 || hoverDy != 0) {
                if(std::abs(hoverDx) >= std::abs(hoverDy)) {
                    facing = {hoverDx > 0 ? 1 : -1, 0};
                } else {
                    facing = {0, hoverDy > 0 ? 1 : -1};
                }
            }
            const bool used = sim.contextUseAt(hoverTile);
            if(used) {
                const stoneforge::TileType afterContext = sim.tileAt(hoverTile.x, hoverTile.y);
                scriptRuntime.emitEvent("onItemUsed", {{"x", std::to_string(hoverTile.x)}, {"y", std::to_string(hoverTile.y)}});
                if(beforeContext != afterContext) {
                    scriptRuntime.emitEvent("onBlockPlaced", {{"x", std::to_string(hoverTile.x)}, {"y", std::to_string(hoverTile.y)}});
                }
            }
        }

        stepTimer += dt;
        if(stepTimer >= stepIntervalSeconds) {
            stepTimer = 0.0F;
            if(!sim.done()) {
                stoneforge::Action action = gameplayInputBlocked ? stoneforge::Action::Wait : actionFromInput();
                if(action != stoneforge::Action::Wait && action != stoneforge::Action::Noop) {
                    autoWalkEnabled = false;
                }

                if(autoWalkEnabled && !inventoryOpen && !gameplayInputBlocked && !mouseMineActive) {
                    const auto autoAction = autoWalkNextAction(sim, sim.playerPos(), sim.exitPos());
                    if(autoAction.has_value()) {
                        action = *autoAction;
                    } else {
                        autoWalkEnabled = false;
                    }
                }

                if(mouseMineActive) {
                    action = stoneforge::Action::Mine;
                }
                const stoneforge::Vec2i playerBefore = sim.playerPos();
                const int hpBefore = sim.hp();

                if(action == stoneforge::Action::MoveUp) {
                    facing = {0, -1};
                } else if(action == stoneforge::Action::MoveDown) {
                    facing = {0, 1};
                } else if(action == stoneforge::Action::MoveLeft) {
                    facing = {-1, 0};
                } else if(action == stoneforge::Action::MoveRight) {
                    facing = {1, 0};
                }

                stoneforge::TileType beforeMineTile = stoneforge::TileType::Empty;
                stoneforge::Vec2i mineTarget{playerBefore.x + facing.x, playerBefore.y + facing.y};
                const stoneforge::Vec2i placeTarget{playerBefore.x + facing.x, playerBefore.y + facing.y};
                const stoneforge::TileType beforePlaceTile = sim.tileAt(placeTarget.x, placeTarget.y);
                if(action == stoneforge::Action::Mine) {
                    if(mouseMineActive) {
                        mineTarget = hoverTile;
                    }
                    beforeMineTile = sim.tileAt(mineTarget.x, mineTarget.y);
                }

                sim.step(action);
                if(sim.reachedExit() || sim.done()) {
                    autoWalkEnabled = false;
                }
                scriptRuntime.emitEvent("onTick", {{"step", std::to_string(sim.steps())}});

                if(action == stoneforge::Action::Place) {
                    const stoneforge::TileType afterPlaceTile = sim.tileAt(placeTarget.x, placeTarget.y);
                    if(afterPlaceTile != beforePlaceTile) {
                        scriptRuntime.emitEvent("onBlockPlaced", {{"x", std::to_string(placeTarget.x)}, {"y", std::to_string(placeTarget.y)}});
                    }
                }

                if(action == stoneforge::Action::Use) {
                    scriptRuntime.emitEvent("onItemUsed", {{"x", std::to_string(playerBefore.x)}, {"y", std::to_string(playerBefore.y)}});
                }

                const bool minedResource = beforeMineTile == stoneforge::TileType::Resource;
                const bool minedTree = beforeMineTile == stoneforge::TileType::Tree;
                const bool minedWorkbench = beforeMineTile == stoneforge::TileType::Workbench;
                if(action == stoneforge::Action::Mine && (minedResource || minedTree || minedWorkbench)) {
                    const stoneforge::TileType afterMineTile = sim.tileAt(mineTarget.x, mineTarget.y);
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
                        scriptRuntime.emitEvent("onBlockBroken", {{"x", std::to_string(mineTarget.x)}, {"y", std::to_string(mineTarget.y)}});
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

        std::string title = "Stoneforge 2D | Seed=" + std::to_string(currentSeed) +
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

        const stoneforge::Vec2i player = sim.playerPos();
        const stoneforge::Vec2i exit = sim.exitPos();

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

                drawStyledTile(atlas, sim.tileAt(wx, wy), wx, wy, px, py, tileSize, t, runtimeBiomes);
            }
        }

        drawTileDetailPass(sim, player, centerX, centerY, tileSize, viewRadiusX, viewRadiusY, t);

        if(showPotentialGoalSpawnArea) {
            const auto& worldCfg = stoneforge::gameConfig().world;
            const int minDistance = std::max(1, worldCfg.exitMinDistance);
            const int maxDistance = std::max(minDistance, worldCfg.exitMaxDistance);
            const int min2 = minDistance * minDistance;
            const int max2 = maxDistance * maxDistance;
            for(int vy = -viewRadiusY; vy <= viewRadiusY; ++vy) {
                for(int vx = -viewRadiusX; vx <= viewRadiusX; ++vx) {
                    const int wx = player.x + vx;
                    const int wy = player.y + vy;
                    const int dx = wx - worldCfg.spawn.x;
                    const int dy = wy - worldCfg.spawn.y;
                    const int dist2 = dx * dx + dy * dy;
                    if(dist2 < min2 || dist2 > max2) {
                        continue;
                    }

                    const int px = centerX + vx * tileSize;
                    const int py = centerY + vy * tileSize;
                    DrawRectangle(px, py, tileSize, tileSize, potentialGoalAreaColor());
                }
            }
        }

        if(forcefieldEnabled) {
            const float threshold = parseThresholdValue(thresholdInput, 0.1F);
            const float maxDistance = static_cast<float>(std::max(1, stoneforge::gameConfig().world.exitMaxDistance));
            for(int vy = -viewRadiusY; vy <= viewRadiusY; ++vy) {
                for(int vx = -viewRadiusX; vx <= viewRadiusX; ++vx) {
                    const int wx = player.x + vx;
                    const int wy = player.y + vy;
                    const float dx = static_cast<float>(exit.x - wx);
                    const float dy = static_cast<float>(exit.y - wy);
                    const float dist = std::sqrt(dx * dx + dy * dy);
                    const float value = 1.0F - std::clamp(dist / maxDistance, 0.0F, 1.0F);
                    if(value < threshold) {
                        continue;
                    }

                    const int px = centerX + vx * tileSize;
                    const int py = centerY + vy * tileSize;
                    DrawRectangle(px, py, tileSize, tileSize, forcefieldColor(value));
                }
            }
        }

        const int hoverPx = centerX + (hoverTile.x - player.x) * tileSize;
        const int hoverPy = centerY + (hoverTile.y - player.y) * tileSize;

        if(!inventoryOpen && hasPlacePreview) {
            drawStyledTile(atlas, previewPlaceTile, hoverTile.x, hoverTile.y, hoverPx, hoverPy, tileSize, t, runtimeBiomes);
            const Color overlay = hoverCanPlace ? Fade(Color{126, 236, 156, 255}, 0.42F) : Fade(Color{236, 116, 116, 255}, 0.45F);
            DrawRectangle(hoverPx, hoverPy, tileSize, tileSize, overlay);
            DrawRectangleLinesEx(
                Rectangle{static_cast<float>(hoverPx), static_cast<float>(hoverPy), static_cast<float>(tileSize), static_cast<float>(tileSize)},
                hoverCanPlace ? 3.0F : 2.0F,
                hoverCanPlace ? Color{126, 236, 156, 235} : Color{236, 116, 116, 235}
            );
            if(!hoverCanPlace) {
                DrawLine(hoverPx + 4, hoverPy + 4, hoverPx + tileSize - 4, hoverPy + tileSize - 4, Color{236, 116, 116, 235});
                DrawLine(hoverPx + tileSize - 4, hoverPy + 4, hoverPx + 4, hoverPy + tileSize - 4, Color{236, 116, 116, 235});
            }
        }

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
        DrawRectangleGradientV(0, 0, screenW, screenH / 2, Fade(Color{138, 166, 204, 255}, 0.08F), Fade(BLANK, 0.0F));
        DrawRectangleGradientV(0, screenH / 2, screenW, screenH / 2, Fade(BLANK, 0.0F), Fade(Color{0, 0, 0, 255}, 0.14F));
        DrawRectangleGradientEx(
            Rectangle{0.0F, 0.0F, static_cast<float>(screenW), static_cast<float>(screenH)},
            Fade(BLACK, 0.16F),
            Fade(BLACK, 0.16F),
            Fade(BLACK, 0.04F),
            Fade(BLACK, 0.04F)
        );

        const int ex = centerX + (exit.x - player.x) * tileSize + tileSize / 2;
        const int ey = centerY + (exit.y - player.y) * tileSize + tileSize / 2;
        DrawCircleGradient(ex, ey, static_cast<float>(tileSize) * 1.2F, Fade(Color{90, 255, 180, 255}, 0.4F), Fade(Color{90, 255, 180, 0}, 0.0F));

        int mobIdx = 0;
        const auto visibleMobs = sim.mobsInRect(
            player.x - viewRadiusX - 1,
            player.y - viewRadiusY - 1,
            player.x + viewRadiusX + 1,
            player.y + viewRadiusY + 1
        );
        for(const auto* mob : visibleMobs) {
            if(mob == nullptr) {
                continue;
            }
            const int mx = centerX + (mob->pos.x - player.x) * tileSize;
            const int my = centerY + (mob->pos.y - player.y) * tileSize;
            drawMobSprite(atlas, *mob, mx, my, tileSize, t, mobIdx);
            ++mobIdx;
        }

        drawPlayerSprite(atlas, centerX, centerY, tileSize, t, facing, sim.hotbarSlot(sim.hotbarSelection()).itemId);
        drawParticles(particles, player, centerX, centerY, tileSize);

        if(hitFlash > 0.01F) {
            DrawRectangle(0, 0, screenW, screenH, Fade(Color{255, 60, 60, 255}, std::clamp(hitFlash, 0.0F, 0.35F)));
        }

        const bool nearWorkbench = sim.isNearWorkbench();
        drawHud(sim, screenW, screenH, tileSize, inventoryOpen, nearWorkbench);
        drawBottomVitals(sim, screenW, screenH);
        drawHotbar(sim, screenW, screenH);

        const stoneforge::Vec2i posNow = sim.playerPos();
        const stoneforge::Vec2i goalNow = sim.exitPos();
        const int goalDistance = std::abs(goalNow.x - posNow.x) + std::abs(goalNow.y - posNow.y);
        const Rectangle autoWalkBtn = {24.0F, 24.0F, 210.0F, 40.0F};
        const Rectangle forcefieldBtn = {244.0F, 24.0F, 226.0F, 40.0F};
        const Rectangle potentialGoalBtn = {478.0F, 24.0F, 286.0F, 40.0F};
        const bool autoWalkClicked = drawButton(autoWalkBtn, autoWalkEnabled ? "Auto-Walk: ON" : "Auto-Walk: OFF", !sim.done());
        const bool forcefieldClicked = drawButton(forcefieldBtn, forcefieldEnabled ? "Forcefield: ON" : "Forcefield: OFF", !sim.done());
        const bool potentialGoalClicked = drawButton(
            potentialGoalBtn,
            showPotentialGoalSpawnArea ? "ShowPotentialGoalSpawnArea: ON" : "ShowPotentialGoalSpawnArea: OFF",
            !sim.done()
        );
        if(autoWalkClicked) {
            autoWalkEnabled = !autoWalkEnabled;
        }
        if(forcefieldClicked) {
            forcefieldEnabled = !forcefieldEnabled;
        }
        if(potentialGoalClicked) {
            showPotentialGoalSpawnArea = !showPotentialGoalSpawnArea;
        }

        const Rectangle thresholdBox = {24.0F, 70.0F, 140.0F, 34.0F};
        if(!sim.done() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), thresholdBox)) {
            thresholdInputActive = true;
        }

        DrawText(TextFormat("Goal distance: %d", goalDistance), 24, 112, 18, Color{208, 224, 243, 255});
        DrawText("G toggles Auto-Walk | F toggles Forcefield | P toggles Goal Area", 24, 133, 16, Color{175, 193, 215, 255});
        DrawText("Threshold", 180, 70, 16, Color{208, 224, 243, 255});
        DrawRectangleRounded(thresholdBox, 0.2F, 6, thresholdInputActive ? Color{34, 45, 62, 255} : Color{24, 31, 43, 255});
        DrawRectangleRoundedLinesEx(thresholdBox, 0.2F, 6, 2.0F, thresholdInputActive ? Color{132, 176, 229, 255} : Color{84, 104, 132, 255});
        const std::string thresholdShown = thresholdInput.empty() ? "0.1" : thresholdInput;
        DrawText(thresholdShown.c_str(), static_cast<int>(thresholdBox.x) + 12, static_cast<int>(thresholdBox.y) + 7, 18, Color{233, 242, 255, 255});
        if(thresholdInputActive && ((static_cast<int>(t * 2.0F) % 2) == 0)) {
            const int tw = MeasureText(thresholdShown.c_str(), 18);
            DrawText("_", static_cast<int>(thresholdBox.x) + 12 + tw + 2, static_cast<int>(thresholdBox.y) + 7, 18, Color{233, 242, 255, 255});
        }

        craftingPanel.craftRequested = false;
        if(inventoryOpen) {
            drawInventoryPanel(sim, screenW, nearWorkbench, sim.selectedHotbarSlotIndex(), dragSourceSlot, dragSplitMode, craftingPanel);
            if(craftingPanel.craftRequested && sim.craft(craftingPanel.requestedRecipe)) {
                consumeCraftingInputs(craftingPanel.slots, craftingPanel.requestedRecipe);
                scriptRuntime.emitEvent("onCraft", {{"recipe", craftingPanel.requestedRecipe}});
            }
        }

        if(commandFeedbackTtl > 0.0F) {
            const Rectangle msgBox{24.0F, static_cast<float>(screenH - 174), static_cast<float>(screenW - 48), 32.0F};
            DrawRectangleRounded(msgBox, 0.15F, 6, Fade(Color{16, 22, 31, 255}, 0.85F));
            DrawRectangleRoundedLinesEx(msgBox, 0.15F, 6, 1.0F, Color{89, 115, 146, 255});
            DrawText(commandFeedback.c_str(), 34, screenH - 165, 18, Color{213, 228, 248, 255});
        }

        if(commandMode) {
            const Rectangle cmdBox{24.0F, static_cast<float>(screenH - 136), static_cast<float>(screenW - 48), 40.0F};
            DrawRectangleRounded(cmdBox, 0.15F, 6, Fade(Color{12, 18, 26, 255}, 0.95F));
            DrawRectangleRoundedLinesEx(cmdBox, 0.15F, 6, 2.0F, Color{132, 176, 229, 255});
            const std::string shown = commandInput.empty() ? "/" : commandInput;
            DrawText(shown.c_str(), 36, screenH - 126, 22, Color{233, 242, 255, 255});
            if((static_cast<int>(t * 2.0F) % 2) == 0) {
                const int tw = MeasureText(shown.c_str(), 22);
                DrawText("_", 36 + tw + 2, screenH - 126, 22, Color{233, 242, 255, 255});
            }
        }

        if(sim.done() && sim.reachedExit()) {
            DrawRectangle(0, 0, screenW, screenH, Fade(Color{8, 13, 18, 255}, 0.55F));
            const Rectangle panel{
                static_cast<float>(screenW / 2 - 270),
                static_cast<float>(screenH / 2 - 110),
                540.0F,
                220.0F
            };
            DrawRectangleRounded(panel, 0.16F, 10, Color{22, 34, 44, 245});
            DrawRectangleRoundedLinesEx(panel, 0.16F, 10, 3.0F, Color{132, 237, 184, 255});
            DrawText("GEWONNEN", screenW / 2 - 120, screenH / 2 - 52, 56, Color{168, 255, 204, 255});
            DrawText("Du hast das Ziel erreicht.", screenW / 2 - 150, screenH / 2 + 18, 28, Color{223, 240, 255, 255});
            DrawText("R = neuer Lauf | ESC = Menu", screenW / 2 - 142, screenH / 2 + 58, 22, Color{193, 210, 229, 255});
        }

        EndDrawing();
    }

    UnloadTexture(atlas);
    CloseWindow();
    return 0;
}
