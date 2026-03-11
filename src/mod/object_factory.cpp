#include "stoneforge/mod/object_factory.hpp"

namespace stoneforge::mod {

namespace {

ObjectArchetype vanillaObject(
    std::string id,
    std::string displayName,
    std::string spriteId,
    bool solid,
    bool passable,
    bool mineable,
    bool blocksLineOfSight
) {
    ObjectArchetype out;
    out.id = std::move(id);
    out.displayName = std::move(displayName);
    out.spriteId = std::move(spriteId);
    out.solid = solid;
    out.passable = passable;
    out.mineable = mineable;
    out.blocksLineOfSight = blocksLineOfSight;
    out.sourceMod = "stoneforge";
    out.vanilla = true;
    return out;
}

}  // namespace

void ObjectFactory::buildFromContent(const ContentRegistry& registry) {
    objects_.clear();
    registerVanillaDefaults();

    for(const auto& [id, block] : registry.blocks()) {
        ObjectArchetype archetype;
        archetype.id = id;
        archetype.displayName = block.displayName;
        archetype.spriteId = block.spriteId;
        archetype.solid = block.solid;
        archetype.passable = !block.solid;
        archetype.mineable = true;
        archetype.blocksLineOfSight = block.solid;
        archetype.sourceMod = block.sourceMod;
        archetype.vanilla = false;
        registerArchetype(std::move(archetype));
    }
}

const ObjectArchetype* ObjectFactory::find(const std::string& id) const {
    const auto it = objects_.find(id);
    if(it == objects_.end()) {
        return nullptr;
    }
    return &it->second;
}

const std::unordered_map<std::string, ObjectArchetype>& ObjectFactory::objects() const {
    return objects_;
}

void ObjectFactory::registerVanillaDefaults() {
    registerArchetype(vanillaObject("stoneforge:empty", "Empty", "base:floor_cold_a", false, true, false, false));
    registerArchetype(vanillaObject("stoneforge:exit", "Exit", "base:exit", false, true, false, false));
    registerArchetype(vanillaObject("stoneforge:wall", "Wall", "base:wall_cold_a", true, false, false, true));
    registerArchetype(vanillaObject("stoneforge:resource", "Resource", "base:ore", true, false, true, true));
    registerArchetype(vanillaObject("stoneforge:tree", "Tree", "base:tree", true, false, true, true));
    registerArchetype(vanillaObject("stoneforge:workbench", "Workbench", "base:workbench", true, false, true, false));
    registerArchetype(vanillaObject("stoneforge:wood_wall", "Wood Wall", "base:wood_wall", true, false, false, true));
    registerArchetype(vanillaObject("stoneforge:wood_log", "Wood Log", "base:wood_log", true, false, false, true));
}

void ObjectFactory::registerArchetype(ObjectArchetype archetype) {
    if(archetype.id.empty()) {
        return;
    }
    objects_[archetype.id] = std::move(archetype);
}

}  // namespace stoneforge::mod
