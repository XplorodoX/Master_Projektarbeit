#pragma once

#include <raylib.h>

#include "stoneforge/simulation.hpp"

namespace stoneforge::client {

const char* itemShortLabel(stoneforge::ItemId item);
const char* itemGlyph(stoneforge::ItemId item);
Color itemTint(stoneforge::ItemId item);

bool drawButton(Rectangle rect, const char* text, bool enabled);

void drawHud(const stoneforge::Simulation& sim, int screenH, int tileSize, bool inventoryOpen, bool nearWorkbench);
void drawBottomVitals(const stoneforge::Simulation& sim, int screenW, int screenH);
void drawHotbar(const stoneforge::Simulation& sim, int screenW, int screenH);
void drawInventoryPanel(stoneforge::Simulation& sim, int screenW, bool nearWorkbench, int selectedHotbarSlot, int& dragSourceSlot, bool& dragSplitMode);

}  // namespace stoneforge::client
