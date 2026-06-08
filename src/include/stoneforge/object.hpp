#pragma once

#include <cstdint>
#include <string_view>

#include "stoneforge/types.hpp"

namespace stoneforge {

enum class ObjectDrop : std::uint8_t {
    None = 0,
    Wood = 1,
    Ore = 2,
    WorkbenchKit = 3
};

class WorldObject {
public:
    virtual ~WorldObject() = default;

    virtual TileType tileType() const = 0;
    virtual std::string_view id() const = 0;

    virtual bool isPassable() const {
        return false;
    }

    virtual bool blocksLineOfSight() const {
        return false;
    }

    virtual bool isMineable() const {
        return false;
    }

    virtual float miningHardness() const {
        return 1.0F;
    }

    virtual float miningSpeed(int axeLevel, int pickaxeLevel) const;

    virtual ObjectDrop minedDrop() const {
        return ObjectDrop::None;
    }
};

const WorldObject& objectForTile(TileType tile);

}  // namespace stoneforge
