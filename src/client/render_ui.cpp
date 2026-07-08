#include "stoneforge/client/render_ui.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "stoneforge/item.hpp"
#include "stoneforge/recipe.hpp"

namespace stoneforge::client {

namespace {

bool isEmpty(const CraftSlot& slot) {
    return slot.itemId.empty() || slot.count <= 0;
}

std::unordered_map<std::string, int> slotCounts(const std::array<CraftSlot, 9>& slots) {
    std::unordered_map<std::string, int> counts;
    for(const auto& slot : slots) {
        if(isEmpty(slot)) {
            continue;
        }
        const std::string id = stoneforge::normalizeItemKey(slot.itemId);
        if(id.empty()) {
            continue;
        }
        counts[id] += slot.count;
    }
    return counts;
}

bool recipeMatchesGrid(const std::array<CraftSlot, 9>& slots, std::string_view recipeId) {
    const auto* def = stoneforge::recipeCatalog().find(recipeId);
    if(def == nullptr) {
        return false;
    }

    std::unordered_map<std::string, int> expected;
    for(const auto& input : def->inputs()) {
        const std::string id = stoneforge::normalizeItemKey(input.itemId);
        if(id.empty() || input.count <= 0) {
            continue;
        }
        expected[id] += input.count;
    }

    const auto actual = slotCounts(slots);
    if(actual.size() != expected.size()) {
        return false;
    }

    for(const auto& [id, count] : expected) {
        const auto it = actual.find(id);
        if(it == actual.end() || it->second != count) {
            return false;
        }
    }

    return true;
}

std::vector<const stoneforge::RecipeBase*> uiRecipes() {
    auto recipes = stoneforge::recipeCatalog().all();
    std::sort(recipes.begin(), recipes.end(), [](const stoneforge::RecipeBase* a, const stoneforge::RecipeBase* b) {
        if(a->requiresWorkbench() != b->requiresWorkbench()) {
            return !a->requiresWorkbench();
        }
        return a->id() < b->id();
    });
    return recipes;
}

std::string recipeCostLabel(const stoneforge::RecipeBase& recipe) {
    if(recipe.inputs().empty()) {
        return "Free";
    }

    std::ostringstream out;
    bool first = true;
    for(const auto& input : recipe.inputs()) {
        if(!first) {
            out << ", ";
        }
        first = false;
        out << input.count << " " << stoneforge::itemDisplayName(input.itemId);
    }
    return out.str();
}

void reduceSlot(CraftSlot& slot, int amount) {
    slot.count = std::max(0, slot.count - amount);
    if(slot.count == 0) {
        slot.itemId.clear();
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

std::string itemShortLabel(std::string_view itemId) {
    std::string label = stoneforge::itemDisplayName(itemId);
    if(label.size() > 12) {
        label.resize(12);
    }
    return label;
}

CraftingPreview evaluateCraftingGrid(const std::array<CraftSlot, 9>& slots) {
    CraftingPreview out{};

    for(const auto* def : uiRecipes()) {
        if(def == nullptr) {
            continue;
        }
        if(!recipeMatchesGrid(slots, def->id())) {
            continue;
        }
        out.valid = true;
        out.recipe = std::string(def->id());
        out.label = std::string(def->label());
        return out;
    }

    return out;
}

void consumeCraftingInputs(std::array<CraftSlot, 9>& slots, std::string_view recipeId) {
    const auto* def = stoneforge::recipeCatalog().find(recipeId);
    if(def == nullptr) {
        return;
    }

    for(const auto& input : def->inputs()) {
        const std::string wanted = stoneforge::normalizeItemKey(input.itemId);
        int remaining = input.count;
        for(auto& slot : slots) {
            if(remaining <= 0) {
                break;
            }
            if(isEmpty(slot)) {
                continue;
            }
            if(stoneforge::normalizeItemKey(slot.itemId) != wanted) {
                continue;
            }

            const int take = std::min(slot.count, remaining);
            reduceSlot(slot, take);
            remaining -= take;
        }
    }
}

void clearCraftingInputs(std::array<CraftSlot, 9>& slots) {
    for(auto& slot : slots) {
        slot = CraftSlot{};
    }
}

std::string itemGlyph(std::string_view itemId) {
    return stoneforge::itemGlyphText(itemId);
}

Color itemTint(std::string_view itemId) {
    const auto rgba = stoneforge::itemTintRgba(itemId);
    return Color{rgba[0], rgba[1], rgba[2], rgba[3]};
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
    (void)sim;
    (void)screenW;
    (void)screenH;
    (void)tileSize;
    (void)inventoryOpen;
    (void)nearWorkbench;
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
    const int hotbarSlots = sim.hotbarSlotCount();
    const int totalW = hotbarSlots * kSlotSize + (hotbarSlots - 1) * kGap;
    const int startX = (screenW - totalW) / 2;
    const int y = screenH - 108;

    DrawRectangleRounded(
        Rectangle{static_cast<float>(startX - 14), static_cast<float>(y - 10), static_cast<float>(totalW + 28), 72.0F},
        0.2F,
        8,
        Fade(Color{13, 17, 24, 255}, 0.88F)
    );

    for(int i = 0; i < hotbarSlots; ++i) {
        const int x = startX + i * (kSlotSize + kGap);
        const Rectangle rect{static_cast<float>(x), static_cast<float>(y), static_cast<float>(kSlotSize), static_cast<float>(kSlotSize)};

        const bool active = i == sim.hotbarSelection();
        DrawRectangleRounded(rect, 0.18F, 6, active ? Color{60, 82, 109, 255} : Color{34, 44, 58, 255});
        DrawRectangleRoundedLinesEx(rect, 0.18F, 6, active ? 3.0F : 2.0F, active ? Color{244, 223, 156, 255} : Color{90, 105, 126, 255});

        const auto slot = sim.hotbarSlot(i);
        if(!slot.itemId.empty() && slot.count > 0) {
            const Rectangle inner{rect.x + 6.0F, rect.y + 6.0F, rect.width - 12.0F, rect.height - 12.0F};
            DrawRectangleRounded(inner, 0.18F, 4, itemTint(slot.itemId));
            const std::string glyph = itemGlyph(slot.itemId);
            const int glyphW = MeasureText(glyph.c_str(), 24);
            DrawText(glyph.c_str(), static_cast<int>(rect.x + (rect.width - static_cast<float>(glyphW)) * 0.5F), static_cast<int>(rect.y + 10.0F), 24, WHITE);
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
    constexpr int kSlotSize = 46;
    constexpr int kSlotGap = 8;
    const int slotCount = sim.inventorySlotCount();
    const int kRows = std::max(1, (slotCount + kCols - 1) / kCols);

    const int gridX = static_cast<int>(panel.x) + 18;
    const int gridY = static_cast<int>(panel.y) + 154;

    std::vector<Rectangle> slotRects(static_cast<std::size_t>(sim.inventorySlotCount()));
    std::array<Rectangle, 9> craftRects{};
    int hoveredSlot = -1;
    int hoveredCraftSlot = -1;
    const Vector2 mouse = GetMousePosition();

    for(int row = 0; row < kRows; ++row) {
        for(int col = 0; col < kCols; ++col) {
            const int idx = row * kCols + col;
            if(idx >= slotCount) {
                continue;
            }
            const float sx = static_cast<float>(gridX + col * (kSlotSize + kSlotGap));
            const float sy = static_cast<float>(gridY + row * (kSlotSize + kSlotGap));
            const Rectangle rect{sx, sy, static_cast<float>(kSlotSize), static_cast<float>(kSlotSize)};
            if(static_cast<std::size_t>(idx) < slotRects.size()) {
                slotRects[static_cast<std::size_t>(idx)] = rect;
            }
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
                if(!src.itemId.empty() && src.count > 0 && (isEmpty(dst) || stoneforge::normalizeItemKey(dst.itemId) == stoneforge::normalizeItemKey(src.itemId))) {
                    const int addCount = splitDrop ? 1 : src.count;
                    dst.itemId = stoneforge::normalizeItemKey(src.itemId);
                    const int maxStack = std::max(1, stoneforge::itemMaxStack(dst.itemId));
                    dst.count = std::min(maxStack, dst.count + std::max(1, addCount));
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
        const bool hotbarSlot = idx < sim.hotbarSlotCount();
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

            const std::string glyph = itemGlyph(slot.itemId);
            const int glyphW = MeasureText(glyph.c_str(), 24);
            DrawText(glyph.c_str(), static_cast<int>(rect.x + (rect.width - static_cast<float>(glyphW)) * 0.5F), static_cast<int>(rect.y + 9.0F), 24, Color{242, 247, 255, 255});

            DrawText(TextFormat("%d", slot.count), static_cast<int>(rect.x + 8.0F), static_cast<int>(rect.y + rect.height - 18.0F), 16, Color{242, 247, 255, 255});
            if(slot.count >= sim.inventoryStackLimit()) {
                DrawText("MAX", static_cast<int>(rect.x + rect.width - 32.0F), static_cast<int>(rect.y + rect.height - 18.0F), 12, Color{255, 230, 172, 255});
            }
        }
    }

    DrawText("Drag LMB: move/swap | Drag RMB: split", static_cast<int>(panel.x) + 18, static_cast<int>(panel.y) + 318, 16, Color{176, 200, 226, 255});
    DrawText("Drop from inventory into crafting grid", static_cast<int>(panel.x) + 18, static_cast<int>(panel.y) + 338, 16, Color{176, 200, 226, 255});
    DrawText(TextFormat("Stack limit: %d", sim.inventoryStackLimit()), static_cast<int>(panel.x) + 18, static_cast<int>(panel.y) + 358, 16, Color{164, 174, 191, 255});

    if(dragSourceSlot >= 0) {
        const auto src = sim.inventorySlot(dragSourceSlot);
        if(!src.itemId.empty() && src.count > 0) {
            const Rectangle ghost{mouse.x + 12.0F, mouse.y + 12.0F, 96.0F, 28.0F};
            DrawRectangleRounded(ghost, 0.2F, 6, Fade(Color{13, 18, 24, 255}, 0.9F));
            DrawRectangleRoundedLinesEx(ghost, 0.2F, 6, 2.0F, Color{118, 171, 220, 255});
            const std::string shortLabel = itemShortLabel(src.itemId);
            DrawText(TextFormat("%s x%d", shortLabel.c_str(), src.count), static_cast<int>(ghost.x) + 8, static_cast<int>(ghost.y) + 6, 16, Color{230, 238, 251, 255});
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
            DrawRectangleRounded(inner, 0.18F, 4, itemTint(cell.itemId));
            const std::string glyph = itemGlyph(cell.itemId);
            DrawText(glyph.c_str(), static_cast<int>(rect.x + 14.0F), static_cast<int>(rect.y + 8.0F), 20, WHITE);
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

    const char* previewLabel = preview.valid ? preview.label.c_str() : "No valid recipe";
    DrawText(previewLabel, craftGridX + 170, craftGridY + 78, 16, canCraftNow ? Color{158, 239, 177, 255} : Color{186, 196, 212, 255});
    if(preview.valid && !canCraftNow) {
        DrawText("Missing resources / workbench", craftGridX + 170, craftGridY + 94, 14, Color{232, 185, 115, 255});
    }

    const auto recipeList = uiRecipes();
    for(int i = 0; i < static_cast<int>(recipeList.size()) && i < 7; ++i) {
        const auto* recipe = recipeList[static_cast<std::size_t>(i)];
        if(recipe == nullptr) {
            continue;
        }

        const bool craftable = sim.canCraft(recipe->id());
        const bool needsBenchNow = recipe->requiresWorkbench() && !nearWorkbench;
        Color textColor = craftable ? Color{158, 239, 177, 255} : Color{175, 183, 197, 255};
        if(needsBenchNow) {
            textColor = Color{232, 185, 115, 255};
        }

        const int y = static_cast<int>(panel.y) + 548 + i * 14;
        const std::string label(recipe->label());
        const std::string cost = recipeCostLabel(*recipe);
        DrawText(label.c_str(), static_cast<int>(panel.x) + 18, y, 14, textColor);
        DrawText(cost.c_str(), static_cast<int>(panel.x) + 250, y, 14, Color{170, 180, 194, 255});
        if(recipe->requiresWorkbench()) {
            DrawText("WB", static_cast<int>(panel.x) + 430, y, 14, Color{202, 162, 118, 255});
        }
    }
}

}  // namespace stoneforge::client
