#pragma once

#include <string>
#include <unordered_map>

#include "stoneforge/mod/content_registry.hpp"

namespace stoneforge::mod {

struct ObjectArchetype {
    std::string id;
    std::string displayName;
    std::string spriteId;
    bool solid = true;
    bool passable = false;
    bool mineable = false;
    bool blocksLineOfSight = true;
    std::string sourceMod;
    bool vanilla = false;
};

class ObjectFactory {
public:
    void buildFromContent(const ContentRegistry& registry);

    const ObjectArchetype* find(const std::string& id) const;
    const std::unordered_map<std::string, ObjectArchetype>& objects() const;

private:
    void registerVanillaDefaults();
    void registerArchetype(ObjectArchetype archetype);

    std::unordered_map<std::string, ObjectArchetype> objects_;
};

}  // namespace stoneforge::mod
