#include "stoneforge/simulation.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>

#include "stoneforge/game_config.hpp"
#include "stoneforge/item.hpp"
#include "stoneforge/object.hpp"
#include "stoneforge/recipe.hpp"

namespace stoneforge {

namespace {

int manhattanDistance(const Vec2i& a, const Vec2i& b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

int floorDiv(int value, int divisor) {
    const int q = value / divisor;
    const int r = value % divisor;
    return (r != 0 && ((r > 0) != (divisor > 0))) ? q - 1 : q;
}

std::int64_t spatialKey(int x, int y, int cellSize) {
    const int cx = floorDiv(x, cellSize);
    const int cy = floorDiv(y, cellSize);
    const std::uint64_t hi = static_cast<std::uint64_t>(static_cast<std::uint32_t>(cx));
    const std::uint64_t lo = static_cast<std::uint64_t>(static_cast<std::uint32_t>(cy));
    return static_cast<std::int64_t>((hi << 32) | lo);
}

std::int64_t chunkKeyFromCoords(int cx, int cy) {
    const std::uint64_t hi = static_cast<std::uint64_t>(static_cast<std::uint32_t>(cx));
    const std::uint64_t lo = static_cast<std::uint64_t>(static_cast<std::uint32_t>(cy));
    return static_cast<std::int64_t>((hi << 32) | lo);
}

std::string mobVariantForBiomeTag(int biomeTag) {
    switch(biomeTag) {
        case 0:
            return "biome_grasland";
        case 1:
            return "biome_wald";
        case 2:
            return "biome_wueste";
        case 3:
            return "biome_bergland";
        case 4:
            return "biome_steppe";
        case 5:
            return "biome_tundra";
        case 6:
            return "biome_hoelle";
        default:
            return "biome_default";
    }
}

Vec2i randomCardinalStep(std::mt19937_64& rng, float idleChance) {
    std::uniform_real_distribution<float> chance(0.0F, 1.0F);
    if(chance(rng) < std::clamp(idleChance, 0.0F, 1.0F)) {
        return {0, 0};
    }

    std::uniform_int_distribution<int> dir(0, 3);
    switch(dir(rng)) {
        case 0:
            return {0, -1};
        case 1:
            return {0, 1};
        case 2:
            return {-1, 0};
        case 3:
            return {1, 0};
        default:
            return {0, 0};
    }
}

Vec2i stepToward(const Vec2i& from, const Vec2i& target) {
    const int dx = target.x - from.x;
    const int dy = target.y - from.y;

    if(std::abs(dx) >= std::abs(dy)) {
        return {dx == 0 ? 0 : (dx > 0 ? 1 : -1), 0};
    }
    return {0, dy == 0 ? 0 : (dy > 0 ? 1 : -1)};
}

Vec2i stepAway(const Vec2i& from, const Vec2i& danger) {
    const int dx = from.x - danger.x;
    const int dy = from.y - danger.y;

    if(std::abs(dx) >= std::abs(dy)) {
        return {dx == 0 ? 0 : (dx > 0 ? 1 : -1), 0};
    }
    return {0, dy == 0 ? 0 : (dy > 0 ? 1 : -1)};
}

struct MobDecision {
    Vec2i step{0, 0};
    bool aggro = false;
};

MobDecision decideZombieStep(const Mob& mob, const MobBehaviorProfile& profile, const Vec2i& player, std::mt19937_64& rng) {
    const int dist = manhattanDistance(mob.pos, player);
    bool aggro = mob.aggro;

    if(dist <= profile.detectRange) {
        aggro = true;
    } else if(dist > profile.loseRange) {
        aggro = false;
    }

    if(aggro) {
        return {stepToward(mob.pos, player), true};
    }

    std::uniform_real_distribution<float> chance(0.0F, 1.0F);
    if(chance(rng) < std::clamp(profile.wanderChance, 0.0F, 1.0F)) {
        return {randomCardinalStep(rng, profile.idleChance), false};
    }
    return {{0, 0}, false};
}

MobDecision decideAnimalStep(const Mob& mob, const MobBehaviorProfile& profile, const Vec2i& player, std::mt19937_64& rng) {
    const int dist = manhattanDistance(mob.pos, player);
    if(profile.preferredDistance > 0 && dist <= profile.preferredDistance) {
        return {stepAway(mob.pos, player), false};
    }

    std::uniform_real_distribution<float> chance(0.0F, 1.0F);
    if(chance(rng) < std::clamp(profile.wanderChance, 0.0F, 1.0F)) {
        return {randomCardinalStep(rng, profile.idleChance), false};
    }
    return {{0, 0}, false};
}

MobDecision decideBossStep(const Mob& mob, const MobBehaviorProfile& profile, const Vec2i& player, std::mt19937_64& rng) {
    const int dist = manhattanDistance(mob.pos, player);
    if(dist <= profile.detectRange) {
        return {stepToward(mob.pos, player), true};
    }

    std::uniform_real_distribution<float> chance(0.0F, 1.0F);
    if(chance(rng) < std::clamp(profile.wanderChance, 0.0F, 1.0F)) {
        return {randomCardinalStep(rng, profile.idleChance), true};
    }
    return {{0, 0}, true};
}

MobDecision decideMobStep(const Mob& mob, const MobBehaviorProfile& profile, const Vec2i& player, std::mt19937_64& rng) {
    if(profile.controller == "animal") {
        return decideAnimalStep(mob, profile, player, rng);
    }
    if(profile.controller == "boss") {
        return decideBossStep(mob, profile, player, rng);
    }
    return decideZombieStep(mob, profile, player, rng);
}

const EntitySpawnEntry& pickSpawnEntry(const GameConfig& cfg, std::mt19937_64& rng) {
    if(cfg.spawnTable.empty()) {
        static const EntitySpawnEntry fallback;
        return fallback;
    }

    int totalWeight = 0;
    for(const auto& entry : cfg.spawnTable) {
        totalWeight += std::max(1, entry.weight);
    }

    std::uniform_int_distribution<int> dist(1, std::max(1, totalWeight));
    int r = dist(rng);
    for(const auto& entry : cfg.spawnTable) {
        r -= std::max(1, entry.weight);
        if(r <= 0) {
            return entry;
        }
    }

    return cfg.spawnTable.back();
}

}  // namespace

Simulation::Simulation() {
    reset(0);
}

void Simulation::reset(std::uint64_t seed) {
    const auto& cfg = gameConfig();

    rng_.seed(seed);
    world_.reset(seed);

    player_ = world_.spawnPoint();
    facing_ = {1, 0};

    hp_ = 10;
    energy_ = 100;

    observationRadius_ = std::max(1, cfg.gameplay.observationRadius);
    maxSteps_ = std::max(1, cfg.gameplay.maxSteps);

    const int inventoryCount = std::max(1, cfg.gameplay.inventorySlots);
    inventorySlots_.assign(static_cast<std::size_t>(inventoryCount), InventorySlot{});
    inventoryStackLimit_ = std::max(1, cfg.gameplay.inventoryStackLimit);
    hotbarSlotCount_ = std::clamp(cfg.gameplay.hotbarSlots, 1, inventoryCount);
    hotbarSelection_ = 0;

    axeLevel_ = 0;
    pickaxeLevel_ = 0;
    clearMiningProgress();

    steps_ = 0;
    starvationTicks_ = 0;
    proximityDamageAccumulator_ = 0.0F;
    done_ = false;
    reachedExit_ = false;
    exitUnlocked_ = !mobsKilledUnlocksExit_;
    totalKills_ = 0;
    visitedTiles_.clear();
    visitedTiles_.insert(spatialKey(player_.x, player_.y, 1));

    mobSpatialCellSize_ = std::max(1, cfg.mobSpatialCellSize);
    mobs_.clear();
    mobSpawnCheckedChunks_.clear();

    rebuildMobSpatialIndex();
    updateMobs();
}

StepResult Simulation::step(Action action) {
    if(done_) {
        return StepResult{0.0F, true, reachedExit_, steps_};
    }

    const auto& cfg = gameConfig();
    const int distanceBefore = manhattanDistance(player_, world_.exitPoint());
    const int hpBefore = hp_;
    const int mobsBefore = static_cast<int>(mobs_.size());
    const bool idleAction = (action == Action::Wait || action == Action::Noop);
    bool moveBlocked = false;

    switch(action) {
        case Action::MoveUp:
            clearMiningProgress();
            if(!tryMove({0, -1})) { moveBlocked = true; }
            break;
        case Action::MoveDown:
            clearMiningProgress();
            if(!tryMove({0, 1})) { moveBlocked = true; }
            break;
        case Action::MoveLeft:
            clearMiningProgress();
            if(!tryMove({-1, 0})) { moveBlocked = true; }
            break;
        case Action::MoveRight:
            clearMiningProgress();
            if(!tryMove({1, 0})) { moveBlocked = true; }
            break;
        case Action::Mine:
            mineForward();
            break;
        case Action::Place:
            clearMiningProgress();
            placeForward();
            break;
        case Action::Use:
            clearMiningProgress();
            useAction();
            break;
        case Action::Wait:
        case Action::Noop:
            clearMiningProgress();
            break;
    }

    if(idleAction) {
        if(energy_ < 100 && (steps_ % std::max(1, cfg.gameplay.idleEnergyRegenInterval) == 0)) {
            energy_ = std::min(100, energy_ + 1);
        }
    } else if(steps_ % std::max(1, cfg.gameplay.activeEnergyDrainInterval) == 0) {
        energy_ = std::max(0, energy_ - 1);
    }

    updateMobs();

    const int mobsKilled = mobsBefore - static_cast<int>(mobs_.size());
    if(mobsKilled > 0) {
        totalKills_ += mobsKilled;
    }
    const std::int64_t tileKey = spatialKey(player_.x, player_.y, 1);
    const bool newTileVisited = visitedTiles_.insert(tileKey).second;
    bool exitJustUnlocked = false;
    if(mobsKilledUnlocksExit_ && !exitUnlocked_ && totalKills_ >= killsRequired_) {
        exitUnlocked_ = true;
        exitJustUnlocked = true;
    }

    bool mobInThreeByThree = false;
    for(const auto& mob : mobs_) {
        if(std::abs(mob.pos.x - player_.x) <= 1 && std::abs(mob.pos.y - player_.y) <= 1) {
            mobInThreeByThree = true;
            break;
        }
    }

    if(mobInThreeByThree) {
        proximityDamageAccumulator_ += cfg.render.stepIntervalSeconds;
        while(proximityDamageAccumulator_ >= 1.0F) {
            hp_ -= 1;
            proximityDamageAccumulator_ -= 1.0F;
        }
    } else {
        proximityDamageAccumulator_ = 0.0F;
    }

    if(energy_ <= 0) {
        ++starvationTicks_;
        if(starvationTicks_ >= std::max(1, cfg.gameplay.starvationTicksToDamage)) {
            hp_ -= 1;
            starvationTicks_ = 0;
        }
    } else {
        starvationTicks_ = 0;
    }

    if(exitUnlocked_ && player_ == world_.exitPoint()) {
        reachedExit_ = true;
        done_ = true;
    }

    if(hp_ <= 0) {
        done_ = true;
    }

    ++steps_;
    if(steps_ >= maxSteps_) {
        done_ = true;
    }

    const int distanceAfter = manhattanDistance(player_, world_.exitPoint());
    const float reward = computeReward(reachedExit_, hpBefore, distanceBefore, distanceAfter,
                                       mobsKilled, exitJustUnlocked, exitUnlocked_, moveBlocked,
                                       newTileVisited, idleAction);

    return StepResult{reward, done_, reachedExit_, steps_};
}

void Simulation::setMiningTargetOverride(const Vec2i& target) {
    miningTargetOverrideActive_ = true;
    miningTargetOverride_ = target;
}

void Simulation::clearMiningTargetOverride() {
    miningTargetOverrideActive_ = false;
}

Observation Simulation::getObservation() const {
    Observation out{};
    const int side = 2 * observationRadius_ + 1;
    out.grid.reserve(side * side);

    for(int dy = -observationRadius_; dy <= observationRadius_; ++dy) {
        for(int dx = -observationRadius_; dx <= observationRadius_; ++dx) {
            const int wx = player_.x + dx;
            const int wy = player_.y + dy;

            int value = static_cast<int>(world_.tileAt(wx, wy));
            if(hasMobAt(wx, wy)) {
                value = 20;
            }

            if(wx == player_.x && wy == player_.y) {
                value = 30;
            }

            out.grid.push_back(value);
        }
    }

    out.hp = hp_;
    out.energy = energy_;
    out.inventory = inventory();
    const Vec2i exit = world_.exitPoint();
    out.exitDx = exit.x - player_.x;
    out.exitDy = exit.y - player_.y;

    return out;
}

Vec2i Simulation::playerPos() const {
    return player_;
}

Vec2i Simulation::exitPos() const {
    return world_.exitPoint();
}

TileType Simulation::tileAt(int x, int y) const {
    return world_.tileAt(x, y);
}

int Simulation::biomeTagAt(int x, int y) const {
    return world_.biomeTagAt(x, y);
}

std::string Simulation::biomeNameAt(int x, int y) const {
    return std::string(world_.biomeNameAt(x, y));
}

bool Simulation::isLakeAt(int x, int y) const {
    return world_.isLakeAt(x, y);
}

bool Simulation::isPlayerInLake() const {
    return world_.isLakeAt(player_.x, player_.y);
}

const std::vector<Mob>& Simulation::mobs() const {
    return mobs_;
}

std::vector<const Mob*> Simulation::mobsInRect(int minX, int minY, int maxX, int maxY) const {
    if(maxX < minX) {
        std::swap(minX, maxX);
    }
    if(maxY < minY) {
        std::swap(minY, maxY);
    }

    std::vector<const Mob*> out;
    if(mobs_.empty()) {
        return out;
    }

    const int cellSize = std::max(1, mobSpatialCellSize_);
    const int minCx = floorDiv(minX, cellSize);
    const int maxCx = floorDiv(maxX, cellSize);
    const int minCy = floorDiv(minY, cellSize);
    const int maxCy = floorDiv(maxY, cellSize);

    for(int cy = minCy; cy <= maxCy; ++cy) {
        for(int cx = minCx; cx <= maxCx; ++cx) {
            const std::uint64_t hi = static_cast<std::uint64_t>(static_cast<std::uint32_t>(cx));
            const std::uint64_t lo = static_cast<std::uint64_t>(static_cast<std::uint32_t>(cy));
            const std::int64_t key = static_cast<std::int64_t>((hi << 32) | lo);
            const auto it = mobSpatialBuckets_.find(key);
            if(it == mobSpatialBuckets_.end()) {
                continue;
            }

            for(const std::size_t idx : it->second) {
                if(idx >= mobs_.size()) {
                    continue;
                }
                const auto& mob = mobs_[idx];
                if(mob.pos.x >= minX && mob.pos.x <= maxX && mob.pos.y >= minY && mob.pos.y <= maxY) {
                    out.push_back(&mob);
                }
            }
        }
    }

    return out;
}

bool Simulation::hasMobAt(int x, int y) const {
    const auto it = mobSpatialBuckets_.find(spatialKey(x, y, std::max(1, mobSpatialCellSize_)));
    if(it == mobSpatialBuckets_.end()) {
        return false;
    }

    for(const std::size_t idx : it->second) {
        if(idx < mobs_.size() && mobs_[idx].pos.x == x && mobs_[idx].pos.y == y) {
            return true;
        }
    }
    return false;
}

int Simulation::hp() const {
    return hp_;
}

int Simulation::energy() const {
    return energy_;
}

int Simulation::inventory() const {
    int total = 0;
    for(const auto& slot : inventorySlots_) {
        total += std::max(0, slot.count);
    }
    return total;
}

int Simulation::wood() const {
    return itemCountByKey("stoneforge:wood");
}

int Simulation::planks() const {
    return itemCountByKey("stoneforge:planks");
}

int Simulation::sticks() const {
    return itemCountByKey("stoneforge:sticks");
}

int Simulation::ore() const {
    return itemCountByKey("stoneforge:ore");
}

int Simulation::workbenches() const {
    return itemCountByKey("stoneforge:workbench_kit");
}

int Simulation::axeLevel() const {
    return axeLevel_;
}

int Simulation::pickaxeLevel() const {
    return pickaxeLevel_;
}

bool Simulation::isNearWorkbench() const {
    for(int dy = -2; dy <= 2; ++dy) {
        for(int dx = -2; dx <= 2; ++dx) {
            if(world_.tileAt(player_.x + dx, player_.y + dy) == TileType::Workbench) {
                return true;
            }
        }
    }
    return false;
}

int Simulation::hotbarSelection() const {
    return hotbarSelection_;
}

void Simulation::setHotbarSelection(int hotbarIndex) {
    hotbarSelection_ = std::clamp(hotbarIndex, 0, hotbarSlotCount_ - 1);
}

int Simulation::selectedHotbarSlotIndex() const {
    return hotbarSelection_;
}

InventorySlot Simulation::hotbarSlot(int hotbarIndex) const {
    if(hotbarIndex < 0 || hotbarIndex >= hotbarSlotCount_) {
        return InventorySlot{};
    }
    return inventorySlot(hotbarIndex);
}

int Simulation::hotbarSlotCount() const {
    return hotbarSlotCount_;
}

int Simulation::inventorySlotCount() const {
    return static_cast<int>(inventorySlots_.size());
}

int Simulation::inventoryStackLimit() const {
    return inventoryStackLimit_;
}

InventorySlot Simulation::inventorySlot(int index) const {
    if(index < 0 || index >= inventorySlotCount()) {
        return InventorySlot{};
    }
    return inventorySlots_[static_cast<std::size_t>(index)];
}

bool Simulation::moveInventoryStack(int fromIndex, int toIndex) {
    if(fromIndex < 0 || toIndex < 0 || fromIndex >= inventorySlotCount() || toIndex >= inventorySlotCount() || fromIndex == toIndex) {
        return false;
    }

    auto& src = inventorySlots_[static_cast<std::size_t>(fromIndex)];
    auto& dst = inventorySlots_[static_cast<std::size_t>(toIndex)];
    if(src.itemId.empty() || src.count <= 0) {
        return false;
    }

    if(dst.itemId.empty() || dst.count <= 0) {
        dst = src;
        src = InventorySlot{};
        return true;
    }

    if(dst.itemId == src.itemId) {
        const int stackLimit = std::min(inventoryStackLimit_, std::max(1, itemMaxStack(dst.itemId)));
        const int room = stackLimit - dst.count;
        if(room <= 0) {
            return false;
        }

        const int moved = std::min(room, src.count);
        dst.count += moved;
        src.count -= moved;
        if(src.count <= 0) {
            src = InventorySlot{};
        }
        return moved > 0;
    }

    std::swap(src, dst);
    return true;
}

bool Simulation::splitInventoryStack(int fromIndex, int toIndex) {
    if(fromIndex < 0 || toIndex < 0 || fromIndex >= inventorySlotCount() || toIndex >= inventorySlotCount() || fromIndex == toIndex) {
        return false;
    }

    auto& src = inventorySlots_[static_cast<std::size_t>(fromIndex)];
    auto& dst = inventorySlots_[static_cast<std::size_t>(toIndex)];
    if(src.itemId.empty() || src.count <= 1) {
        return false;
    }

    if(!dst.itemId.empty() && dst.itemId != src.itemId) {
        return false;
    }

    int moved = (src.count + 1) / 2;
    if(dst.itemId.empty()) {
        moved = std::min(moved, std::min(inventoryStackLimit_, std::max(1, itemMaxStack(src.itemId))));
        dst.itemId = src.itemId;
        dst.count = moved;
        src.count -= moved;
        if(src.count <= 0) {
            src = InventorySlot{};
        }
        return moved > 0;
    }

    const int room = std::min(inventoryStackLimit_, std::max(1, itemMaxStack(dst.itemId))) - dst.count;
    if(room <= 0) {
        return false;
    }
    moved = std::min(moved, room);
    dst.count += moved;
    src.count -= moved;
    if(src.count <= 0) {
        src = InventorySlot{};
    }
    return moved > 0;
}

bool Simulation::canCraft(std::string_view recipeId) const {
    const RecipeBase* def = recipeCatalog().find(recipeId);
    if(def == nullptr) {
        return false;
    }

    if(def->requiresWorkbench() && !isNearWorkbench()) {
        return false;
    }

    for(const auto& input : def->inputs()) {
        if(!hasItemAmountByKey(input.itemId, input.count)) {
            return false;
        }
    }

    return def->canCraftWithTools(axeLevel_, pickaxeLevel_);
}

bool Simulation::craft(std::string_view recipeId) {
    if(!canCraft(recipeId)) {
        return false;
    }

    const RecipeBase* def = recipeCatalog().find(recipeId);
    if(def == nullptr) {
        return false;
    }

    const auto inventoryBefore = inventorySlots_;
    const int axeBefore = axeLevel_;
    const int pickaxeBefore = pickaxeLevel_;

    for(const auto& input : def->inputs()) {
        if(!removeItemByKey(input.itemId, input.count)) {
            inventorySlots_ = inventoryBefore;
            axeLevel_ = axeBefore;
            pickaxeLevel_ = pickaxeBefore;
            return false;
        }
    }

    for(const auto& output : def->outputs()) {
        if(!addItemByKey(output.itemId, output.count)) {
            inventorySlots_ = inventoryBefore;
            axeLevel_ = axeBefore;
            pickaxeLevel_ = pickaxeBefore;
            return false;
        }
    }

    def->applyToolUpgrades(axeLevel_, pickaxeLevel_);

    energy_ = std::max(0, energy_ - 1);
    return true;
}

bool Simulation::placeWorkbenchForward() {
    if(!hasItemAmountByKey("stoneforge:workbench_kit", 1)) {
        return false;
    }

    const Vec2i target{player_.x + facing_.x, player_.y + facing_.y};
    if(world_.tileAt(target.x, target.y) != TileType::Empty) {
        return false;
    }

    world_.setTile(target.x, target.y, TileType::Workbench);
    if(!removeItemByKey("stoneforge:workbench_kit", 1)) {
        world_.setTile(target.x, target.y, TileType::Empty);
        return false;
    }
    energy_ = std::max(0, energy_ - 1);
    return true;
}

bool Simulation::placeFromHotbarAt(const Vec2i& target) {
    if(!canPlaceFromHotbarAt(target)) {
        return false;
    }

    if(!tryPlaceFromSlotIndexAt(selectedHotbarSlotIndex(), target)) {
        return false;
    }

    energy_ = std::max(0, energy_ - 1);
    return true;
}

bool Simulation::contextUseAt(const Vec2i& target) {
    if(!isWithinMiningRange(target) || !hasLineOfSightTo(target)) {
        return false;
    }

    const TileType tile = world_.tileAt(target.x, target.y);
    if(tile == TileType::Empty) {
        return placeFromHotbarAt(target);
    }

    if(tile == TileType::Workbench) {
        const int energyBefore = energy_;
        const int invBefore = inventory();
        const int axeBefore = axeLevel_;
        const int pickaxeBefore = pickaxeLevel_;
        useAction();
        return energy_ != energyBefore || inventory() != invBefore || axeLevel_ != axeBefore || pickaxeLevel_ != pickaxeBefore;
    }

    return false;
}

bool Simulation::isMining() const {
    return miningActive_;
}

Vec2i Simulation::miningTarget() const {
    return miningTarget_;
}

TileType Simulation::miningTile() const {
    return miningTile_;
}

float Simulation::miningProgress01() const {
    if(!miningActive_) {
        return 0.0F;
    }
    const float hardness = std::max(0.001F, miningHardness(miningTile_));
    return std::clamp(miningProgress_ / hardness, 0.0F, 1.0F);
}

bool Simulation::hasLineOfSightTo(const Vec2i& target) const {
    int x0 = player_.x;
    int y0 = player_.y;
    const int x1 = target.x;
    const int y1 = target.y;

    int dx = std::abs(x1 - x0);
    const int sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0);
    const int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while(!(x0 == x1 && y0 == y1)) {
        const int e2 = 2 * err;
        if(e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if(e2 <= dx) {
            err += dx;
            y0 += sy;
        }

        if(x0 == x1 && y0 == y1) {
            break;
        }

        const TileType losTile = world_.tileAt(x0, y0);
        if(objectForTile(losTile).blocksLineOfSight()) {
            return false;
        }
    }

    return true;
}

bool Simulation::canMineTarget(const Vec2i& target) const {
    const TileType tile = world_.tileAt(target.x, target.y);
    return isMineableTile(tile) && isWithinMiningRange(target) && hasLineOfSightTo(target);
}

bool Simulation::canPlaceFromHotbarAt(const Vec2i& target) const {
    if(target == player_) {
        return false;
    }
    if(!isWithinMiningRange(target) || !hasLineOfSightTo(target)) {
        return false;
    }
    if(world_.tileAt(target.x, target.y) != TileType::Empty) {
        return false;
    }

    const auto slot = hotbarSlot(hotbarSelection_);
    if(slot.itemId.empty() || slot.count <= 0) {
        return false;
    }

    return itemPlacementTile(slot.itemId) != TileType::Empty;
}

TileType Simulation::previewPlacementTileForSelectedHotbar() const {
    const auto slot = hotbarSlot(hotbarSelection_);
    if(slot.itemId.empty() || slot.count <= 0) {
        return TileType::Empty;
    }
    return itemPlacementTile(slot.itemId);
}

float Simulation::miningRangeTiles() const {
    const auto& cfg = gameConfig().gameplay;
    const int bestToolLevel = std::max(axeLevel_, pickaxeLevel_);
    return cfg.miningRangeBaseTiles + static_cast<float>(bestToolLevel) * cfg.miningRangePerToolLevel;
}

int Simulation::observationRadius() const {
    return observationRadius_;
}

int Simulation::observationSize() const {
    const int side = 2 * observationRadius_ + 1;
    return side * side + 5;
}

int Simulation::steps() const {
    return steps_;
}

bool Simulation::done() const {
    return done_;
}

bool Simulation::reachedExit() const {
    return reachedExit_;
}

bool Simulation::commandSetTile(const Vec2i& target, TileType tile) {
    if(target == player_ && tile != TileType::Empty) {
        return false;
    }

    world_.setTile(target.x, target.y, tile);
    return true;
}

bool Simulation::commandSpawnEntity(
    const std::string& entityId,
    const Vec2i& target,
    int hp,
    bool aggro,
    const std::string& variant,
    const std::string& behaviorType
) {
    if(!world_.isPassable(target.x, target.y)) {
        return false;
    }

    if(hasMobAt(target.x, target.y)) {
        return false;
    }

    (void)aggro;
    const int biomeTag = world_.biomeTagAt(target.x, target.y);
    std::string resolvedVariant = variant;
    if(resolvedVariant.empty() || resolvedVariant == "default") {
        resolvedVariant = mobVariantForBiomeTag(biomeTag);
    }

    std::string resolvedBehavior = behaviorType;
    if(resolvedBehavior.empty()) {
        resolvedBehavior = "passive";
    }

    mobs_.push_back(Mob{target, std::max(1, hp), entityId, resolvedBehavior, false, resolvedVariant, biomeTag});
    const int chunkX = floorDiv(target.x, World::kChunkSize);
    const int chunkY = floorDiv(target.y, World::kChunkSize);
    mobSpawnCheckedChunks_.insert(chunkKeyFromCoords(chunkX, chunkY));
    rebuildMobSpatialIndex();
    return true;
}

bool Simulation::commandGiveItem(std::string_view itemId, int amount) {
    return addItemByKey(itemId, amount);
}

bool Simulation::commandTeleportPlayer(const Vec2i& target) {
    if(!world_.isPassable(target.x, target.y)) {
        return false;
    }
    if(hasMobAt(target.x, target.y)) {
        return false;
    }
    player_ = target;
    clearMiningProgress();
    return true;
}

bool Simulation::tryMove(const Vec2i& delta) {
    const Vec2i candidate{player_.x + delta.x, player_.y + delta.y};
    facing_ = delta;

    if(world_.isPassable(candidate.x, candidate.y) && !hasMobAt(candidate.x, candidate.y)) {
        player_ = candidate;
        return true;
    }
    return false;
}

Vec2i Simulation::actionDelta(Action action) const {
    switch(action) {
        case Action::MoveUp:
            return {0, -1};
        case Action::MoveDown:
            return {0, 1};
        case Action::MoveLeft:
            return {-1, 0};
        case Action::MoveRight:
            return {1, 0};
        default:
            return facing_;
    }
}

void Simulation::mineForward() {
    const Vec2i target = miningTargetOverrideActive_ ? miningTargetOverride_ : Vec2i{player_.x + facing_.x, player_.y + facing_.y};
    const TileType tile = world_.tileAt(target.x, target.y);

    if(!canMineTarget(target)) {
        clearMiningProgress();
        return;
    }

    if(!miningActive_ || !(miningTarget_ == target) || miningTile_ != tile) {
        miningActive_ = true;
        miningTarget_ = target;
        miningTile_ = tile;
        miningProgress_ = 0.0F;
    }

    miningProgress_ += miningSpeed(tile);
    energy_ = std::max(0, energy_ - 1);

    if(miningProgress_ < miningHardness(tile)) {
        return;
    }

    const auto& object = objectForTile(tile);
    if(!object.isMineable()) {
        clearMiningProgress();
        return;
    }

    if(!addDropItem(object.minedDrop())) {
        clearMiningProgress();
        return;
    }

    world_.setTile(target.x, target.y, TileType::Empty);
    clearMiningProgress();
}

void Simulation::placeForward() {
    if(!tryPlaceFromSlotIndex(selectedHotbarSlotIndex())) {
        return;
    }
    energy_ = std::max(0, energy_ - 1);
}

void Simulation::useAction() {
    // Sword slash: hit all mobs in a 3x3 area centered on the player.
    for(auto& mob : mobs_) {
        const int dx = std::abs(mob.pos.x - player_.x);
        const int dy = std::abs(mob.pos.y - player_.y);
        if(dx <= 1 && dy <= 1) {
            mob.hp -= 1;
        }
    }

    mobs_.erase(
        std::remove_if(
            mobs_.begin(),
            mobs_.end(),
            [](const Mob& mob) {
                return mob.hp <= 0;
            }
        ),
        mobs_.end()
    );

    rebuildMobSpatialIndex();
    energy_ = std::max(0, energy_ - 1);
}

void Simulation::clearMiningProgress() {
    miningActive_ = false;
    miningTarget_ = {0, 0};
    miningTile_ = TileType::Empty;
    miningProgress_ = 0.0F;
}

int Simulation::itemCount(ItemId item) const {
    if(item == ItemId::None) {
        return 0;
    }
    return itemCountByKey(itemById(item).key());
}

bool Simulation::addItem(ItemId item, int amount) {
    if(item == ItemId::None) {
        return false;
    }
    return addItemByKey(itemById(item).key(), amount);
}

int Simulation::itemCountByKey(std::string_view itemId) const {
    const std::string normalized = normalizeItemKey(itemId);
    if(normalized.empty()) {
        return 0;
    }

    int total = 0;
    for(const auto& slot : inventorySlots_) {
        if(slot.itemId == normalized) {
            total += std::max(0, slot.count);
        }
    }
    return total;
}

bool Simulation::addItemByKey(std::string_view itemId, int amount) {
    const std::string normalized = normalizeItemKey(itemId);
    if(normalized.empty() || amount <= 0) {
        return false;
    }

    const int stackLimit = std::min(inventoryStackLimit_, std::max(1, itemMaxStack(normalized)));

    int totalRoom = 0;
    for(const auto& slot : inventorySlots_) {
        if(slot.itemId == normalized) {
            totalRoom += std::max(0, stackLimit - slot.count);
        } else if(slot.itemId.empty() || slot.count <= 0) {
            totalRoom += stackLimit;
        }
    }

    if(totalRoom < amount) {
        return false;
    }

    int left = amount;
    for(auto& slot : inventorySlots_) {
        if(left <= 0) {
            break;
        }
        if(slot.itemId != normalized) {
            continue;
        }

        const int room = std::max(0, stackLimit - slot.count);
        const int moved = std::min(room, left);
        slot.count += moved;
        left -= moved;
    }

    for(auto& slot : inventorySlots_) {
        if(left <= 0) {
            break;
        }
        if(!slot.itemId.empty() && slot.count > 0) {
            continue;
        }

        const int moved = std::min(stackLimit, left);
        slot.itemId = normalized;
        slot.count = moved;
        left -= moved;
    }

    return left == 0;
}

bool Simulation::addDropItem(ObjectDrop drop) {
    switch(drop) {
        case ObjectDrop::None:
            return true;
        case ObjectDrop::Wood:
            return addItemByKey("stoneforge:wood", 1);
        case ObjectDrop::Ore:
            return addItemByKey("stoneforge:ore", 1);
        case ObjectDrop::WorkbenchKit:
            return addItemByKey("stoneforge:workbench_kit", 1);
        default:
            return false;
    }
}

bool Simulation::removeItem(ItemId item, int amount) {
    if(item == ItemId::None) {
        return false;
    }
    return removeItemByKey(itemById(item).key(), amount);
}

bool Simulation::removeItemByKey(std::string_view itemId, int amount) {
    const std::string normalized = normalizeItemKey(itemId);
    if(normalized.empty() || amount <= 0) {
        return false;
    }
    if(!hasItemAmountByKey(normalized, amount)) {
        return false;
    }

    int left = amount;
    for(auto& slot : inventorySlots_) {
        if(left <= 0) {
            break;
        }
        if(slot.itemId != normalized || slot.count <= 0) {
            continue;
        }

        const int removed = std::min(slot.count, left);
        slot.count -= removed;
        left -= removed;
        if(slot.count <= 0) {
            slot = InventorySlot{};
        }
    }

    return left == 0;
}

bool Simulation::hasItemAmount(ItemId item, int amount) const {
    if(item == ItemId::None) {
        return false;
    }
    return hasItemAmountByKey(itemById(item).key(), amount);
}

bool Simulation::hasItemAmountByKey(std::string_view itemId, int amount) const {
    return itemCountByKey(itemId) >= amount;
}

bool Simulation::tryPlaceFromSlotIndex(int slotIndex) {
    const Vec2i target{player_.x + facing_.x, player_.y + facing_.y};
    return tryPlaceFromSlotIndexAt(slotIndex, target);
}

bool Simulation::tryPlaceFromSlotIndexAt(int slotIndex, const Vec2i& target) {
    if(slotIndex < 0 || slotIndex >= inventorySlotCount()) {
        return false;
    }

    if(target == player_) {
        return false;
    }

    if(world_.tileAt(target.x, target.y) != TileType::Empty) {
        return false;
    }

    auto& slot = inventorySlots_[static_cast<std::size_t>(slotIndex)];
    if(slot.itemId.empty() || slot.count <= 0) {
        return false;
    }

    const TileType placeTile = itemPlacementTile(slot.itemId);
    if(placeTile == TileType::Empty) {
        return false;
    }

    world_.setTile(target.x, target.y, placeTile);
    slot.count -= 1;
    if(slot.count <= 0) {
        slot = InventorySlot{};
    }
    return true;
}

float Simulation::miningHardness(TileType tile) const {
    return objectForTile(tile).miningHardness();
}

float Simulation::miningSpeed(TileType tile) const {
    return objectForTile(tile).miningSpeed(axeLevel_, pickaxeLevel_);
}

bool Simulation::isMineableTile(TileType tile) const {
    return objectForTile(tile).isMineable();
}

bool Simulation::isWithinMiningRange(const Vec2i& target) const {
    const float dx = static_cast<float>(target.x - player_.x);
    const float dy = static_cast<float>(target.y - player_.y);
    const float range = miningRangeTiles();
    return (dx * dx + dy * dy) <= (range * range);
}

void Simulation::updateMobs() {
    // Spawn passive mobs lazily per discovered chunk: 90% chance, max one mob per chunk.
    constexpr float kChunkSpawnChance = 0.00F;
    constexpr int kSpawnAttemptsPerChunk = 10;

    const int chunkSize = World::kChunkSize;
    const int playerChunkX = floorDiv(player_.x, chunkSize);
    const int playerChunkY = floorDiv(player_.y, chunkSize);
    const int chunkRadius = std::max(1, (observationRadius_ + chunkSize - 1) / chunkSize + 1);

    std::uniform_real_distribution<float> chanceDist(0.0F, 1.0F);
    std::uniform_int_distribution<int> tileDist(0, chunkSize - 1);

    for(int dy = -chunkRadius; dy <= chunkRadius; ++dy) {
        for(int dx = -chunkRadius; dx <= chunkRadius; ++dx) {
            const int cx = playerChunkX + dx;
            const int cy = playerChunkY + dy;
            const std::int64_t ck = chunkKeyFromCoords(cx, cy);

            if(mobSpawnCheckedChunks_.find(ck) != mobSpawnCheckedChunks_.end()) {
                continue;
            }
            mobSpawnCheckedChunks_.insert(ck);

            if(chanceDist(rng_) > kChunkSpawnChance) {
                continue;
            }

            const int chunkMinX = cx * chunkSize;
            const int chunkMinY = cy * chunkSize;

            bool spawned = false;
            for(int attempt = 0; attempt < kSpawnAttemptsPerChunk; ++attempt) {
                const int wx = chunkMinX + tileDist(rng_);
                const int wy = chunkMinY + tileDist(rng_);
                if(!world_.isPassable(wx, wy)) {
                    continue;
                }
                if(player_.x == wx && player_.y == wy) {
                    continue;
                }
                if(hasMobAt(wx, wy)) {
                    continue;
                }

                const int biomeTag = world_.biomeTagAt(wx, wy);
                mobs_.push_back(Mob{{wx, wy}, 1, "stoneforge:mob", "passive", false, mobVariantForBiomeTag(biomeTag), biomeTag});
                spawned = true;
                break;
            }

            if(!spawned) {
                for(int ly = 0; ly < chunkSize && !spawned; ++ly) {
                    for(int lx = 0; lx < chunkSize && !spawned; ++lx) {
                        const int wx = chunkMinX + lx;
                        const int wy = chunkMinY + ly;
                        if(!world_.isPassable(wx, wy)) {
                            continue;
                        }
                        if(player_.x == wx && player_.y == wy) {
                            continue;
                        }
                        if(hasMobAt(wx, wy)) {
                            continue;
                        }

                        const int biomeTag = world_.biomeTagAt(wx, wy);
                        mobs_.push_back(Mob{{wx, wy}, 1, "stoneforge:mob", "passive", false, mobVariantForBiomeTag(biomeTag), biomeTag});
                        spawned = true;
                    }
                }
            }
        }
    }

    auto tileKey = [](int x, int y) {
        const std::uint64_t hi = static_cast<std::uint64_t>(static_cast<std::uint32_t>(x));
        const std::uint64_t lo = static_cast<std::uint64_t>(static_cast<std::uint32_t>(y));
        return static_cast<std::int64_t>((hi << 32) | lo);
    };

    std::unordered_set<std::int64_t> occupied;
    occupied.reserve(mobs_.size() * 2 + 1);
    for(const auto& mob : mobs_) {
        occupied.insert(tileKey(mob.pos.x, mob.pos.y));
    }

    const bool mobsCanMoveThisStep = (steps_ % 2) == 0;

    for(auto& mob : mobs_) {
        if(!mobsCanMoveThisStep) {
            continue;
        }

        const int dx = player_.x - mob.pos.x;
        const int dy = player_.y - mob.pos.y;

        // 7x7 centered around mob => player visible when both axes are within [-3, +3].
        if(std::abs(dx) > 3 || std::abs(dy) > 3) {
            continue;
        }

        const Vec2i step = stepToward(mob.pos, player_);
        if(step.x == 0 && step.y == 0) {
            continue;
        }

        const Vec2i next{mob.pos.x + step.x, mob.pos.y + step.y};
        if(next == player_) {
            continue;
        }
        if(!world_.isPassable(next.x, next.y)) {
            continue;
        }

        const std::int64_t nextKey = tileKey(next.x, next.y);
        if(occupied.find(nextKey) != occupied.end()) {
            continue;
        }

        occupied.erase(tileKey(mob.pos.x, mob.pos.y));
        mob.pos = next;
        occupied.insert(nextKey);
    }

    rebuildMobSpatialIndex();
}

void Simulation::rebuildMobSpatialIndex() {
    mobSpatialBuckets_.clear();
    mobSpatialBuckets_.reserve(mobs_.size());

    const int cellSize = std::max(1, mobSpatialCellSize_);
    for(std::size_t i = 0; i < mobs_.size(); ++i) {
        const auto& mob = mobs_[i];
        mobSpatialBuckets_[spatialKey(mob.pos.x, mob.pos.y, cellSize)].push_back(i);
    }
}

float Simulation::computeReward(bool reachedExit, int hpBefore, int previousDistance, int currentDistance,
                                int mobsKilledThisStep, bool exitJustUnlocked, bool isExitUnlocked,
                                bool moveBlocked, bool newTileVisited, bool idleAction) const {
    float reward = -0.01F;

    if(newTileVisited) {
        reward += 0.02F;
    }

    if(idleAction) {
        reward -= 0.02F;
    }

    const int damage = std::max(0, hpBefore - hp_);
    reward -= static_cast<float>(damage) * 0.5F;

    if(moveBlocked) {
        reward -= 0.05F;
    }

    if(mobsKilledThisStep > 0) {
        reward += static_cast<float>(mobsKilledThisStep) * 2.0F;
    }

    if(exitJustUnlocked) {
        reward += 5.0F;
    }

    (void)isExitUnlocked;
    const int progress = previousDistance - currentDistance;
    reward += static_cast<float>(progress) * 0.10F;

    if(reachedExit) {
        reward += 100.0F;
    }

    if(done_ && !reachedExit) {
        reward -= 10.0F;
    }

    return reward;
}

}  // namespace stoneforge
