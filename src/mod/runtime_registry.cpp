#include "stoneforge/mod/runtime_registry.hpp"

#include <algorithm>

namespace stoneforge::mod {

namespace {

constexpr std::uint32_t kBlockRuntimeBase = 1000U;
constexpr std::uint32_t kEntityRuntimeBase = 2000U;

}  // namespace

void RuntimeRegistry::build(const ContentRegistry& contentRegistry, const ObjectFactory& objectFactory) {
    contentRegistry_ = &contentRegistry;
    objectFactory_ = &objectFactory;

    blockIdToRuntime_.clear();
    runtimeToBlockId_.clear();
    entityIdToRuntime_.clear();
    runtimeToEntityId_.clear();

    std::vector<std::string> blockIds;
    blockIds.reserve(objectFactory.objects().size());
    for(const auto& [id, obj] : objectFactory.objects()) {
        (void)obj;
        blockIds.push_back(id);
    }
    std::sort(blockIds.begin(), blockIds.end());

    std::uint32_t nextBlockId = kBlockRuntimeBase;
    for(const auto& id : blockIds) {
        blockIdToRuntime_[id] = nextBlockId;
        runtimeToBlockId_[nextBlockId] = id;
        ++nextBlockId;
    }

    std::vector<std::string> entityIds;
    entityIds.reserve(contentRegistry.entities().size());
    for(const auto& [id, def] : contentRegistry.entities()) {
        (void)def;
        entityIds.push_back(id);
    }
    std::sort(entityIds.begin(), entityIds.end());

    std::uint32_t nextEntityId = kEntityRuntimeBase;
    for(const auto& id : entityIds) {
        entityIdToRuntime_[id] = nextEntityId;
        runtimeToEntityId_[nextEntityId] = id;
        ++nextEntityId;
    }
}

bool RuntimeRegistry::resolveBlockRuntimeId(const std::string& token, std::uint32_t& outRuntimeId) const {
    std::uint32_t numeric = 0;
    if(parseRuntimeIdToken(token, numeric)) {
        if(runtimeToBlockId_.find(numeric) != runtimeToBlockId_.end()) {
            outRuntimeId = numeric;
            return true;
        }
    }

    const auto it = blockIdToRuntime_.find(token);
    if(it == blockIdToRuntime_.end()) {
        return false;
    }
    outRuntimeId = it->second;
    return true;
}

bool RuntimeRegistry::resolveEntityRuntimeId(const std::string& token, std::uint32_t& outRuntimeId) const {
    std::uint32_t numeric = 0;
    if(parseRuntimeIdToken(token, numeric)) {
        if(runtimeToEntityId_.find(numeric) != runtimeToEntityId_.end()) {
            outRuntimeId = numeric;
            return true;
        }
    }

    const auto it = entityIdToRuntime_.find(token);
    if(it == entityIdToRuntime_.end()) {
        return false;
    }
    outRuntimeId = it->second;
    return true;
}

const ObjectArchetype* RuntimeRegistry::findBlockByRuntimeId(std::uint32_t runtimeId) const {
    if(objectFactory_ == nullptr) {
        return nullptr;
    }

    const auto it = runtimeToBlockId_.find(runtimeId);
    if(it == runtimeToBlockId_.end()) {
        return nullptr;
    }
    return objectFactory_->find(it->second);
}

const EntityDef* RuntimeRegistry::findEntityByRuntimeId(std::uint32_t runtimeId) const {
    if(contentRegistry_ == nullptr) {
        return nullptr;
    }

    const auto it = runtimeToEntityId_.find(runtimeId);
    if(it == runtimeToEntityId_.end()) {
        return nullptr;
    }
    return contentRegistry_->findEntity(it->second);
}

std::vector<std::string> RuntimeRegistry::blockIds() const {
    std::vector<std::string> out;
    out.reserve(blockIdToRuntime_.size());
    for(const auto& [id, runtimeId] : blockIdToRuntime_) {
        out.push_back(id);
        out.push_back(std::to_string(runtimeId));
    }
    std::sort(out.begin(), out.end());
    return out;
}

std::vector<std::string> RuntimeRegistry::entityIds() const {
    std::vector<std::string> out;
    out.reserve(entityIdToRuntime_.size());
    for(const auto& [id, runtimeId] : entityIdToRuntime_) {
        out.push_back(id);
        out.push_back(std::to_string(runtimeId));
    }
    std::sort(out.begin(), out.end());
    return out;
}

std::vector<std::string> RuntimeRegistry::blockEntries() const {
    std::vector<std::string> out;
    out.reserve(runtimeToBlockId_.size());
    for(const auto& [runtimeId, id] : runtimeToBlockId_) {
        out.push_back(id + "=#" + std::to_string(runtimeId));
    }
    std::sort(out.begin(), out.end());
    return out;
}

std::vector<std::string> RuntimeRegistry::entityEntries() const {
    std::vector<std::string> out;
    out.reserve(runtimeToEntityId_.size());
    for(const auto& [runtimeId, id] : runtimeToEntityId_) {
        out.push_back(id + "=#" + std::to_string(runtimeId));
    }
    std::sort(out.begin(), out.end());
    return out;
}

bool RuntimeRegistry::parseRuntimeIdToken(const std::string& token, std::uint32_t& outValue) {
    if(token.empty()) {
        return false;
    }

    std::size_t idx = 0;
    if(token[0] == '#') {
        idx = 1;
    }
    if(idx >= token.size()) {
        return false;
    }

    std::uint32_t value = 0;
    for(; idx < token.size(); ++idx) {
        const char c = token[idx];
        if(c < '0' || c > '9') {
            return false;
        }
        value = value * 10U + static_cast<std::uint32_t>(c - '0');
    }

    outValue = value;
    return true;
}

}  // namespace stoneforge::mod
