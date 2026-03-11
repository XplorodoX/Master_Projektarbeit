#include "stoneforge/mod/asset_manager.hpp"

namespace stoneforge::mod {

void AssetManager::mountMod(const std::string& modId, const std::filesystem::path& rootPath) {
    roots_[modId] = rootPath;
}

std::optional<std::filesystem::path> AssetManager::resolve(const std::string& modId, const std::string& relativePath) const {
    const auto it = roots_.find(modId);
    if(it == roots_.end()) {
        return std::nullopt;
    }

    const std::filesystem::path candidate = it->second / relativePath;
    if(std::filesystem::exists(candidate)) {
        return candidate;
    }

    return std::nullopt;
}

}  // namespace stoneforge::mod
