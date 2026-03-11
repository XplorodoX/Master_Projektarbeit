#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "stoneforge/mod/content_registry.hpp"
#include "stoneforge/mod/object_factory.hpp"

namespace stoneforge::mod {

class RuntimeRegistry {
public:
    void build(const ContentRegistry& contentRegistry, const ObjectFactory& objectFactory);

    bool resolveBlockRuntimeId(const std::string& token, std::uint32_t& outRuntimeId) const;
    bool resolveEntityRuntimeId(const std::string& token, std::uint32_t& outRuntimeId) const;

    const ObjectArchetype* findBlockByRuntimeId(std::uint32_t runtimeId) const;
    const EntityDef* findEntityByRuntimeId(std::uint32_t runtimeId) const;

    std::vector<std::string> blockIds() const;
    std::vector<std::string> entityIds() const;
    std::vector<std::string> blockEntries() const;
    std::vector<std::string> entityEntries() const;

private:
    static bool parseRuntimeIdToken(const std::string& token, std::uint32_t& outValue);

    const ContentRegistry* contentRegistry_ = nullptr;
    const ObjectFactory* objectFactory_ = nullptr;

    std::unordered_map<std::string, std::uint32_t> blockIdToRuntime_;
    std::unordered_map<std::uint32_t, std::string> runtimeToBlockId_;
    std::unordered_map<std::string, std::uint32_t> entityIdToRuntime_;
    std::unordered_map<std::uint32_t, std::string> runtimeToEntityId_;
};

}  // namespace stoneforge::mod
