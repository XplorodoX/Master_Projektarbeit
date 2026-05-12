#include "stoneforge/item.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <string>

namespace stoneforge {

namespace {

struct LegacyItemInfo {
    ItemId id;
    const char* key;
    const char* displayName;
    TileType placementTile;
    int maxStack;
    const char* glyph;
    std::array<unsigned char, 4> tint;
    const char* icon;
};

constexpr std::array<LegacyItemInfo, 5> kLegacyItems{{
    {ItemId::Wood, "stoneforge:wood", "Wood", TileType::WoodLog, 64, "W", {156, 109, 71, 255}, "wood"},
    {ItemId::Planks, "stoneforge:planks", "Planks", TileType::WoodWall, 64, "P", {187, 141, 97, 255}, "planks"},
    {ItemId::Sticks, "stoneforge:sticks", "Sticks", TileType::Empty, 64, "S", {136, 99, 72, 255}, "sticks"},
    {ItemId::Ore, "stoneforge:ore", "Ore", TileType::Wall, 64, "O", {185, 162, 93, 255}, "ore"},
    {ItemId::WorkbenchKit, "stoneforge:workbench_kit", "Workbench Kit", TileType::Workbench, 64, "B", {130, 95, 70, 255}, "workbench_kit"},
}};

const LegacyItemInfo* findLegacyById(ItemId id) {
    for(const auto& item : kLegacyItems) {
        if(item.id == id) {
            return &item;
        }
    }
    return nullptr;
}

const LegacyItemInfo* findLegacyByKey(std::string_view key) {
    const std::string normalized = normalizeItemKey(key);
    for(const auto& item : kLegacyItems) {
        if(normalized == item.key) {
            return &item;
        }

        const std::size_t sep = std::string_view(item.key).find(':');
        if(sep != std::string_view::npos && key == std::string_view(item.key).substr(sep + 1)) {
            return &item;
        }
    }
    return nullptr;
}

std::string prettifyItemLabel(std::string_view key) {
    std::string normalized = normalizeItemKey(key);
    if(normalized.empty()) {
        return "None";
    }

    const std::size_t sep = normalized.find(':');
    std::string label = (sep == std::string::npos) ? normalized : normalized.substr(sep + 1);
    std::replace(label.begin(), label.end(), '_', ' ');

    bool capitalizeNext = true;
    for(char& c : label) {
        if(std::isspace(static_cast<unsigned char>(c))) {
            capitalizeNext = true;
            continue;
        }
        if(capitalizeNext) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            capitalizeNext = false;
        }
    }
    return label;
}

}  // namespace

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

    static const std::array<const ItemBase*, 6> kById{&none, &wood, &planks, &sticks, &ore, &workbench};
    const int idx = static_cast<int>(item);
    if(idx >= 0 && idx < static_cast<int>(kById.size())) {
        return *kById[static_cast<std::size_t>(idx)];
    }
    return none;
}

ItemId itemIdFromKey(std::string_view key) {
    const auto* legacy = findLegacyByKey(key);
    if(legacy == nullptr) {
        return ItemId::None;
    }
    return legacy->id;
}

std::string_view itemKeyFromId(ItemId item) {
    const auto* legacy = findLegacyById(item);
    if(legacy == nullptr) {
        return "stoneforge:none";
    }
    return legacy->key;
}

std::string normalizeItemKey(std::string_view key) {
    if(key.empty()) {
        return {};
    }

    std::string out(key);
    if(out.find(':') == std::string::npos) {
        out = "stoneforge:" + out;
    }
    return out;
}

std::string itemDisplayName(std::string_view key) {
    if(const auto* legacy = findLegacyByKey(key)) {
        return legacy->displayName;
    }

    return prettifyItemLabel(key);
}

TileType itemPlacementTile(std::string_view key) {
    if(const auto* legacy = findLegacyByKey(key)) {
        return legacy->placementTile;
    }

    return TileType::Empty;
}

int itemMaxStack(std::string_view key) {
    if(const auto* legacy = findLegacyByKey(key)) {
        return std::max(1, legacy->maxStack);
    }

    return 64;
}

std::string itemGlyphText(std::string_view key) {
    if(const auto* legacy = findLegacyByKey(key)) {
        return legacy->glyph;
    }

    return "?";
}

std::array<unsigned char, 4> itemTintRgba(std::string_view key) {
    if(const auto* legacy = findLegacyByKey(key)) {
        return legacy->tint;
    }

    return {92, 112, 148, 255};
}

std::string itemIconId(std::string_view key) {
    if(const auto* legacy = findLegacyByKey(key)) {
        return legacy->icon;
    }

    return std::string(normalizeItemKey(key));
}

}  // namespace stoneforge
