#pragma once

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
    std::string sourceMod;
};

class ContentRegistry {
public:
    void registerSprite(SpriteDef def);
    void registerBlock(BlockDef def);
    void registerItem(ItemDef def);

    const std::unordered_map<std::string, SpriteDef>& sprites() const;
    const std::unordered_map<std::string, BlockDef>& blocks() const;
    const std::unordered_map<std::string, ItemDef>& items() const;

private:
    std::unordered_map<std::string, SpriteDef> sprites_;
    std::unordered_map<std::string, BlockDef> blocks_;
    std::unordered_map<std::string, ItemDef> items_;
};

}  // namespace stoneforge::mod
