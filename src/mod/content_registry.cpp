#include "stoneforge/mod/content_registry.hpp"

namespace stoneforge::mod {

void ContentRegistry::registerSprite(SpriteDef def) {
    sprites_[def.id] = std::move(def);
}

void ContentRegistry::registerBlock(BlockDef def) {
    blocks_[def.id] = std::move(def);
}

void ContentRegistry::registerItem(ItemDef def) {
    items_[def.id] = std::move(def);
}

const std::unordered_map<std::string, SpriteDef>& ContentRegistry::sprites() const {
    return sprites_;
}

const std::unordered_map<std::string, BlockDef>& ContentRegistry::blocks() const {
    return blocks_;
}

const std::unordered_map<std::string, ItemDef>& ContentRegistry::items() const {
    return items_;
}

}  // namespace stoneforge::mod
