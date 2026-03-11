#pragma once

#include <array>
#include <string>
#include <string_view>

#include "stoneforge/simulation.hpp"
#include "stoneforge/types.hpp"

namespace stoneforge {

namespace mod {
class ContentRegistry;
struct ItemDef;
}  // namespace mod

class ItemBase {
public:
    virtual ~ItemBase() = default;

    virtual ItemId id() const = 0;
    virtual std::string_view key() const = 0;
    virtual std::string_view displayName() const = 0;

    virtual bool isPlaceable() const {
        return false;
    }

    virtual TileType placementTile() const {
        return TileType::Empty;
    }

    virtual bool isTool() const {
        return false;
    }

    virtual int maxStack() const {
        return 64;
    }
};

class PlaceableItem final : public ItemBase {
public:
    PlaceableItem(ItemId id, std::string_view key, std::string_view displayName, TileType placeTile);

    ItemId id() const override;
    std::string_view key() const override;
    std::string_view displayName() const override;
    bool isPlaceable() const override;
    TileType placementTile() const override;

private:
    ItemId id_ = ItemId::None;
    std::string_view key_;
    std::string_view displayName_;
    TileType placeTile_ = TileType::Empty;
};

class MaterialItem final : public ItemBase {
public:
    MaterialItem(ItemId id, std::string_view key, std::string_view displayName);

    ItemId id() const override;
    std::string_view key() const override;
    std::string_view displayName() const override;

private:
    ItemId id_ = ItemId::None;
    std::string_view key_;
    std::string_view displayName_;
};

class ToolItem final : public ItemBase {
public:
    ToolItem(ItemId id, std::string_view key, std::string_view displayName);

    ItemId id() const override;
    std::string_view key() const override;
    std::string_view displayName() const override;
    bool isTool() const override;
    int maxStack() const override;

private:
    ItemId id_ = ItemId::None;
    std::string_view key_;
    std::string_view displayName_;
};

const ItemBase& itemById(ItemId item);
ItemId itemIdFromKey(std::string_view key);
std::string_view itemKeyFromId(ItemId item);
std::string normalizeItemKey(std::string_view key);
std::string itemDisplayName(std::string_view key);
TileType itemPlacementTile(std::string_view key);
int itemMaxStack(std::string_view key);
std::string itemGlyphText(std::string_view key);
std::array<unsigned char, 4> itemTintRgba(std::string_view key);
std::string itemIconId(std::string_view key);
const mod::ItemDef* itemDefinition(std::string_view key);
void setItemRegistry(const mod::ContentRegistry* registry);

}  // namespace stoneforge
