#include "stoneforge/game_config.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

#include <nlohmann/json.hpp>

namespace stoneforge {

namespace {

using json = nlohmann::json;

GameConfig makeDefaultConfig() {
    GameConfig cfg;

    cfg.mobBehaviorProfiles = {
        MobBehaviorProfile{"default", "zombie", 8, 12, 0, 0.55F, 0.25F, 1, 1, false},
        MobBehaviorProfile{"zombie", "zombie", 10, 14, 0, 0.45F, 0.15F, 1, 1, true},
        MobBehaviorProfile{"animal", "animal", 5, 8, 4, 0.75F, 0.20F, 1, 1, false},
        MobBehaviorProfile{"boss", "boss", 16, 20, 0, 0.25F, 0.05F, 1, 2, true},
    };

    cfg.entityBehaviorMap = {
        {"stoneforge:mob", "zombie"},
        {"stoneforge:zombie", "zombie"},
        {"stoneforge:animal", "animal"},
        {"stoneforge:boss", "boss"},
    };

    cfg.spawnTable = {
        EntitySpawnEntry{"stoneforge:mob", "zombie", 1, false, "default", 6},
        EntitySpawnEntry{"stoneforge:zombie", "zombie", 2, true, "default", 4},
        EntitySpawnEntry{"stoneforge:animal", "animal", 1, false, "default", 2},
    };

    return cfg;
}

GameConfig gConfig = makeDefaultConfig();

std::string normalizeId(std::string id) {
    if(id.empty()) {
        return id;
    }
    if(id.find(':') == std::string::npos) {
        return "stoneforge:" + id;
    }
    return id;
}

int readInt(const json& node, const char* key, int fallback) {
    if(!node.contains(key)) {
        return fallback;
    }
    if(node[key].is_number_integer()) {
        return node[key].get<int>();
    }
    return fallback;
}

double readDouble(const json& node, const char* key, double fallback) {
    if(!node.contains(key)) {
        return fallback;
    }
    if(node[key].is_number()) {
        return node[key].get<double>();
    }
    return fallback;
}

float readFloat(const json& node, const char* key, float fallback) {
    if(!node.contains(key)) {
        return fallback;
    }
    if(node[key].is_number()) {
        return node[key].get<float>();
    }
    return fallback;
}

bool readBool(const json& node, const char* key, bool fallback) {
    if(!node.contains(key)) {
        return fallback;
    }
    if(node[key].is_boolean()) {
        return node[key].get<bool>();
    }
    return fallback;
}

std::uint64_t readU64(const json& node, const char* key, std::uint64_t fallback) {
    if(!node.contains(key)) {
        return fallback;
    }

    const auto& v = node[key];
    if(v.is_number_unsigned()) {
        return v.get<std::uint64_t>();
    }
    if(v.is_number_integer()) {
        const auto i = v.get<long long>();
        return i < 0 ? fallback : static_cast<std::uint64_t>(i);
    }
    return fallback;
}

void parseWorldConfig(GameConfig& cfg, const json& root) {
    if(!root.contains("worldgen") || !root["worldgen"].is_object()) {
        return;
    }

    const auto& world = root["worldgen"];
    if(world.contains("spawn") && world["spawn"].is_array() && world["spawn"].size() >= 2) {
        cfg.world.spawn.x = world["spawn"][0].get<int>();
        cfg.world.spawn.y = world["spawn"][1].get<int>();
    }
    if(world.contains("exit") && world["exit"].is_array() && world["exit"].size() >= 2) {
        cfg.world.exit.x = world["exit"][0].get<int>();
        cfg.world.exit.y = world["exit"][1].get<int>();
    }

    cfg.world.randomizeExitFromSpawn = readBool(world, "randomizeExitFromSpawn", cfg.world.randomizeExitFromSpawn);
    cfg.world.exitMinDistance = std::max(1, readInt(world, "exitMinDistance", cfg.world.exitMinDistance));
    cfg.world.exitMaxDistance = std::max(cfg.world.exitMinDistance, readInt(world, "exitMaxDistance", cfg.world.exitMaxDistance));

    cfg.world.coldBiomeMax = readDouble(world, "coldBiomeMax", cfg.world.coldBiomeMax);
    cfg.world.warmBiomeMax = readDouble(world, "warmBiomeMax", cfg.world.warmBiomeMax);

    cfg.world.coldWallThreshold = readDouble(world, "coldWallThreshold", cfg.world.coldWallThreshold);
    cfg.world.warmWallThreshold = readDouble(world, "warmWallThreshold", cfg.world.warmWallThreshold);
    cfg.world.mossWallThreshold = readDouble(world, "mossWallThreshold", cfg.world.mossWallThreshold);

    cfg.world.coldOreThreshold = readDouble(world, "coldOreThreshold", cfg.world.coldOreThreshold);
    cfg.world.warmOreThreshold = readDouble(world, "warmOreThreshold", cfg.world.warmOreThreshold);
    cfg.world.mossOreThreshold = readDouble(world, "mossOreThreshold", cfg.world.mossOreThreshold);

    cfg.world.warmTreeThreshold = readDouble(world, "warmTreeThreshold", cfg.world.warmTreeThreshold);
    cfg.world.mossTreeThreshold = readDouble(world, "mossTreeThreshold", cfg.world.mossTreeThreshold);

    cfg.world.spawnClearRadius = std::max(0, readInt(world, "spawnClearRadius", cfg.world.spawnClearRadius));
    cfg.world.exitClearRadius = std::max(0, readInt(world, "exitClearRadius", cfg.world.exitClearRadius));

    if(world.contains("noiseSalts") && world["noiseSalts"].is_object()) {
        const auto& salts = world["noiseSalts"];
        cfg.world.biomeSalt = readU64(salts, "biome", cfg.world.biomeSalt);
        cfg.world.densitySalt = readU64(salts, "density", cfg.world.densitySalt);
        cfg.world.oreSalt = readU64(salts, "ore", cfg.world.oreSalt);
        cfg.world.treeSalt = readU64(salts, "tree", cfg.world.treeSalt);
    }

    cfg.world.forceGuaranteedPath = readBool(world, "forceGuaranteedPath", cfg.world.forceGuaranteedPath);
    cfg.world.guaranteedPathFallback = readBool(world, "guaranteedPathFallback", cfg.world.guaranteedPathFallback);

    if(world.contains("procedural") && world["procedural"].is_object()) {
        const auto& procedural = world["procedural"];

        cfg.world.enableCellularSmoothing =
            readBool(procedural, "enableCellularSmoothing", cfg.world.enableCellularSmoothing);
        cfg.world.cellularIterations =
            std::max(0, readInt(procedural, "cellularIterations", cfg.world.cellularIterations));
        cfg.world.cellularBirthMinNeighbors =
            std::clamp(readInt(procedural, "cellularBirthMinNeighbors", cfg.world.cellularBirthMinNeighbors), 1, 8);
        cfg.world.cellularSurvivalMinNeighbors =
            std::clamp(readInt(procedural, "cellularSurvivalMinNeighbors", cfg.world.cellularSurvivalMinNeighbors), 1, 8);

        cfg.world.enableFloodFillValidation =
            readBool(procedural, "enableFloodFillValidation", cfg.world.enableFloodFillValidation);
        cfg.world.validationRadiusChunks =
            std::max(1, readInt(procedural, "validationRadiusChunks", cfg.world.validationRadiusChunks));

        cfg.world.enableMacroGraphPrecheck =
            readBool(procedural, "enableMacroGraphPrecheck", cfg.world.enableMacroGraphPrecheck);
        cfg.world.macroGraphRadiusChunks =
            std::max(1, readInt(procedural, "macroGraphRadiusChunks", cfg.world.macroGraphRadiusChunks));
    }
}

void parseGameplayConfig(GameConfig& cfg, const json& root) {
    if(root.contains("gameplay") && root["gameplay"].is_object()) {
        const auto& gameplay = root["gameplay"];
        cfg.gameplay.observationRadius = std::max(1, readInt(gameplay, "observationRadius", cfg.gameplay.observationRadius));
        cfg.gameplay.maxSteps = std::max(1, readInt(gameplay, "maxSteps", cfg.gameplay.maxSteps));
        cfg.gameplay.inventorySlots = std::max(1, readInt(gameplay, "inventorySlots", cfg.gameplay.inventorySlots));
        cfg.gameplay.inventoryStackLimit = std::max(1, readInt(gameplay, "inventoryStackLimit", cfg.gameplay.inventoryStackLimit));
        cfg.gameplay.hotbarSlots = std::max(1, readInt(gameplay, "hotbarSlots", cfg.gameplay.hotbarSlots));
        cfg.gameplay.miningRangeBaseTiles = std::max(0.5F, readFloat(gameplay, "miningRangeBaseTiles", cfg.gameplay.miningRangeBaseTiles));
        cfg.gameplay.miningRangePerToolLevel = std::max(0.0F, readFloat(gameplay, "miningRangePerToolLevel", cfg.gameplay.miningRangePerToolLevel));
        cfg.gameplay.idleEnergyRegenInterval = std::max(1, readInt(gameplay, "idleEnergyRegenInterval", cfg.gameplay.idleEnergyRegenInterval));
        cfg.gameplay.activeEnergyDrainInterval = std::max(1, readInt(gameplay, "activeEnergyDrainInterval", cfg.gameplay.activeEnergyDrainInterval));
        cfg.gameplay.starvationTicksToDamage = std::max(1, readInt(gameplay, "starvationTicksToDamage", cfg.gameplay.starvationTicksToDamage));

        if(gameplay.contains("mobSpawn") && gameplay["mobSpawn"].is_object()) {
            const auto& spawn = gameplay["mobSpawn"];
            cfg.mobSpawn.count = std::max(0, readInt(spawn, "count", cfg.mobSpawn.count));
            cfg.mobSpawn.offsetX = readInt(spawn, "offsetX", cfg.mobSpawn.offsetX);
            cfg.mobSpawn.offsetY = readInt(spawn, "offsetY", cfg.mobSpawn.offsetY);
            cfg.mobSpawn.jitterMin = readInt(spawn, "jitterMin", cfg.mobSpawn.jitterMin);
            cfg.mobSpawn.jitterMax = readInt(spawn, "jitterMax", cfg.mobSpawn.jitterMax);
            if(cfg.mobSpawn.jitterMax < cfg.mobSpawn.jitterMin) {
                std::swap(cfg.mobSpawn.jitterMin, cfg.mobSpawn.jitterMax);
            }
            cfg.mobSpawn.passableRetries = std::max(1, readInt(spawn, "passableRetries", cfg.mobSpawn.passableRetries));
        }

        cfg.mobSpatialCellSize = std::max(1, readInt(gameplay, "mobSpatialCellSize", cfg.mobSpatialCellSize));
    }

    if(root.contains("render") && root["render"].is_object()) {
        cfg.render.stepIntervalSeconds = std::max(0.01F, readFloat(root["render"], "stepIntervalSeconds", cfg.render.stepIntervalSeconds));
    }
}

void parseBehaviorProfiles(GameConfig& cfg, const json& root) {
    if(root.contains("entityBehaviorProfiles") && root["entityBehaviorProfiles"].is_array()) {
        cfg.mobBehaviorProfiles.clear();
        for(const auto& node : root["entityBehaviorProfiles"]) {
            if(!node.is_object()) {
                continue;
            }

            MobBehaviorProfile profile;
            profile.id = node.value("id", profile.id);
            profile.controller = node.value("controller", profile.controller);
            profile.detectRange = std::max(0, readInt(node, "detectRange", profile.detectRange));
            profile.loseRange = std::max(profile.detectRange, readInt(node, "loseRange", profile.loseRange));
            profile.preferredDistance = std::max(0, readInt(node, "preferredDistance", profile.preferredDistance));
            profile.wanderChance = std::clamp(readFloat(node, "wanderChance", profile.wanderChance), 0.0F, 1.0F);
            profile.idleChance = std::clamp(readFloat(node, "idleChance", profile.idleChance), 0.0F, 1.0F);
            profile.moveInterval = std::max(1, readInt(node, "moveInterval", profile.moveInterval));
            profile.contactDamage = std::max(0, readInt(node, "contactDamage", profile.contactDamage));
            profile.defaultAggro = readBool(node, "defaultAggro", profile.defaultAggro);

            if(!profile.id.empty()) {
                cfg.mobBehaviorProfiles.push_back(std::move(profile));
            }
        }
    }

    if(root.contains("entityBehaviorMap") && root["entityBehaviorMap"].is_object()) {
        cfg.entityBehaviorMap.clear();
        for(auto it = root["entityBehaviorMap"].begin(); it != root["entityBehaviorMap"].end(); ++it) {
            if(!it.value().is_string()) {
                continue;
            }
            cfg.entityBehaviorMap[normalizeId(it.key())] = it.value().get<std::string>();
        }
    }

    if(root.contains("spawnTable") && root["spawnTable"].is_array()) {
        cfg.spawnTable.clear();
        for(const auto& node : root["spawnTable"]) {
            if(!node.is_object()) {
                continue;
            }

            EntitySpawnEntry entry;
            entry.entityId = normalizeId(node.value("entityId", entry.entityId));
            entry.behaviorType = node.value("behaviorType", entry.behaviorType);
            entry.hp = std::max(1, readInt(node, "hp", entry.hp));
            entry.aggro = readBool(node, "aggro", entry.aggro);
            entry.variant = node.value("variant", entry.variant);
            entry.weight = std::max(1, readInt(node, "weight", entry.weight));

            if(!entry.entityId.empty()) {
                cfg.spawnTable.push_back(std::move(entry));
            }
        }
    }

    if(cfg.mobBehaviorProfiles.empty()) {
        cfg.mobBehaviorProfiles = makeDefaultConfig().mobBehaviorProfiles;
    }
}

}  // namespace

const MobBehaviorProfile* GameConfig::findMobBehavior(std::string_view behaviorType) const {
    if(behaviorType.empty()) {
        behaviorType = "default";
    }

    for(const auto& profile : mobBehaviorProfiles) {
        if(profile.id == behaviorType) {
            return &profile;
        }
    }

    for(const auto& profile : mobBehaviorProfiles) {
        if(profile.id == "default") {
            return &profile;
        }
    }

    return mobBehaviorProfiles.empty() ? nullptr : &mobBehaviorProfiles.front();
}

std::string GameConfig::behaviorTypeForEntity(std::string_view entityId, std::string_view fallbackType) const {
    const std::string id = normalizeId(std::string(entityId));
    const auto it = entityBehaviorMap.find(id);
    if(it != entityBehaviorMap.end()) {
        return it->second;
    }
    return std::string(fallbackType.empty() ? "default" : fallbackType);
}

const GameConfig& gameConfig() {
    return gConfig;
}

GameConfig& mutableGameConfig() {
    return gConfig;
}

void resetGameConfigToDefaults() {
    gConfig = makeDefaultConfig();
}

bool loadGameConfigFile(const std::filesystem::path& path, std::string* errorMessage) {
    GameConfig cfg = makeDefaultConfig();

    if(!std::filesystem::exists(path)) {
        gConfig = std::move(cfg);
        return true;
    }

    std::ifstream in(path);
    if(!in.is_open()) {
        if(errorMessage) {
            *errorMessage = "cannot open game config: " + path.string();
        }
        return false;
    }

    std::stringstream buffer;
    buffer << in.rdbuf();

    json root;
    try {
        root = json::parse(buffer.str());
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = "game config parse error in " + path.string() + ": " + ex.what();
        }
        return false;
    }

    if(!root.is_object()) {
        if(errorMessage) {
            *errorMessage = "game config root must be an object";
        }
        return false;
    }

    parseWorldConfig(cfg, root);
    parseGameplayConfig(cfg, root);
    parseBehaviorProfiles(cfg, root);

    cfg.gameplay.hotbarSlots = std::clamp(cfg.gameplay.hotbarSlots, 1, cfg.gameplay.inventorySlots);

    gConfig = std::move(cfg);
    return true;
}

}  // namespace stoneforge
