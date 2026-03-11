#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "stoneforge/mod/asset_manager.hpp"
#include "stoneforge/mod/content_registry.hpp"

namespace stoneforge::mod {

struct LoadedModInfo {
    std::string id;
    std::string name;
    std::string version;
    std::filesystem::path rootPath;
    std::vector<std::string> scripts;
};

class ModLoader {
public:
    bool loadAll(
        const std::filesystem::path& baseModPath,
        const std::filesystem::path& modsRootPath,
        ContentRegistry& registry,
        AssetManager& assets,
        std::vector<LoadedModInfo>& outMods,
        std::string* errorMessage
    ) const;
};

}  // namespace stoneforge::mod
