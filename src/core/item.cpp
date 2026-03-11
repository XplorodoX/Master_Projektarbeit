#include "stoneforge/item.hpp"

#include "stoneforge/simulation.hpp"

namespace stoneforge {

PlaceableItem::PlaceableItem(ItemId id, std::string_view key, std::string_view displayName, TileType placeTile)
    : id_(id), key_(key), displayName_(displayName), placeTile_(placeTile) {}

ItemId PlaceableItem::id() const {
    return id_;
}

std::string_view PlaceableItem::key() const {
    return key_;
}

std::string_view PlaceableItem::displayName() const {
    return displayName_;
}

bool PlaceableItem::isPlaceable() const {
    return true;
}

TileType PlaceableItem::placementTile() const {
    return placeTile_;
}

MaterialItem::MaterialItem(ItemId id, std::string_view key, std::string_view displayName)
    : id_(id), key_(key), displayName_(displayName) {}

ItemId MaterialItem::id() const {
    return id_;
}

std::string_view MaterialItem::key() const {
    return key_;
}

std::string_view MaterialItem::displayName() const {
    return displayName_;
}

ToolItem::ToolItem(ItemId id, std::string_view key, std::string_view displayName)
    : id_(id), key_(key), displayName_(displayName) {}

ItemId ToolItem::id() const {
    return id_;
}

std::string_view ToolItem::key() const {
    return key_;
}

std::string_view ToolItem::displayName() const {
    return displayName_;
}

bool ToolItem::isTool() const {
    return true;
}

int ToolItem::maxStack() const {
    return 1;
}

const ItemBase& itemById(ItemId item) {
    static MaterialItem none(ItemId::None, "stoneforge:none", "None");
    static PlaceableItem wood(ItemId::Wood, "stoneforge:wood", "Wood", TileType::WoodLog);
    static PlaceableItem planks(ItemId::Planks, "stoneforge:planks", "Planks", TileType::WoodWall);
    static MaterialItem sticks(ItemId::Sticks, "stoneforge:sticks", "Sticks");
    static PlaceableItem ore(ItemId::Ore, "stoneforge:ore", "Ore", TileType::Wall);
    static PlaceableItem workbench(ItemId::WorkbenchKit, "stoneforge:workbench_kit", "Workbench Kit", TileType::Workbench);

    switch(item) {
        case ItemId::Wood:
            return wood;
        case ItemId::Planks:
            return planks;
        case ItemId::Sticks:
            return sticks;
        case ItemId::Ore:
            return ore;
        case ItemId::WorkbenchKit:
            return workbench;
        case ItemId::None:
        default:
            return none;
    }
}

ItemId itemIdFromKey(std::string_view key) {
    if(key == "wood" || key == "stoneforge:wood") {
        return ItemId::Wood;
    }
    if(key == "planks" || key == "stoneforge:planks") {
        return ItemId::Planks;
    }
    if(key == "sticks" || key == "stoneforge:sticks") {
        return ItemId::Sticks;
    }
    if(key == "ore" || key == "stoneforge:ore") {
        return ItemId::Ore;
    }
    if(key == "workbench" || key == "workbench_kit" || key == "stoneforge:workbench_kit") {
        return ItemId::WorkbenchKit;
    }
    return ItemId::None;
}

}  // namespace stoneforge
