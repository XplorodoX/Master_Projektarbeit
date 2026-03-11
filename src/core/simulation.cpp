#include "stoneforge/simulation.hpp"

#include <algorithm>
#include <cmath>

#include "stoneforge/item.hpp"
#include "stoneforge/object.hpp"
#include "stoneforge/recipe.hpp"

namespace stoneforge {

namespace {

int manhattanDistance(const Vec2i& a, const Vec2i& b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

constexpr float kMiningRangeBaseTiles = 4.5F;

}  // namespace

Simulation::Simulation() {
    reset(0);
}

void Simulation::reset(std::uint64_t seed) {
    rng_.seed(seed);
    world_.reset(seed);

    player_ = world_.spawnPoint();
    facing_ = {1, 0};

    hp_ = 10;
    energy_ = 100;
    for(auto& slot : inventorySlots_) {
        slot = InventorySlot{};
    }
    hotbarSelection_ = 0;
    axeLevel_ = 0;
    pickaxeLevel_ = 0;
    clearMiningProgress();

    steps_ = 0;
    starvationTicks_ = 0;
    done_ = false;
    reachedExit_ = false;

    mobs_.clear();
    std::uniform_int_distribution<int> jitter(-8, 8);
    for(int i = 0; i < 3; ++i) {
        Vec2i p{player_.x + 10 + jitter(rng_), player_.y + 10 + jitter(rng_)};
        for(int attempts = 0; attempts < 20 && !world_.isPassable(p.x, p.y); ++attempts) {
            p = {player_.x + 10 + jitter(rng_), player_.y + 10 + jitter(rng_)};
        }
        if(world_.isPassable(p.x, p.y)) {
            mobs_.push_back(Mob{p, 1, "stoneforge:mob", false, "default"});
        }
    }
}

StepResult Simulation::step(Action action) {
    if(done_) {
        return StepResult{0.0F, true, reachedExit_, steps_};
    }

    const int distanceBefore = manhattanDistance(player_, world_.exitPoint());
    const int hpBefore = hp_;
    const bool idleAction = (action == Action::Wait || action == Action::Noop);

    switch(action) {
        case Action::MoveUp:
            clearMiningProgress();
            tryMove({0, -1});
            break;
        case Action::MoveDown:
            clearMiningProgress();
            tryMove({0, 1});
            break;
        case Action::MoveLeft:
            clearMiningProgress();
            tryMove({-1, 0});
            break;
        case Action::MoveRight:
            clearMiningProgress();
            tryMove({1, 0});
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

    // Energy tuning: no hard per-step drain; action costs come from actions,
    // passive drain is slower and idle recovers a bit.
    if(idleAction) {
        if(energy_ < 100 && (steps_ % 8 == 0)) {
            energy_ = std::min(100, energy_ + 1);
        }
    } else if(steps_ % 18 == 0) {
        energy_ = std::max(0, energy_ - 1);
    }

    updateMobs();

    if(energy_ <= 0) {
        ++starvationTicks_;
        if(starvationTicks_ >= 18) {
            hp_ -= 1;
            starvationTicks_ = 0;
        }
    } else {
        starvationTicks_ = 0;
    }

    if(player_ == world_.exitPoint()) {
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
    const float reward = computeReward(reachedExit_, hpBefore, distanceBefore, distanceAfter);

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
    out.grid.reserve((2 * kObservationRadius + 1) * (2 * kObservationRadius + 1));

    for(int dy = -kObservationRadius; dy <= kObservationRadius; ++dy) {
        for(int dx = -kObservationRadius; dx <= kObservationRadius; ++dx) {
            const int wx = player_.x + dx;
            const int wy = player_.y + dy;

            int value = static_cast<int>(world_.tileAt(wx, wy));
            for(const auto& mob : mobs_) {
                if(mob.pos.x == wx && mob.pos.y == wy) {
                    value = 20;
                    break;
                }
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

const std::vector<Mob>& Simulation::mobs() const {
    return mobs_;
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
    hotbarSelection_ = std::clamp(hotbarIndex, 0, kHotbarSlotCount - 1);
}

int Simulation::selectedHotbarSlotIndex() const {
    return hotbarSelection_;
}

InventorySlot Simulation::hotbarSlot(int hotbarIndex) const {
    if(hotbarIndex < 0 || hotbarIndex >= kHotbarSlotCount) {
        return InventorySlot{};
    }
    return inventorySlot(hotbarIndex);
}

int Simulation::inventorySlotCount() const {
    return kInventorySlotCount;
}

InventorySlot Simulation::inventorySlot(int index) const {
    if(index < 0 || index >= kInventorySlotCount) {
        return InventorySlot{};
    }
    return inventorySlots_[static_cast<std::size_t>(index)];
}

bool Simulation::moveInventoryStack(int fromIndex, int toIndex) {
    if(fromIndex < 0 || toIndex < 0 || fromIndex >= kInventorySlotCount || toIndex >= kInventorySlotCount || fromIndex == toIndex) {
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
        const int stackLimit = std::max(1, itemMaxStack(dst.itemId));
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
    if(fromIndex < 0 || toIndex < 0 || fromIndex >= kInventorySlotCount || toIndex >= kInventorySlotCount || fromIndex == toIndex) {
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
        moved = std::min(moved, std::max(1, itemMaxStack(src.itemId)));
        dst.itemId = src.itemId;
        dst.count = moved;
        src.count -= moved;
        if(src.count <= 0) {
            src = InventorySlot{};
        }
        return moved > 0;
    }

    const int room = std::max(1, itemMaxStack(dst.itemId)) - dst.count;
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

bool Simulation::canCraft(RecipeId recipe) const {
    const RecipeBase* def = recipeCatalog().find(recipe);
    if(def == nullptr) {
        return false;
    }

    if(def->requiresWorkbench() && !isNearWorkbench()) {
        return false;
    }

    for(const auto& input : def->inputs()) {
        if(!hasItemAmount(input.item, input.count)) {
            return false;
        }
    }

    return def->canCraftWithTools(axeLevel_, pickaxeLevel_);
}

bool Simulation::craft(RecipeId recipe) {
    if(!canCraft(recipe)) {
        return false;
    }

    const RecipeBase* def = recipeCatalog().find(recipe);
    if(def == nullptr) {
        return false;
    }

    const auto inventoryBefore = inventorySlots_;
    const int axeBefore = axeLevel_;
    const int pickaxeBefore = pickaxeLevel_;

    for(const auto& input : def->inputs()) {
        if(!removeItem(input.item, input.count)) {
            inventorySlots_ = inventoryBefore;
            axeLevel_ = axeBefore;
            pickaxeLevel_ = pickaxeBefore;
            return false;
        }
    }

    for(const auto& output : def->outputs()) {
        if(!addItem(output.item, output.count)) {
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
    const int bestToolLevel = std::max(axeLevel_, pickaxeLevel_);
    return kMiningRangeBaseTiles + static_cast<float>(bestToolLevel) * 0.75F;
}

int Simulation::steps() const {
    return steps_;
}

bool Simulation::done() const {
    return done_;
}

bool Simulation::commandSetTile(const Vec2i& target, TileType tile) {
    if(target == player_ && tile != TileType::Empty) {
        return false;
    }

    world_.setTile(target.x, target.y, tile);
    return true;
}

bool Simulation::commandSpawnEntity(const std::string& entityId, const Vec2i& target, int hp, bool aggro, const std::string& variant) {
    if(!world_.isPassable(target.x, target.y)) {
        return false;
    }

    for(const auto& mob : mobs_) {
        if(mob.pos == target) {
            return false;
        }
    }

    mobs_.push_back(Mob{target, std::max(1, hp), entityId, aggro, variant});
    return true;
}

bool Simulation::commandGiveItem(std::string_view itemId, int amount) {
    return addItemByKey(itemId, amount);
}

bool Simulation::commandTeleportPlayer(const Vec2i& target) {
    if(!world_.isPassable(target.x, target.y)) {
        return false;
    }
    player_ = target;
    clearMiningProgress();
    return true;
}

bool Simulation::tryMove(const Vec2i& delta) {
    const Vec2i candidate{player_.x + delta.x, player_.y + delta.y};
    facing_ = delta;

    if(world_.isPassable(candidate.x, candidate.y)) {
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
    if(tryPlaceFromSlotIndex(selectedHotbarSlotIndex())) {
        energy_ = std::max(0, energy_ - 1);
        return;
    }

    if(craft(RecipeId::PickaxeTier2)) {
        return;
    }
    if(craft(RecipeId::AxeTier2)) {
        return;
    }
    if(craft(RecipeId::PickaxeTier1)) {
        return;
    }
    if(craft(RecipeId::AxeTier1)) {
        return;
    }
    if(craft(RecipeId::Workbench)) {
        return;
    }
    if(craft(RecipeId::Sticks)) {
        return;
    }
    (void)craft(RecipeId::Planks);
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

    const int stackLimit = std::max(1, itemMaxStack(normalized));

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
    if(slotIndex < 0 || slotIndex >= kInventorySlotCount) {
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
    std::uniform_int_distribution<int> roll(0, 4);
    for(auto& mob : mobs_) {
        Vec2i next = mob.pos;

        if(mob.aggro) {
            const int dx = player_.x - mob.pos.x;
            const int dy = player_.y - mob.pos.y;
            const int dist = std::abs(dx) + std::abs(dy);
            if(dist <= 9) {
                if(std::abs(dx) >= std::abs(dy)) {
                    next.x += (dx == 0 ? 0 : (dx > 0 ? 1 : -1));
                } else {
                    next.y += (dy == 0 ? 0 : (dy > 0 ? 1 : -1));
                }
            } else {
                switch(roll(rng_)) {
                    case 0:
                        next.y -= 1;
                        break;
                    case 1:
                        next.y += 1;
                        break;
                    case 2:
                        next.x -= 1;
                        break;
                    case 3:
                        next.x += 1;
                        break;
                    case 4:
                        break;
                }
            }
        } else {
            switch(roll(rng_)) {
                case 0:
                    next.y -= 1;
                    break;
                case 1:
                    next.y += 1;
                    break;
                case 2:
                    next.x -= 1;
                    break;
                case 3:
                    next.x += 1;
                    break;
                case 4:
                    break;
            }
        }

        if(world_.isPassable(next.x, next.y)) {
            mob.pos = next;
        }

        if(mob.pos == player_) {
            hp_ -= 1;
        }
    }
}

float Simulation::computeReward(bool reachedExit, int hpBefore, int previousDistance, int currentDistance) const {
    float reward = -0.01F;

    const int damage = std::max(0, hpBefore - hp_);
    reward -= static_cast<float>(damage) * 0.5F;

    const int progress = previousDistance - currentDistance;
    reward += static_cast<float>(progress) * 0.03F;

    if(reachedExit) {
        reward += 50.0F;
    }

    if(done_ && !reachedExit) {
        reward -= 10.0F;
    }

    return reward;
}

}  // namespace stoneforge
