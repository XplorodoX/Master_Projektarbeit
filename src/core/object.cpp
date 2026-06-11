#include "stoneforge/object.hpp"

namespace stoneforge {

float WorldObject::miningSpeed(int /*axeLevel*/, int /*pickaxeLevel*/) const {
    return 0.1F;
}

namespace {

class EmptyObject final : public WorldObject {
public:
    TileType tileType() const override {
        return TileType::Empty;
    }

    std::string_view id() const override {
        return "stoneforge:empty";
    }

    bool isPassable() const override {
        return true;
    }
};

class ExitObject final : public WorldObject {
public:
    TileType tileType() const override {
        return TileType::Exit;
    }

    std::string_view id() const override {
        return "stoneforge:exit";
    }

    bool isPassable() const override {
        return true;
    }
};

class WallObject final : public WorldObject {
public:
    TileType tileType() const override {
        return TileType::Wall;
    }

    std::string_view id() const override {
        return "stoneforge:wall";
    }

    bool blocksLineOfSight() const override {
        return true;
    }
};

class ResourceObject final : public WorldObject {
public:
    TileType tileType() const override {
        return TileType::Resource;
    }

    std::string_view id() const override {
        return "stoneforge:resource";
    }

    bool isMineable() const override {
        return true;
    }

    float miningHardness() const override {
        return 6.0F;
    }

    float miningSpeed(int /*axeLevel*/, int pickaxeLevel) const override {
        if(pickaxeLevel >= 2) {
            return 0.45F;
        }
        if(pickaxeLevel >= 1) {
            return 0.22F;
        }
        return 0.08F;
    }

    ObjectDrop minedDrop() const override {
        return ObjectDrop::Ore;
    }
};

class TreeObject final : public WorldObject {
public:
    TileType tileType() const override {
        return TileType::Tree;
    }

    std::string_view id() const override {
        return "stoneforge:tree";
    }

    bool isMineable() const override {
        return true;
    }

    float miningHardness() const override {
        return 2.2F;
    }

    float miningSpeed(int axeLevel, int /*pickaxeLevel*/) const override {
        if(axeLevel >= 2) {
            return 0.78F;
        }
        if(axeLevel >= 1) {
            return 0.45F;
        }
        return 0.22F;
    }

    ObjectDrop minedDrop() const override {
        return ObjectDrop::Wood;
    }
};

class WorkbenchObject final : public WorldObject {
public:
    TileType tileType() const override {
        return TileType::Workbench;
    }

    std::string_view id() const override {
        return "stoneforge:workbench";
    }

    bool isMineable() const override {
        return true;
    }

    float miningHardness() const override {
        return 3.6F;
    }

    float miningSpeed(int axeLevel, int /*pickaxeLevel*/) const override {
        if(axeLevel >= 2) {
            return 0.65F;
        }
        if(axeLevel >= 1) {
            return 0.38F;
        }
        return 0.16F;
    }

    ObjectDrop minedDrop() const override {
        return ObjectDrop::WorkbenchKit;
    }
};

class WoodWallObject final : public WorldObject {
public:
    TileType tileType() const override {
        return TileType::WoodWall;
    }

    std::string_view id() const override {
        return "stoneforge:wood_wall";
    }

    bool blocksLineOfSight() const override {
        return true;
    }
};

class WoodLogObject final : public WorldObject {
public:
    TileType tileType() const override {
        return TileType::WoodLog;
    }

    std::string_view id() const override {
        return "stoneforge:wood_log";
    }

    bool blocksLineOfSight() const override {
        return true;
    }
};

class StructureObject final : public WorldObject {
public:
    StructureObject(TileType tileType, const char* id) : tileType_(tileType), id_(id) {}

    TileType tileType() const override {
        return tileType_;
    }

    std::string_view id() const override {
        return id_;
    }

    bool blocksLineOfSight() const override {
        return true;
    }

private:
    TileType tileType_;
    const char* id_;
};

}  // namespace

const WorldObject& objectForTile(TileType tile) {
    static EmptyObject empty;
    static WallObject wall;
    static ResourceObject resource;
    static ExitObject exit;
    static TreeObject tree;
    static WorkbenchObject workbench;
    static WoodWallObject woodWall;
    static WoodLogObject woodLog;
    static StructureObject grasslandStructure{TileType::StructureGrassland, "stoneforge:structure_grassland"};
    static StructureObject forestStructure{TileType::StructureForest, "stoneforge:structure_forest"};
    static StructureObject desertStructure{TileType::StructureDesert, "stoneforge:structure_desert"};
    static StructureObject mountainStructure{TileType::StructureMountain, "stoneforge:structure_mountain"};
    static StructureObject steppeStructure{TileType::StructureSteppe, "stoneforge:structure_steppe"};
    static StructureObject tundraStructure{TileType::StructureTundra, "stoneforge:structure_tundra"};
    static StructureObject hellStructure{TileType::StructureHelle, "stoneforge:structure_helle"};

    switch(tile) {
        case TileType::Empty:
            return empty;
        case TileType::Wall:
            return wall;
        case TileType::Resource:
            return resource;
        case TileType::Exit:
            return exit;
        case TileType::Tree:
            return tree;
        case TileType::Workbench:
            return workbench;
        case TileType::WoodWall:
            return woodWall;
        case TileType::WoodLog:
            return woodLog;
        case TileType::StructureGrassland:
            return grasslandStructure;
        case TileType::StructureForest:
            return forestStructure;
        case TileType::StructureDesert:
            return desertStructure;
        case TileType::StructureMountain:
            return mountainStructure;
        case TileType::StructureSteppe:
            return steppeStructure;
        case TileType::StructureTundra:
            return tundraStructure;
        case TileType::StructureHelle:
            return hellStructure;
        default:
            return empty;
    }
}

}  // namespace stoneforge
