#include "stoneforge/client/render_ui.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include "stoneforge/item.hpp"

namespace stoneforge::client {

namespace {

struct RecipeEntry {
    stoneforge::RecipeId recipe;
    const char* name;
    const char* cost;
    bool requiresWorkbench;
};

constexpr std::array<RecipeEntry, 7> kRecipes{ {
    {stoneforge::RecipeId::Planks, "Planks x4", "1 Wood", false},
    {stoneforge::RecipeId::Sticks, "Sticks x4", "2 Planks vertical", false},
    {stoneforge::RecipeId::Workbench, "Workbench Kit x1", "10 Planks", false},
    {stoneforge::RecipeId::AxeTier1, "Axe Lv1", "Axe shape (Planks+Sticks)", true},
    {stoneforge::RecipeId::PickaxeTier1, "Pickaxe Lv1", "Pick shape (Planks+Sticks)", true},
    {stoneforge::RecipeId::AxeTier2, "Axe Lv2", "Axe shape (Ore+Sticks)", true},
    {stoneforge::RecipeId::PickaxeTier2, "Pickaxe Lv2", "Pick shape (Ore+Sticks)", true},
} };

bool isEmpty(const CraftSlot& slot) {
    return slot.item == stoneforge::ItemId::None || slot.count <= 0;
}

int totalCount(const std::array<CraftSlot, 9>& slots, stoneforge::ItemId item) {
    int out = 0;
    for(const auto& slot : slots) {
        if(slot.item == item && slot.count > 0) {
            out += slot.count;
        }
    }
    return out;
}

bool hasOnlyItem(const std::array<CraftSlot, 9>& slots, stoneforge::ItemId item) {
    for(const auto& slot : slots) {
        if(isEmpty(slot)) {
            continue;
        }
        if(slot.item != item) {
            return false;
        }
    }
    return true;
}

std::vector<int> findStickVerticalPattern(const std::array<CraftSlot, 9>& slots) {
    for(int col = 0; col < 3; ++col) {
        for(int row = 0; row < 2; ++row) {
            const int a = row * 3 + col;
            const int b = (row + 1) * 3 + col;
            bool ok = true;
            for(int i = 0; i < 9; ++i) {
                if(i == a || i == b) {
                    if(slots[static_cast<std::size_t>(i)].item != stoneforge::ItemId::Planks || slots[static_cast<std::size_t>(i)].count < 1) {
                        ok = false;
                        break;
                    }
                } else if(!isEmpty(slots[static_cast<std::size_t>(i)])) {
                    ok = false;
                    break;
                }
            }
            if(ok) {
                return {a, b};
            }
        }
    }
    return {};
}

bool matchesToolPattern(
    const std::array<CraftSlot, 9>& slots,
    stoneforge::ItemId headMaterial,
    const std::array<stoneforge::ItemId, 9>& required
) {
    for(int i = 0; i < 9; ++i) {
        const auto need = required[static_cast<std::size_t>(i)];
        const auto& slot = slots[static_cast<std::size_t>(i)];
        if(need == stoneforge::ItemId::None) {
            if(!isEmpty(slot)) {
                return false;
            }
            continue;
        }

        const stoneforge::ItemId expected = (need == stoneforge::ItemId::Ore) ? headMaterial : need;
        if(slot.item != expected || slot.count < 1) {
            return false;
        }
    }
    return true;
}

std::vector<int> recipeSlotsForConsume(const std::array<CraftSlot, 9>& slots, stoneforge::RecipeId recipe) {
    const std::array<stoneforge::ItemId, 9> pickPattern{
        stoneforge::ItemId::Ore, stoneforge::ItemId::Ore, stoneforge::ItemId::Ore,
        stoneforge::ItemId::None, stoneforge::ItemId::Sticks, stoneforge::ItemId::None,
        stoneforge::ItemId::None, stoneforge::ItemId::Sticks, stoneforge::ItemId::None
    };

    const std::array<stoneforge::ItemId, 9> axeLeftPattern{
        stoneforge::ItemId::Ore, stoneforge::ItemId::Ore, stoneforge::ItemId::None,
        stoneforge::ItemId::Ore, stoneforge::ItemId::Sticks, stoneforge::ItemId::None,
        stoneforge::ItemId::None, stoneforge::ItemId::Sticks, stoneforge::ItemId::None
    };

    const std::array<stoneforge::ItemId, 9> axeRightPattern{
        stoneforge::ItemId::None, stoneforge::ItemId::Ore, stoneforge::ItemId::Ore,
        stoneforge::ItemId::None, stoneforge::ItemId::Sticks, stoneforge::ItemId::Ore,
        stoneforge::ItemId::None, stoneforge::ItemId::Sticks, stoneforge::ItemId::None
    };

    if(recipe == stoneforge::RecipeId::Sticks) {
        return findStickVerticalPattern(slots);
    }

    if(recipe == stoneforge::RecipeId::PickaxeTier1 && matchesToolPattern(slots, stoneforge::ItemId::Planks, pickPattern)) {
        return {0, 1, 2, 4, 7};
    }
    if(recipe == stoneforge::RecipeId::PickaxeTier2 && matchesToolPattern(slots, stoneforge::ItemId::Ore, pickPattern)) {
        return {0, 1, 2, 4, 7};
    }

    if(recipe == stoneforge::RecipeId::AxeTier1) {
        if(matchesToolPattern(slots, stoneforge::ItemId::Planks, axeLeftPattern)) {
            return {0, 1, 3, 4, 7};
        }
        if(matchesToolPattern(slots, stoneforge::ItemId::Planks, axeRightPattern)) {
            return {1, 2, 4, 5, 7};
        }
    }
    if(recipe == stoneforge::RecipeId::AxeTier2) {
        if(matchesToolPattern(slots, stoneforge::ItemId::Ore, axeLeftPattern)) {
            return {0, 1, 3, 4, 7};
        }
        if(matchesToolPattern(slots, stoneforge::ItemId::Ore, axeRightPattern)) {
            return {1, 2, 4, 5, 7};
        }
    }

    return {};
}

void reduceSlot(CraftSlot& slot, int amount) {
    slot.count = std::max(0, slot.count - amount);
    if(slot.count == 0) {
        slot.item = stoneforge::ItemId::None;
    }
}

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

const char* itemShortLabel(std::string_view itemId) {
    switch(stoneforge::itemIdFromKey(itemId)) {
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
            return "Item";
    }
}

CraftingPreview evaluateCraftingGrid(const std::array<CraftSlot, 9>& slots) {
    CraftingPreview out{};

    int occupied = 0;
    int woodSlot = -1;
    for(int i = 0; i < 9; ++i) {
        if(!isEmpty(slots[static_cast<std::size_t>(i)])) {
            ++occupied;
            if(slots[static_cast<std::size_t>(i)].item == stoneforge::ItemId::Wood) {
                woodSlot = i;
            }
        }
    }

    if(occupied == 1 && woodSlot >= 0 && slots[static_cast<std::size_t>(woodSlot)].count >= 1) {
        out.valid = true;
        out.recipe = stoneforge::RecipeId::Planks;
        out.label = "Planks x4";
        return out;
    }

    if(!findStickVerticalPattern(slots).empty()) {
        out.valid = true;
        out.recipe = stoneforge::RecipeId::Sticks;
        out.label = "Sticks x4";
        return out;
    }

    if(hasOnlyItem(slots, stoneforge::ItemId::Planks) && totalCount(slots, stoneforge::ItemId::Planks) >= 10) {
        out.valid = true;
        out.recipe = stoneforge::RecipeId::Workbench;
        out.label = "Workbench Kit x1";
        return out;
    }

    if(!recipeSlotsForConsume(slots, stoneforge::RecipeId::AxeTier1).empty()) {
        out.valid = true;
        out.recipe = stoneforge::RecipeId::AxeTier1;
        out.label = "Axe Lv1";
        return out;
    }
    if(!recipeSlotsForConsume(slots, stoneforge::RecipeId::PickaxeTier1).empty()) {
        out.valid = true;
        out.recipe = stoneforge::RecipeId::PickaxeTier1;
        out.label = "Pickaxe Lv1";
        return out;
    }
    if(!recipeSlotsForConsume(slots, stoneforge::RecipeId::AxeTier2).empty()) {
        out.valid = true;
        out.recipe = stoneforge::RecipeId::AxeTier2;
        out.label = "Axe Lv2";
        return out;
    }
    if(!recipeSlotsForConsume(slots, stoneforge::RecipeId::PickaxeTier2).empty()) {
        out.valid = true;
        out.recipe = stoneforge::RecipeId::PickaxeTier2;
        out.label = "Pickaxe Lv2";
        return out;
    }

    return out;
}

void consumeCraftingInputs(std::array<CraftSlot, 9>& slots, stoneforge::RecipeId recipe) {
    if(recipe == stoneforge::RecipeId::Planks) {
        for(auto& slot : slots) {
            if(slot.item == stoneforge::ItemId::Wood && slot.count > 0) {
                reduceSlot(slot, 1);
                return;
            }
        }
        return;
    }

    if(recipe == stoneforge::RecipeId::Workbench) {
        int remaining = 10;
        for(auto& slot : slots) {
            if(slot.item != stoneforge::ItemId::Planks || slot.count <= 0 || remaining <= 0) {
                continue;
            }
            const int take = std::min(slot.count, remaining);
            reduceSlot(slot, take);
            remaining -= take;
        }
        return;
    }

    const auto indices = recipeSlotsForConsume(slots, recipe);
    if(indices.empty()) {
        return;
    }
    for(int idx : indices) {
        reduceSlot(slots[static_cast<std::size_t>(idx)], 1);
    }
}

void clearCraftingInputs(std::array<CraftSlot, 9>& slots) {
    for(auto& slot : slots) {
        slot = CraftSlot{};
    }
}

const char* itemGlyph(std::string_view itemId) {
    switch(stoneforge::itemIdFromKey(itemId)) {
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
            return "?";
    }
}

Color itemTint(std::string_view itemId) {
    switch(stoneforge::itemIdFromKey(itemId)) {
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
            return Color{92, 112, 148, 255};
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

void drawHud(const stoneforge::Simulation& sim, int screenW, int screenH, int tileSize, bool inventoryOpen, bool nearWorkbench) {
    const int hotbarY = screenH - 108;
    const Rectangle panel = {
        static_cast<float>(screenW / 2 - 430),
        static_cast<float>(hotbarY - 94),
        860.0F,
        56.0F
    };

    DrawRectangleRounded(panel, 0.28F, 10, Fade(Color{13, 18, 24, 255}, 0.88F));
    DrawRectangleRoundedLinesEx(panel, 0.28F, 10, 2.0F, Color{90, 108, 132, 255});

    DrawText(TextFormat("Tile %dpx", tileSize), static_cast<int>(panel.x) + 16, static_cast<int>(panel.y) + 11, 18, Color{170, 190, 216, 255});
    DrawText(TextFormat("Axe %d", sim.axeLevel()), static_cast<int>(panel.x) + 112, static_cast<int>(panel.y) + 11, 18, Color{201, 156, 94, 255});
    DrawText(TextFormat("Pick %d", sim.pickaxeLevel()), static_cast<int>(panel.x) + 186, static_cast<int>(panel.y) + 11, 18, Color{236, 198, 102, 255});
    DrawText(TextFormat("Range %.2f", sim.miningRangeTiles()), static_cast<int>(panel.x) + 271, static_cast<int>(panel.y) + 11, 18, Color{178, 209, 245, 255});

    DrawText(nearWorkbench ? "Workbench: in range" : "Workbench: none", static_cast<int>(panel.x) + 398, static_cast<int>(panel.y) + 11, 18, nearWorkbench ? Color{149, 232, 166, 255} : Color{233, 145, 136, 255});
    DrawText(inventoryOpen ? "Inventory open" : "Inventory closed", static_cast<int>(panel.x) + 578, static_cast<int>(panel.y) + 11, 18, Color{186, 206, 235, 255});
    DrawText(TextFormat("Slot %d", sim.hotbarSelection() + 1), static_cast<int>(panel.x) + 736, static_cast<int>(panel.y) + 11, 18, Color{244, 223, 156, 255});

    const std::string info = "Wheel hotbar | 1-9 direct select | RMB context | TAB inventory/crafting | Ctrl+Wheel zoom";
    const int infoW = MeasureText(info.c_str(), 16);
    DrawText(info.c_str(), screenW / 2 - infoW / 2, static_cast<int>(panel.y) + 34, 16, Color{184, 197, 214, 255});
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
        if(!slot.itemId.empty() && slot.count > 0) {
            const Rectangle inner{rect.x + 6.0F, rect.y + 6.0F, rect.width - 12.0F, rect.height - 12.0F};
            DrawRectangleRounded(inner, 0.18F, 4, itemTint(slot.itemId));
            const char* glyph = itemGlyph(slot.itemId);
            const int glyphW = MeasureText(glyph, 24);
            DrawText(glyph, static_cast<int>(rect.x + (rect.width - static_cast<float>(glyphW)) * 0.5F), static_cast<int>(rect.y + 10.0F), 24, WHITE);
            DrawText(TextFormat("%d", slot.count), static_cast<int>(rect.x + 7.0F), static_cast<int>(rect.y + rect.height - 17.0F), 15, Color{241, 247, 255, 255});
        }

        DrawText(TextFormat("%d", i + 1), static_cast<int>(rect.x + rect.width - 14.0F), static_cast<int>(rect.y + 4.0F), 12, Color{180, 192, 210, 255});
    }
}

void drawInventoryPanel(
    stoneforge::Simulation& sim,
    int screenW,
    bool nearWorkbench,
    int selectedHotbarSlot,
    int& dragSourceSlot,
    bool& dragSplitMode,
    CraftingPanelState& crafting
) {
    const Rectangle panel = {static_cast<float>(screenW - 496), 18.0F, 474.0F, 650.0F};
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
    std::array<Rectangle, 9> craftRects{};
    int hoveredSlot = -1;
    int hoveredCraftSlot = -1;
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

    const int craftGridX = static_cast<int>(panel.x) + 18;
    const int craftGridY = static_cast<int>(panel.y) + 392;
    constexpr int kCraftSlotSize = 40;
    constexpr int kCraftGap = 6;
    for(int row = 0; row < 3; ++row) {
        for(int col = 0; col < 3; ++col) {
            const int idx = row * 3 + col;
            const float sx = static_cast<float>(craftGridX + col * (kCraftSlotSize + kCraftGap));
            const float sy = static_cast<float>(craftGridY + row * (kCraftSlotSize + kCraftGap));
            const Rectangle rect{sx, sy, static_cast<float>(kCraftSlotSize), static_cast<float>(kCraftSlotSize)};
            craftRects[static_cast<std::size_t>(idx)] = rect;
            if(CheckCollisionPointRec(mouse, rect)) {
                hoveredCraftSlot = idx;
            }
        }
    }

    if(dragSourceSlot < 0) {
        if(hoveredSlot >= 0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            const auto slot = sim.inventorySlot(hoveredSlot);
            if(!slot.itemId.empty() && slot.count > 0) {
                dragSourceSlot = hoveredSlot;
                dragSplitMode = false;
            }
        } else if(hoveredSlot >= 0 && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            const auto slot = sim.inventorySlot(hoveredSlot);
            if(!slot.itemId.empty() && slot.count > 1) {
                dragSourceSlot = hoveredSlot;
                dragSplitMode = true;
            }
        }

        if(hoveredCraftSlot >= 0 && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            auto& cell = crafting.slots[static_cast<std::size_t>(hoveredCraftSlot)];
            if(!isEmpty(cell)) {
                reduceSlot(cell, 1);
            }
        }
        if(hoveredCraftSlot >= 0 && IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
            crafting.slots[static_cast<std::size_t>(hoveredCraftSlot)] = CraftSlot{};
        }
    } else {
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
            const bool splitDrop = dragSplitMode || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT);
            if(hoveredCraftSlot >= 0) {
                const auto src = sim.inventorySlot(dragSourceSlot);
                auto& dst = crafting.slots[static_cast<std::size_t>(hoveredCraftSlot)];
                const stoneforge::ItemId srcLegacy = stoneforge::itemIdFromKey(src.itemId);
                if(srcLegacy != stoneforge::ItemId::None && src.count > 0 && (isEmpty(dst) || dst.item == srcLegacy)) {
                    const int addCount = splitDrop ? 1 : src.count;
                    dst.item = srcLegacy;
                    dst.count = std::min(stoneforge::Simulation::kInventoryStackLimit, dst.count + std::max(1, addCount));
                }
            } else if(hoveredSlot >= 0 && hoveredSlot != dragSourceSlot) {
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

        if(!slot.itemId.empty() && slot.count > 0) {
            const Rectangle inner{rect.x + 5.0F, rect.y + 5.0F, rect.width - 10.0F, rect.height - 10.0F};
            DrawRectangleRounded(inner, 0.18F, 4, itemTint(slot.itemId));

            const char* glyph = itemGlyph(slot.itemId);
            const int glyphW = MeasureText(glyph, 24);
            DrawText(glyph, static_cast<int>(rect.x + (rect.width - static_cast<float>(glyphW)) * 0.5F), static_cast<int>(rect.y + 9.0F), 24, Color{242, 247, 255, 255});

            DrawText(TextFormat("%d", slot.count), static_cast<int>(rect.x + 8.0F), static_cast<int>(rect.y + rect.height - 18.0F), 16, Color{242, 247, 255, 255});
            if(slot.count >= stoneforge::Simulation::kInventoryStackLimit) {
                DrawText("MAX", static_cast<int>(rect.x + rect.width - 32.0F), static_cast<int>(rect.y + rect.height - 18.0F), 12, Color{255, 230, 172, 255});
            }
        }
    }

    DrawText("Drag LMB: move/swap | Drag RMB: split", static_cast<int>(panel.x) + 18, static_cast<int>(panel.y) + 318, 16, Color{176, 200, 226, 255});
    DrawText("Drop from inventory into crafting grid", static_cast<int>(panel.x) + 18, static_cast<int>(panel.y) + 338, 16, Color{176, 200, 226, 255});
    DrawText(TextFormat("Stack limit: %d", stoneforge::Simulation::kInventoryStackLimit), static_cast<int>(panel.x) + 18, static_cast<int>(panel.y) + 358, 16, Color{164, 174, 191, 255});

    if(dragSourceSlot >= 0) {
        const auto src = sim.inventorySlot(dragSourceSlot);
        if(!src.itemId.empty() && src.count > 0) {
            const Rectangle ghost{mouse.x + 12.0F, mouse.y + 12.0F, 96.0F, 28.0F};
            DrawRectangleRounded(ghost, 0.2F, 6, Fade(Color{13, 18, 24, 255}, 0.9F));
            DrawRectangleRoundedLinesEx(ghost, 0.2F, 6, 2.0F, Color{118, 171, 220, 255});
            DrawText(TextFormat("%s x%d", itemShortLabel(src.itemId), src.count), static_cast<int>(ghost.x) + 8, static_cast<int>(ghost.y) + 6, 16, Color{230, 238, 251, 255});
        }
    }

    DrawText("Crafting Table", static_cast<int>(panel.x) + 18, static_cast<int>(panel.y) + 362, 22, Color{208, 223, 246, 255});

    for(int i = 0; i < 9; ++i) {
        const auto rect = craftRects[static_cast<std::size_t>(i)];
        const auto& cell = crafting.slots[static_cast<std::size_t>(i)];
        const bool hovered = i == hoveredCraftSlot;
        DrawRectangleRounded(rect, 0.18F, 6, hovered ? Color{53, 70, 90, 255} : Color{33, 45, 60, 255});
        DrawRectangleRoundedLinesEx(rect, 0.18F, 6, 2.0F, Color{102, 125, 151, 255});
        if(!isEmpty(cell)) {
            const Rectangle inner{rect.x + 4.0F, rect.y + 4.0F, rect.width - 8.0F, rect.height - 8.0F};
            DrawRectangleRounded(inner, 0.18F, 4, itemTint(stoneforge::itemById(cell.item).key()));
            DrawText(itemGlyph(stoneforge::itemById(cell.item).key()), static_cast<int>(rect.x + 14.0F), static_cast<int>(rect.y + 8.0F), 20, WHITE);
            DrawText(TextFormat("%d", cell.count), static_cast<int>(rect.x + 6.0F), static_cast<int>(rect.y + rect.height - 16.0F), 12, Color{240, 247, 255, 255});
        }
    }

    const auto preview = evaluateCraftingGrid(crafting.slots);
    const bool canCraftNow = preview.valid && sim.canCraft(preview.recipe);

    const Rectangle outRect{
        static_cast<float>(craftGridX + 3 * (kCraftSlotSize + kCraftGap) + 30),
        static_cast<float>(craftGridY + kCraftSlotSize + 6),
        52.0F,
        52.0F
    };
    DrawRectangleRounded(outRect, 0.18F, 6, Color{33, 45, 60, 255});
    DrawRectangleRoundedLinesEx(outRect, 0.18F, 6, 2.0F, canCraftNow ? Color{145, 234, 165, 255} : Color{106, 124, 146, 255});
    DrawText("=>", static_cast<int>(outRect.x - 24.0F), static_cast<int>(outRect.y + 16.0F), 24, Color{173, 192, 216, 255});
    if(preview.valid) {
        DrawText("OK", static_cast<int>(outRect.x + 15.0F), static_cast<int>(outRect.y + 16.0F), 20, canCraftNow ? Color{158, 239, 177, 255} : Color{208, 177, 130, 255});
    }

    Rectangle craftBtn{
        static_cast<float>(craftGridX + 170),
        static_cast<float>(craftGridY + 108),
        96.0F,
        34.0F
    };
    Rectangle clearBtn{
        static_cast<float>(craftGridX + 272),
        static_cast<float>(craftGridY + 108),
        88.0F,
        34.0F
    };
    if(drawButton(craftBtn, "Craft", canCraftNow)) {
        crafting.craftRequested = true;
        crafting.requestedRecipe = preview.recipe;
    }

    bool anyCraftInput = false;
    for(const auto& slot : crafting.slots) {
        if(!isEmpty(slot)) {
            anyCraftInput = true;
            break;
        }
    }
    if(drawButton(clearBtn, "Clear", anyCraftInput)) {
        clearCraftingInputs(crafting.slots);
    }

    const char* previewLabel = preview.valid ? preview.label : "No valid recipe";
    DrawText(previewLabel, craftGridX + 170, craftGridY + 78, 16, canCraftNow ? Color{158, 239, 177, 255} : Color{186, 196, 212, 255});
    if(preview.valid && !canCraftNow) {
        DrawText("Missing resources / workbench", craftGridX + 170, craftGridY + 94, 14, Color{232, 185, 115, 255});
    }

    for(int i = 0; i < static_cast<int>(kRecipes.size()); ++i) {
        const auto& recipe = kRecipes[static_cast<std::size_t>(i)];
        const bool craftable = sim.canCraft(recipe.recipe);
        const bool needsBenchNow = recipe.requiresWorkbench && !nearWorkbench;
        Color textColor = craftable ? Color{158, 239, 177, 255} : Color{175, 183, 197, 255};
        if(needsBenchNow) {
            textColor = Color{232, 185, 115, 255};
        }

        const int y = static_cast<int>(panel.y) + 548 + i * 14;
        DrawText(recipe.name, static_cast<int>(panel.x) + 18, y, 14, textColor);
        DrawText(recipe.cost, static_cast<int>(panel.x) + 250, y, 14, Color{170, 180, 194, 255});
        if(recipe.requiresWorkbench) {
            DrawText("WB", static_cast<int>(panel.x) + 430, y, 14, Color{202, 162, 118, 255});
        }
    }
}

}  // namespace stoneforge::client
