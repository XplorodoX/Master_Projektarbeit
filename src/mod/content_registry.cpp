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

void ContentRegistry::registerBiome(BiomeDef def) {
    biomes_[def.id] = std::move(def);
}

void ContentRegistry::registerEntity(EntityDef def) {
    entities_[def.id] = std::move(def);
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

const std::unordered_map<std::string, BiomeDef>& ContentRegistry::biomes() const {
    return biomes_;
}

const std::unordered_map<std::string, EntityDef>& ContentRegistry::entities() const {
    return entities_;
}

const BiomeDef* ContentRegistry::findBiome(const std::string& id) const {
    const auto it = biomes_.find(id);
    if(it == biomes_.end()) {
        return nullptr;
    }
    return &it->second;
}

const EntityDef* ContentRegistry::findEntity(const std::string& id) const {
    const auto it = entities_.find(id);
    if(it == entities_.end()) {
        return nullptr;
    }
    return &it->second;
}

}  // namespace stoneforge::mod
