#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

namespace stoneforge::mod {

class AssetManager {
public:
    void mountMod(const std::string& modId, const std::filesystem::path& rootPath);
    std::optional<std::filesystem::path> resolve(const std::string& modId, const std::string& relativePath) const;

private:
    std::unordered_map<std::string, std::filesystem::path> roots_;
};

}  // namespace stoneforge::mod
