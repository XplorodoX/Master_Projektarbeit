#pragma once

#include <cstdint>

namespace stoneforge {

struct Vec2i {
    int x = 0;
    int y = 0;

    bool operator==(const Vec2i& other) const {
        return x == other.x && y == other.y;
    }
};

enum class TileType : std::uint8_t {
    Empty = 0,
    Wall = 1,
    Resource = 2,
    Exit = 3,
    Tree = 4,
    Workbench = 5,
    WoodWall = 6,
    WoodLog = 7,
    StructureGrassland = 8,
    StructureForest = 9,
    StructureDesert = 10,
    StructureMountain = 11,
    StructureSteppe = 12,
    StructureTundra = 13,
    StructureHelle = 14
};

enum class Action : int {
    MoveUp = 0,
    MoveDown = 1,
    MoveLeft = 2,
    MoveRight = 3,
    Mine = 4,
    Place = 5,
    Use = 6,
    Wait = 7,
    Noop = 8
};

struct StepResult {
    float reward = 0.0F;
    bool done = false;
    bool reachedExit = false;
    int step = 0;
};

}  // namespace stoneforge
