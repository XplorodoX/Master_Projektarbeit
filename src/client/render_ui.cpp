#include "stoneforge/client/render_ui.hpp"

#include <algorithm>
#include <array>
#include <string>

namespace stoneforge::client {

namespace {

struct RecipeEntry {
    stoneforge::RecipeId recipe;
    const char* keyHint;
    const char* name;
    const char* cost;
    bool requiresWorkbench;
};

constexpr std::array<RecipeEntry, 7> kRecipes{ {
    {stoneforge::RecipeId::Planks, "[F1]", "Planks x4", "1 Wood", false},
    {stoneforge::RecipeId::Sticks, "[F2]", "Sticks x4", "2 Planks", false},
    {stoneforge::RecipeId::Workbench, "[F3]", "Workbench Kit x1", "10 Planks", false},
    {stoneforge::RecipeId::AxeTier1, "[F4]", "Axe Lv1", "3 Planks + 2 Sticks", true},
    {stoneforge::RecipeId::PickaxeTier1, "[F5]", "Pickaxe Lv1", "3 Planks + 2 Sticks", true},
    {stoneforge::RecipeId::AxeTier2, "[F6]", "Axe Lv2", "3 Ore + 2 Sticks", true},
    {stoneforge::RecipeId::PickaxeTier2, "[F7]", "Pickaxe Lv2", "3 Ore + 2 Sticks", true},
} };

void drawHeartIcon(int x, int y, int size, bool filled) {
    const Color fill = filled ? Color{230, 74, 74, 255} : Color{72, 52, 56, 255};
    const Color edge = filled ? Color{255, 172, 172, 255} : Color{103, 82, 90, 255};

    DrawCircle(x + size / 4, y + size / 4, static_cast<float>(size) * 0.26F, fill);
    DrawCircle(x + (3 * size) / 4, y + size / 4, static_cast<float>(size) * 0.26F, fill);
    DrawTriangle(
        Vector2{static_cast<float>(x + size / 2), static_cast<float>(y + size)},
        Vector2{static_cast<float>(x), static_cast<float>(y + size / 3)},
        Vector2{static_cast<float>(x + size), static_cast<float>(y + size / 3)},
        fill
    );

    DrawCircleLines(x + size / 4, y + size / 4, static_cast<float>(size) * 0.26F, edge);
    DrawCircleLines(x + (3 * size) / 4, y + size / 4, static_cast<float>(size) * 0.26F, edge);
    DrawLine(x, y + size / 3, x + size / 2, y + size, edge);
    DrawLine(x + size, y + size / 3, x + size / 2, y + size, edge);
}

}  // namespace

const char* itemShortLabel(stoneforge::ItemId item) {
    switch(item) {
        case stoneforge::ItemId::Wood:
            return "Wood";
        case stoneforge::ItemId::Planks:
            return "Plank";
        case stoneforge::ItemId::Sticks:
            return "Stick";
        case stoneforge::ItemId::Ore:
            return "Ore";
        case stoneforge::ItemId::WorkbenchKit:
            return "Bench";
        default:
            return "-";
    }
}

const char* itemGlyph(stoneforge::ItemId item) {
    switch(item) {
        case stoneforge::ItemId::Wood:
            return "W";
        case stoneforge::ItemId::Planks:
            return "P";
        case stoneforge::ItemId::Sticks:
            return "S";
        case stoneforge::ItemId::Ore:
            return "O";
        case stoneforge::ItemId::WorkbenchKit:
            return "B";
        default:
            return "";
    }
}

Color itemTint(stoneforge::ItemId item) {
    switch(item) {
        case stoneforge::ItemId::Wood:
            return Color{156, 109, 71, 255};
        case stoneforge::ItemId::Planks:
            return Color{187, 141, 97, 255};
        case stoneforge::ItemId::Sticks:
            return Color{136, 99, 72, 255};
        case stoneforge::ItemId::Ore:
            return Color{185, 162, 93, 255};
        case stoneforge::ItemId::WorkbenchKit:
            return Color{130, 95, 70, 255};
        default:
            return Color{80, 86, 100, 255};
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

void drawHud(const stoneforge::Simulation& sim, int screenH, int tileSize, bool inventoryOpen, bool nearWorkbench) {
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

void drawBottomVitals(const stoneforge::Simulation& sim, int screenW, int screenH) {
    constexpr int kHeartCount = 10;
    constexpr int kHeartSize = 18;
    constexpr int kHeartGap = 4;

    const int hotbarY = screenH - 108;
    const int heartsY = hotbarY - 40;
    const int heartsW = kHeartCount * kHeartSize + (kHeartCount - 1) * kHeartGap;
    const int heartsX = screenW / 2 - heartsW / 2;

    DrawRectangleRounded(
        Rectangle{static_cast<float>(heartsX - 14), static_cast<float>(heartsY - 10), static_cast<float>(heartsW + 28), 66.0F},
        0.2F,
        8,
        Fade(Color{13, 17, 24, 255}, 0.84F)
    );

    const int hpClamped = std::clamp(sim.hp(), 0, kHeartCount);
    for(int i = 0; i < kHeartCount; ++i) {
        const int x = heartsX + i * (kHeartSize + kHeartGap);
        drawHeartIcon(x, heartsY, kHeartSize, i < hpClamped);
    }

    const int barW = heartsW;
    const int barH = 10;
    const int barX = heartsX;
    const int barY = heartsY + 28;
    DrawRectangleRounded(Rectangle{static_cast<float>(barX), static_cast<float>(barY), static_cast<float>(barW), static_cast<float>(barH)}, 0.35F, 8, Color{44, 56, 72, 255});
    const float energy01 = std::clamp(static_cast<float>(sim.energy()) / 100.0F, 0.0F, 1.0F);
    DrawRectangleRounded(Rectangle{static_cast<float>(barX), static_cast<float>(barY), static_cast<float>(barW) * energy01, static_cast<float>(barH)}, 0.35F, 8, Color{95, 179, 255, 255});
    DrawText(TextFormat("Energy %d", sim.energy()), barX + barW / 2 - 42, barY - 2, 14, Color{218, 231, 247, 255});
}

void drawHotbar(const stoneforge::Simulation& sim, int screenW, int screenH) {
    constexpr int kSlotSize = 52;
    constexpr int kGap = 8;
    const int totalW = stoneforge::Simulation::kHotbarSlotCount * kSlotSize + (stoneforge::Simulation::kHotbarSlotCount - 1) * kGap;
    const int startX = (screenW - totalW) / 2;
    const int y = screenH - 108;

    DrawRectangleRounded(
        Rectangle{static_cast<float>(startX - 14), static_cast<float>(y - 10), static_cast<float>(totalW + 28), 72.0F},
        0.2F,
        8,
        Fade(Color{13, 17, 24, 255}, 0.88F)
    );

    for(int i = 0; i < stoneforge::Simulation::kHotbarSlotCount; ++i) {
        const int x = startX + i * (kSlotSize + kGap);
        const Rectangle rect{static_cast<float>(x), static_cast<float>(y), static_cast<float>(kSlotSize), static_cast<float>(kSlotSize)};

        const bool active = i == sim.hotbarSelection();
        DrawRectangleRounded(rect, 0.18F, 6, active ? Color{60, 82, 109, 255} : Color{34, 44, 58, 255});
        DrawRectangleRoundedLinesEx(rect, 0.18F, 6, active ? 3.0F : 2.0F, active ? Color{244, 223, 156, 255} : Color{90, 105, 126, 255});

        const auto slot = sim.hotbarSlot(i);
        if(slot.item != stoneforge::ItemId::None && slot.count > 0) {
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

void drawInventoryPanel(stoneforge::Simulation& sim, int screenW, bool nearWorkbench, int selectedHotbarSlot, int& dragSourceSlot, bool& dragSplitMode) {
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

    std::array<Rectangle, stoneforge::Simulation::kInventorySlotCount> slotRects{};
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
            if(slot.item != stoneforge::ItemId::None && slot.count > 0) {
                dragSourceSlot = hoveredSlot;
                dragSplitMode = false;
            }
        } else if(hoveredSlot >= 0 && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            const auto slot = sim.inventorySlot(hoveredSlot);
            if(slot.item != stoneforge::ItemId::None && slot.count > 1) {
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
        const bool hotbarSlot = idx < stoneforge::Simulation::kHotbarSlotCount;
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

        if(slot.item != stoneforge::ItemId::None && slot.count > 0) {
            const Rectangle inner{rect.x + 5.0F, rect.y + 5.0F, rect.width - 10.0F, rect.height - 10.0F};
            DrawRectangleRounded(inner, 0.18F, 4, itemTint(slot.item));

            const char* glyph = itemGlyph(slot.item);
            const int glyphW = MeasureText(glyph, 24);
            DrawText(glyph, static_cast<int>(rect.x + (rect.width - static_cast<float>(glyphW)) * 0.5F), static_cast<int>(rect.y + 9.0F), 24, Color{242, 247, 255, 255});

            DrawText(TextFormat("%d", slot.count), static_cast<int>(rect.x + 8.0F), static_cast<int>(rect.y + rect.height - 18.0F), 16, Color{242, 247, 255, 255});
            if(slot.count >= stoneforge::Simulation::kInventoryStackLimit) {
                DrawText("MAX", static_cast<int>(rect.x + rect.width - 32.0F), static_cast<int>(rect.y + rect.height - 18.0F), 12, Color{255, 230, 172, 255});
            }
        }
    }

    DrawText("Drag LMB: move/swap | Drag RMB: split", static_cast<int>(panel.x) + 18, static_cast<int>(panel.y) + 318, 16, Color{176, 200, 226, 255});
    DrawText(TextFormat("Stack limit: %d", stoneforge::Simulation::kInventoryStackLimit), static_cast<int>(panel.x) + 18, static_cast<int>(panel.y) + 338, 16, Color{164, 174, 191, 255});

    if(dragSourceSlot >= 0) {
        const auto src = sim.inventorySlot(dragSourceSlot);
        if(src.item != stoneforge::ItemId::None && src.count > 0) {
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

}  // namespace stoneforge::client
