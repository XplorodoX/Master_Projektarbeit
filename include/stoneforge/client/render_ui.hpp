#pragma once

#include <array>
#include <string>
#include <string_view>

#include <raylib.h>

#include "stoneforge/simulation.hpp"

namespace stoneforge::client {

struct CraftSlot {
	std::string itemId;
	int count = 0;
};

struct CraftingPanelState {
	std::array<CraftSlot, 9> slots{};
	bool craftRequested = false;
	std::string requestedRecipe = "stoneforge:planks";
};

struct CraftingPreview {
	bool valid = false;
	std::string recipe;
	std::string label;
};

const char* itemShortLabel(std::string_view itemId);
const char* itemGlyph(std::string_view itemId);
Color itemTint(std::string_view itemId);

CraftingPreview evaluateCraftingGrid(const std::array<CraftSlot, 9>& slots);
void consumeCraftingInputs(std::array<CraftSlot, 9>& slots, std::string_view recipeId);
void clearCraftingInputs(std::array<CraftSlot, 9>& slots);

bool drawButton(Rectangle rect, const char* text, bool enabled);

void drawHud(const stoneforge::Simulation& sim, int screenW, int screenH, int tileSize, bool inventoryOpen, bool nearWorkbench);
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
