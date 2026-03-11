#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "stoneforge/types.hpp"

namespace stoneforge {

struct WorldGenConfig {
    Vec2i spawn{0, 0};
    Vec2i exit{64, 64};

    double coldBiomeMax = 0.33;
    double warmBiomeMax = 0.66;

    double coldWallThreshold = 0.11;
    double warmWallThreshold = 0.16;
    double mossWallThreshold = 0.08;

    double coldOreThreshold = 0.03;
    double warmOreThreshold = 0.05;
    double mossOreThreshold = 0.02;

    double warmTreeThreshold = 0.11;
    double mossTreeThreshold = 0.15;

    int spawnClearRadius = 2;
    int exitClearRadius = 1;

    std::uint64_t biomeSalt = 0xabcddcbaULL;
    std::uint64_t densitySalt = 0x10203040ULL;
    std::uint64_t oreSalt = 0x99887766ULL;
    std::uint64_t treeSalt = 0x55443322ULL;
};

struct GameplayConfig {
    int observationRadius = 5;
    int maxSteps = 2500;

    int inventorySlots = 24;
    int inventoryStackLimit = 64;
    int hotbarSlots = 9;

    float miningRangeBaseTiles = 4.5F;
    float miningRangePerToolLevel = 0.75F;

    int idleEnergyRegenInterval = 8;
    int activeEnergyDrainInterval = 18;
    int starvationTicksToDamage = 18;
};

struct MobSpawnConfig {
    int count = 3;
    int offsetX = 10;
    int offsetY = 10;
    int jitterMin = -8;
    int jitterMax = 8;
    int passableRetries = 20;
};

struct RenderConfig {
    float stepIntervalSeconds = 0.12F;
};

struct MobBehaviorProfile {
    std::string id = "default";
    std::string controller = "zombie";

    int detectRange = 9;
    int loseRange = 12;
    int preferredDistance = 0;

    float wanderChance = 0.6F;
    float idleChance = 0.2F;

    int moveInterval = 1;
    int contactDamage = 1;
    bool defaultAggro = false;
};

struct EntitySpawnEntry {
    std::string entityId = "stoneforge:mob";
    std::string behaviorType = "zombie";
    int hp = 1;
    bool aggro = false;
    std::string variant = "default";
    int weight = 1;
};

struct GameConfig {
    WorldGenConfig world;
    GameplayConfig gameplay;
    MobSpawnConfig mobSpawn;
    RenderConfig render;

    int mobSpatialCellSize = 8;

    std::vector<MobBehaviorProfile> mobBehaviorProfiles;
    std::unordered_map<std::string, std::string> entityBehaviorMap;
    std::vector<EntitySpawnEntry> spawnTable;

    const MobBehaviorProfile* findMobBehavior(std::string_view behaviorType) const;
    std::string behaviorTypeForEntity(std::string_view entityId, std::string_view fallbackType) const;
};

const GameConfig& gameConfig();
GameConfig& mutableGameConfig();
void resetGameConfigToDefaults();

bool loadGameConfigFile(const std::filesystem::path& path, std::string* errorMessage = nullptr);

}  // namespace stoneforge
