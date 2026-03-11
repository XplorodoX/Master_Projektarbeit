#pragma once

#include <array>

#include <raylib.h>

#include "stoneforge/simulation.hpp"

namespace stoneforge::client {

struct CraftSlot {
	stoneforge::ItemId item = stoneforge::ItemId::None;
	int count = 0;
};

struct CraftingPanelState {
	std::array<CraftSlot, 9> slots{};
	bool craftRequested = false;
	stoneforge::RecipeId requestedRecipe = stoneforge::RecipeId::Planks;
};

struct CraftingPreview {
	bool valid = false;
	stoneforge::RecipeId recipe = stoneforge::RecipeId::Planks;
	const char* label = "";
};

const char* itemShortLabel(stoneforge::ItemId item);
const char* itemGlyph(stoneforge::ItemId item);
Color itemTint(stoneforge::ItemId item);

CraftingPreview evaluateCraftingGrid(const std::array<CraftSlot, 9>& slots);
void consumeCraftingInputs(std::array<CraftSlot, 9>& slots, stoneforge::RecipeId recipe);
void clearCraftingInputs(std::array<CraftSlot, 9>& slots);

bool drawButton(Rectangle rect, const char* text, bool enabled);

void drawHud(const stoneforge::Simulation& sim, int screenH, int tileSize, bool inventoryOpen, bool nearWorkbench);
void drawBottomVitals(const stoneforge::Simulation& sim, int screenW, int screenH);
void drawHotbar(const stoneforge::Simulation& sim, int screenW, int screenH);
void drawInventoryPanel(
	stoneforge::Simulation& sim,
	int screenW,
	bool nearWorkbench,
	int selectedHotbarSlot,
	int& dragSourceSlot,
	bool& dragSplitMode,
	CraftingPanelState& crafting
);

}  // namespace stoneforge::client
