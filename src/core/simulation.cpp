#include "lecon/simulation.hpp"

#include <algorithm>
#include <cmath>

namespace lecon {

namespace {

int manhattanDistance(const Vec2i& a, const Vec2i& b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

constexpr float kMiningRangeBaseTiles = 4.5F;

TileType placementTileForItem(ItemId item) {
    switch(item) {
        case ItemId::WorkbenchKit:
            return TileType::Workbench;
        case ItemId::Planks:
            return TileType::WoodWall;
        case ItemId::Wood:
            return TileType::WoodLog;
        case ItemId::Ore:
            return TileType::Wall;
        default:
            return TileType::Empty;
    }
}

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
            mobs_.push_back(Mob{p, 1});
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
    return itemCount(ItemId::Wood);
}

int Simulation::planks() const {
    return itemCount(ItemId::Planks);
}

int Simulation::sticks() const {
    return itemCount(ItemId::Sticks);
}

int Simulation::ore() const {
    return itemCount(ItemId::Ore);
}

int Simulation::workbenches() const {
    return itemCount(ItemId::WorkbenchKit);
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
    if(src.item == ItemId::None || src.count <= 0) {
        return false;
    }

    if(dst.item == ItemId::None || dst.count <= 0) {
        dst = src;
        src = InventorySlot{};
        return true;
    }

    if(dst.item == src.item) {
        const int room = kInventoryStackLimit - dst.count;
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
    if(src.item == ItemId::None || src.count <= 1) {
        return false;
    }

    if(dst.item != ItemId::None && dst.item != src.item) {
        return false;
    }

    int moved = (src.count + 1) / 2;
    if(dst.item == ItemId::None) {
        moved = std::min(moved, kInventoryStackLimit);
        dst.item = src.item;
        dst.count = moved;
        src.count -= moved;
        if(src.count <= 0) {
            src = InventorySlot{};
        }
        return moved > 0;
    }

    const int room = kInventoryStackLimit - dst.count;
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
    switch(recipe) {
        case RecipeId::Planks:
            return hasItemAmount(ItemId::Wood, 1);
        case RecipeId::Sticks:
            return hasItemAmount(ItemId::Planks, 2);
        case RecipeId::Workbench:
            return hasItemAmount(ItemId::Planks, 10);
        case RecipeId::AxeTier1:
            return axeLevel_ < 1 && hasItemAmount(ItemId::Planks, 3) && hasItemAmount(ItemId::Sticks, 2) && isNearWorkbench();
        case RecipeId::PickaxeTier1:
            return pickaxeLevel_ < 1 && hasItemAmount(ItemId::Planks, 3) && hasItemAmount(ItemId::Sticks, 2) && isNearWorkbench();
        case RecipeId::AxeTier2:
            return axeLevel_ < 2 && axeLevel_ >= 1 && hasItemAmount(ItemId::Ore, 3) && hasItemAmount(ItemId::Sticks, 2) && isNearWorkbench();
        case RecipeId::PickaxeTier2:
            return pickaxeLevel_ < 2 && pickaxeLevel_ >= 1 && hasItemAmount(ItemId::Ore, 3) && hasItemAmount(ItemId::Sticks, 2) && isNearWorkbench();
        default:
            return false;
    }
}

bool Simulation::craft(RecipeId recipe) {
    if(!canCraft(recipe)) {
        return false;
    }

    const auto inventoryBefore = inventorySlots_;
    const int axeBefore = axeLevel_;
    const int pickaxeBefore = pickaxeLevel_;

    switch(recipe) {
        case RecipeId::Planks:
            if(!removeItem(ItemId::Wood, 1) || !addItem(ItemId::Planks, 4)) {
                inventorySlots_ = inventoryBefore;
                axeLevel_ = axeBefore;
                pickaxeLevel_ = pickaxeBefore;
                return false;
            }
            break;
        case RecipeId::Sticks:
            if(!removeItem(ItemId::Planks, 2) || !addItem(ItemId::Sticks, 4)) {
                inventorySlots_ = inventoryBefore;
                axeLevel_ = axeBefore;
                pickaxeLevel_ = pickaxeBefore;
                return false;
            }
            break;
        case RecipeId::Workbench:
            if(!removeItem(ItemId::Planks, 10) || !addItem(ItemId::WorkbenchKit, 1)) {
                inventorySlots_ = inventoryBefore;
                axeLevel_ = axeBefore;
                pickaxeLevel_ = pickaxeBefore;
                return false;
            }
            break;
        case RecipeId::AxeTier1:
            if(!removeItem(ItemId::Planks, 3) || !removeItem(ItemId::Sticks, 2)) {
                inventorySlots_ = inventoryBefore;
                axeLevel_ = axeBefore;
                pickaxeLevel_ = pickaxeBefore;
                return false;
            }
            axeLevel_ = 1;
            break;
        case RecipeId::PickaxeTier1:
            if(!removeItem(ItemId::Planks, 3) || !removeItem(ItemId::Sticks, 2)) {
                inventorySlots_ = inventoryBefore;
                axeLevel_ = axeBefore;
                pickaxeLevel_ = pickaxeBefore;
                return false;
            }
            pickaxeLevel_ = 1;
            break;
        case RecipeId::AxeTier2:
            if(!removeItem(ItemId::Ore, 3) || !removeItem(ItemId::Sticks, 2)) {
                inventorySlots_ = inventoryBefore;
                axeLevel_ = axeBefore;
                pickaxeLevel_ = pickaxeBefore;
                return false;
            }
            axeLevel_ = 2;
            break;
        case RecipeId::PickaxeTier2:
            if(!removeItem(ItemId::Ore, 3) || !removeItem(ItemId::Sticks, 2)) {
                inventorySlots_ = inventoryBefore;
                axeLevel_ = axeBefore;
                pickaxeLevel_ = pickaxeBefore;
                return false;
            }
            pickaxeLevel_ = 2;
            break;
    }

    energy_ = std::max(0, energy_ - 1);
    return true;
}

bool Simulation::placeWorkbenchForward() {
    if(!hasItemAmount(ItemId::WorkbenchKit, 1)) {
        return false;
    }

    const Vec2i target{player_.x + facing_.x, player_.y + facing_.y};
    if(world_.tileAt(target.x, target.y) != TileType::Empty) {
        return false;
    }

    world_.setTile(target.x, target.y, TileType::Workbench);
    if(!removeItem(ItemId::WorkbenchKit, 1)) {
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

        // Half cover rule: solid built walls block mining LOS.
        const TileType losTile = world_.tileAt(x0, y0);
        if(losTile == TileType::Wall || losTile == TileType::WoodWall || losTile == TileType::WoodLog) {
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
    if(slot.item == ItemId::None || slot.count <= 0) {
        return false;
    }

    return placementTileForItem(slot.item) != TileType::Empty;
}

TileType Simulation::previewPlacementTileForSelectedHotbar() const {
    const auto slot = hotbarSlot(hotbarSelection_);
    if(slot.item == ItemId::None || slot.count <= 0) {
        return TileType::Empty;
    }
    return placementTileForItem(slot.item);
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

    if(tile == TileType::Resource) {
        if(!addItem(ItemId::Ore, 1)) {
            clearMiningProgress();
            return;
        }
        world_.setTile(target.x, target.y, TileType::Empty);
    } else if(tile == TileType::Tree) {
        if(!addItem(ItemId::Wood, 1)) {
            clearMiningProgress();
            return;
        }
        world_.setTile(target.x, target.y, TileType::Empty);
    } else if(tile == TileType::Workbench) {
        if(!addItem(ItemId::WorkbenchKit, 1)) {
            clearMiningProgress();
            return;
        }
        world_.setTile(target.x, target.y, TileType::Empty);
    }

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
    int total = 0;
    for(const auto& slot : inventorySlots_) {
        if(slot.item == item) {
            total += std::max(0, slot.count);
        }
    }
    return total;
}

bool Simulation::addItem(ItemId item, int amount) {
    if(item == ItemId::None || amount <= 0) {
        return false;
    }

    int totalRoom = 0;
    for(const auto& slot : inventorySlots_) {
        if(slot.item == item) {
            totalRoom += std::max(0, kInventoryStackLimit - slot.count);
        } else if(slot.item == ItemId::None || slot.count <= 0) {
            totalRoom += kInventoryStackLimit;
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
        if(slot.item != item) {
            continue;
        }

        const int room = std::max(0, kInventoryStackLimit - slot.count);
        const int moved = std::min(room, left);
        slot.count += moved;
        left -= moved;
    }

    for(auto& slot : inventorySlots_) {
        if(left <= 0) {
            break;
        }
        if(slot.item != ItemId::None && slot.count > 0) {
            continue;
        }

        const int moved = std::min(kInventoryStackLimit, left);
        slot.item = item;
        slot.count = moved;
        left -= moved;
    }

    return left == 0;
}

bool Simulation::removeItem(ItemId item, int amount) {
    if(item == ItemId::None || amount <= 0) {
        return false;
    }
    if(!hasItemAmount(item, amount)) {
        return false;
    }

    int left = amount;
    for(auto& slot : inventorySlots_) {
        if(left <= 0) {
            break;
        }
        if(slot.item != item || slot.count <= 0) {
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
    return itemCount(item) >= amount;
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
    if(slot.item == ItemId::None || slot.count <= 0) {
        return false;
    }

    const TileType placeTile = placementTileForItem(slot.item);
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
    switch(tile) {
        case TileType::Tree:
            return 2.2F;
        case TileType::Resource:
            return 6.0F;
        case TileType::Workbench:
            return 3.6F;
        default:
            return 1.0F;
    }
}

float Simulation::miningSpeed(TileType tile) const {
    if(tile == TileType::Tree) {
        if(axeLevel_ >= 2) {
            return 0.78F;
        }
        if(axeLevel_ >= 1) {
            return 0.45F;
        }
        return 0.22F;
    }

    if(tile == TileType::Resource) {
        if(pickaxeLevel_ >= 2) {
            return 0.45F;
        }
        if(pickaxeLevel_ >= 1) {
            return 0.22F;
        }
        return 0.08F;
    }

    if(tile == TileType::Workbench) {
        if(axeLevel_ >= 2) {
            return 0.65F;
        }
        if(axeLevel_ >= 1) {
            return 0.38F;
        }
        return 0.16F;
    }

    return 0.1F;
}

bool Simulation::isMineableTile(TileType tile) const {
    return tile == TileType::Resource || tile == TileType::Tree || tile == TileType::Workbench;
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

}  // namespace lecon
