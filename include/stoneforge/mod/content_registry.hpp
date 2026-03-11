#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace stoneforge::mod {

struct SpriteDef {
    std::string id;
    std::string slot;
    std::string texture;
    std::string sourceMod;
};

struct BlockDef {
    std::string id;
    std::string displayName;
    std::string spriteId;
    bool solid = true;
    std::string sourceMod;
};

struct ItemDef {
    std::string id;
    std::string displayName;
    std::string icon;
    std::string glyph = "?";
    std::array<unsigned char, 4> tint{92, 112, 148, 255};
    int maxStack = 64;
    std::string placeTile;
    std::string placeBlockId;
    std::string sourceMod;
};

struct BiomeDef {
    std::string id;
    std::string displayName;
    std::string floorA;
    std::string floorB;
    std::string wallA;
    std::string wallB;
    float center = 0.5F;
    float span = 0.34F;
    std::string sourceMod;
};

struct EntityDef {
    std::string id;
    std::string displayName;
    std::string kind;
    std::string spriteSlot;
    int hp = 1;
    std::string sourceMod;
};

class ContentRegistry {
public:
    void registerSprite(SpriteDef def);
    void registerBlock(BlockDef def);
    void registerItem(ItemDef def);
    void registerBiome(BiomeDef def);
    void registerEntity(EntityDef def);

    const std::unordered_map<std::string, SpriteDef>& sprites() const;
    const std::unordered_map<std::string, BlockDef>& blocks() const;
    const std::unordered_map<std::string, ItemDef>& items() const;
    const std::unordered_map<std::string, BiomeDef>& biomes() const;
    const std::unordered_map<std::string, EntityDef>& entities() const;

    const BiomeDef* findBiome(const std::string& id) const;
    const EntityDef* findEntity(const std::string& id) const;
    const ItemDef* findItem(const std::string& id) const;

private:
    std::unordered_map<std::string, SpriteDef> sprites_;
    std::unordered_map<std::string, BlockDef> blocks_;
    std::unordered_map<std::string, ItemDef> items_;
    std::unordered_map<std::string, BiomeDef> biomes_;
    std::unordered_map<std::string, EntityDef> entities_;
};

}  // namespace stoneforge::mod
