#include "stoneforge/mod/mod_loader.hpp"

#include <algorithm>
#include <fstream>
#include <optional>
#include <sstream>

#include <nlohmann/json.hpp>

#include "stoneforge/recipe.hpp"

namespace stoneforge::mod {

namespace {

using json = nlohmann::json;

std::optional<json> readJsonFile(const std::filesystem::path& path, std::string* errorMessage) {
    std::ifstream in(path);
    if(!in.is_open()) {
        if(errorMessage) {
            *errorMessage = "cannot open json file: " + path.string();
        }
        return std::nullopt;
    }

    std::stringstream buffer;
    buffer << in.rdbuf();
    try {
        return json::parse(buffer.str());
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = "json parse error in " + path.string() + ": " + ex.what();
        }
        return std::nullopt;
    }
}

std::string namespacedId(const std::string& modId, std::string id) {
    if(id.find(':') != std::string::npos) {
        return id;
    }
    return modId + ":" + id;
}

unsigned char toByteColor(int value) {
    return static_cast<unsigned char>(std::clamp(value, 0, 255));
}

bool loadSingleMod(
    const std::filesystem::path& modPath,
    ContentRegistry& registry,
    AssetManager& assets,
    std::vector<LoadedModInfo>& outMods,
    std::string* errorMessage
) {
    const auto modJsonPath = modPath / "mod.json";
    if(!std::filesystem::exists(modJsonPath)) {
        return true;
    }

    auto modJsonOpt = readJsonFile(modJsonPath, errorMessage);
    if(!modJsonOpt) {
        return false;
    }

    const auto& modJson = *modJsonOpt;
    LoadedModInfo mod;
    mod.id = modJson.value("id", modPath.filename().string());
    mod.name = modJson.value("name", mod.id);
    mod.version = modJson.value("version", "0.0.1");
    mod.rootPath = modPath;

    if(modJson.contains("scripts") && modJson["scripts"].is_array()) {
        for(const auto& script : modJson["scripts"]) {
            if(script.is_string()) {
                mod.scripts.push_back(script.get<std::string>());
            }
        }
    }

    assets.mountMod(mod.id, modPath);

    const auto spritesPath = modPath / "sprites.json";
    if(std::filesystem::exists(spritesPath)) {
        auto spritesJsonOpt = readJsonFile(spritesPath, errorMessage);
        if(!spritesJsonOpt) {
            return false;
        }
        for(const auto& node : *spritesJsonOpt) {
            if(!node.is_object()) {
                continue;
            }
            SpriteDef sprite;
            sprite.id = namespacedId(mod.id, node.value("id", ""));
            sprite.slot = node.value("slot", "");
            sprite.texture = node.value("texture", "");
            sprite.sourceMod = mod.id;
            if(!sprite.id.empty() && !sprite.slot.empty() && !sprite.texture.empty()) {
                registry.registerSprite(std::move(sprite));
            }
        }
    }

    const auto blocksPath = modPath / "blocks.json";
    if(std::filesystem::exists(blocksPath)) {
        auto blocksJsonOpt = readJsonFile(blocksPath, errorMessage);
        if(!blocksJsonOpt) {
            return false;
        }
        for(const auto& node : *blocksJsonOpt) {
            if(!node.is_object()) {
                continue;
            }
            BlockDef block;
            block.id = namespacedId(mod.id, node.value("id", ""));
            block.displayName = node.value("name", block.id);
            block.spriteId = namespacedId(mod.id, node.value("sprite", ""));
            block.solid = node.value("solid", true);
            block.sourceMod = mod.id;
            if(!block.id.empty()) {
                registry.registerBlock(std::move(block));
            }
        }
    }

    const auto itemsPath = modPath / "items.json";
    if(std::filesystem::exists(itemsPath)) {
        auto itemsJsonOpt = readJsonFile(itemsPath, errorMessage);
        if(!itemsJsonOpt) {
            return false;
        }
        for(const auto& node : *itemsJsonOpt) {
            if(!node.is_object()) {
                continue;
            }
            ItemDef item;
            item.id = namespacedId(mod.id, node.value("id", ""));
            item.displayName = node.value("name", item.id);
            item.icon = node.value("icon", "");
            item.glyph = node.value("glyph", "?");
            item.maxStack = std::max(1, node.value("maxStack", 64));
            item.placeTile = node.value("placeTile", "");
            item.placeBlockId = node.value("placeBlock", "");
            if(!item.placeBlockId.empty()) {
                item.placeBlockId = namespacedId(mod.id, item.placeBlockId);
            }
            if(node.contains("tint") && node["tint"].is_array()) {
                const auto& tint = node["tint"];
                if(tint.size() >= 3) {
                    item.tint[0] = toByteColor(tint[0].get<int>());
                    item.tint[1] = toByteColor(tint[1].get<int>());
                    item.tint[2] = toByteColor(tint[2].get<int>());
                    item.tint[3] = (tint.size() >= 4) ? toByteColor(tint[3].get<int>()) : 255;
                }
            }
            item.sourceMod = mod.id;
            if(!item.id.empty()) {
                registry.registerItem(std::move(item));
            }
        }
    }

    const auto biomesPath = modPath / "biomes.json";
    if(std::filesystem::exists(biomesPath)) {
        auto biomesJsonOpt = readJsonFile(biomesPath, errorMessage);
        if(!biomesJsonOpt) {
            return false;
        }
        for(const auto& node : *biomesJsonOpt) {
            if(!node.is_object()) {
                continue;
            }
            BiomeDef biome;
            biome.id = namespacedId(mod.id, node.value("id", ""));
            biome.displayName = node.value("name", biome.id);
            biome.floorA = node.value("floorA", "FloorACold");
            biome.floorB = node.value("floorB", "FloorBCold");
            biome.wallA = node.value("wallA", "WallACold");
            biome.wallB = node.value("wallB", "WallBCold");
            biome.center = node.value("center", 0.5F);
            biome.span = node.value("span", 0.34F);
            biome.sourceMod = mod.id;
            if(!biome.id.empty()) {
                registry.registerBiome(std::move(biome));
            }
        }
    }

    const auto entitiesPath = modPath / "entities.json";
    if(std::filesystem::exists(entitiesPath)) {
        auto entitiesJsonOpt = readJsonFile(entitiesPath, errorMessage);
        if(!entitiesJsonOpt) {
            return false;
        }
        for(const auto& node : *entitiesJsonOpt) {
            if(!node.is_object()) {
                continue;
            }
            EntityDef entity;
            entity.id = namespacedId(mod.id, node.value("id", ""));
            entity.displayName = node.value("name", entity.id);
            entity.kind = node.value("kind", "mob");
            entity.spriteSlot = node.value("spriteSlot", "Mob");
            entity.hp = node.value("hp", 1);
            entity.sourceMod = mod.id;
            if(!entity.id.empty()) {
                registry.registerEntity(std::move(entity));
            }
        }
    }

    const auto recipesPath = modPath / "recipes.json";
    if(std::filesystem::exists(recipesPath)) {
        if(!stoneforge::recipeCatalog().loadJsonFile(recipesPath, mod.id, errorMessage)) {
            return false;
        }
    }

    outMods.push_back(std::move(mod));
    return true;
}

}  // namespace

bool ModLoader::loadAll(
    const std::filesystem::path& baseModPath,
    const std::filesystem::path& modsRootPath,
    ContentRegistry& registry,
    AssetManager& assets,
    std::vector<LoadedModInfo>& outMods,
    std::string* errorMessage
) const {
    outMods.clear();

    if(std::filesystem::exists(baseModPath)) {
        if(!loadSingleMod(baseModPath, registry, assets, outMods, errorMessage)) {
            return false;
        }
    }

    if(!std::filesystem::exists(modsRootPath)) {
        return true;
    }

    for(const auto& entry : std::filesystem::directory_iterator(modsRootPath)) {
        if(!entry.is_directory()) {
            continue;
        }

        if(!loadSingleMod(entry.path(), registry, assets, outMods, errorMessage)) {
            return false;
        }
    }

    return true;
}

}  // namespace stoneforge::mod
