#pragma once

#include <array>
#include <cstdint>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "stoneforge/object.hpp"
#include "stoneforge/types.hpp"
#include "stoneforge/world.hpp"

namespace stoneforge {

struct Observation {
    std::vector<int> grid;
    int hp = 0;
    int energy = 0;
    int inventory = 0;
};

struct Mob {
    Vec2i pos{};
    int hp = 1;
    std::string entityId = "stoneforge:mob";
    bool aggro = false;
    std::string variant = "default";
};

enum class ItemId : int {
    None = 0,
    Wood = 1,
    Planks = 2,
    Sticks = 3,
    Ore = 4,
    WorkbenchKit = 5
};

struct InventorySlot {
    std::string itemId;
    int count = 0;
};

class Simulation {
public:
    static constexpr int kObservationRadius = 5;
    static constexpr int kActionCount = 9;
    static constexpr int kDefaultMaxSteps = 2500;
    static constexpr int kInventorySlotCount = 24;
    static constexpr int kInventoryStackLimit = 64;
    static constexpr int kHotbarSlotCount = 9;

    Simulation();

    void reset(std::uint64_t seed);
    StepResult step(Action action);
    void setMiningTargetOverride(const Vec2i& target);
    void clearMiningTargetOverride();

    Observation getObservation() const;

    Vec2i playerPos() const;
    Vec2i exitPos() const;
    TileType tileAt(int x, int y) const;
    const std::vector<Mob>& mobs() const;

    int hp() const;
    int energy() const;
    int inventory() const;
    int wood() const;
    int planks() const;
    int sticks() const;
    int ore() const;
    int workbenches() const;
    int axeLevel() const;
    int pickaxeLevel() const;
    bool isNearWorkbench() const;
    int hotbarSelection() const;
    void setHotbarSelection(int hotbarIndex);
    int selectedHotbarSlotIndex() const;
    InventorySlot hotbarSlot(int hotbarIndex) const;
    int inventorySlotCount() const;
    InventorySlot inventorySlot(int index) const;
    bool moveInventoryStack(int fromIndex, int toIndex);
    bool splitInventoryStack(int fromIndex, int toIndex);
    bool canCraft(std::string_view recipeId) const;
    bool craft(std::string_view recipeId);
    bool placeWorkbenchForward();
    bool placeFromHotbarAt(const Vec2i& target);
    bool contextUseAt(const Vec2i& target);
    bool isMining() const;
    Vec2i miningTarget() const;
    TileType miningTile() const;
    float miningProgress01() const;
    bool hasLineOfSightTo(const Vec2i& target) const;
    bool canMineTarget(const Vec2i& target) const;
    bool canPlaceFromHotbarAt(const Vec2i& target) const;
    TileType previewPlacementTileForSelectedHotbar() const;
    float miningRangeTiles() const;
    int steps() const;
    bool done() const;

    bool commandSetTile(const Vec2i& target, TileType tile);
    bool commandSpawnEntity(const std::string& entityId, const Vec2i& target, int hp = 1, bool aggro = false, const std::string& variant = "default");
    bool commandGiveItem(std::string_view itemId, int amount);
    bool commandTeleportPlayer(const Vec2i& target);

private:
    bool tryMove(const Vec2i& delta);
    Vec2i actionDelta(Action action) const;
    void mineForward();
    void placeForward();
    void useAction();
    void updateMobs();
    void clearMiningProgress();
    int itemCount(ItemId item) const;
    bool addItem(ItemId item, int amount);
    bool addDropItem(ObjectDrop drop);
    bool removeItem(ItemId item, int amount);
    bool hasItemAmount(ItemId item, int amount) const;
    int itemCountByKey(std::string_view itemId) const;
    bool addItemByKey(std::string_view itemId, int amount);
    bool removeItemByKey(std::string_view itemId, int amount);
    bool hasItemAmountByKey(std::string_view itemId, int amount) const;
    bool tryPlaceFromSlotIndex(int slotIndex);
    bool tryPlaceFromSlotIndexAt(int slotIndex, const Vec2i& target);
    float miningHardness(TileType tile) const;
    float miningSpeed(TileType tile) const;
    bool isMineableTile(TileType tile) const;
    bool isWithinMiningRange(const Vec2i& target) const;

    float computeReward(bool reachedExit, int hpBefore, int previousDistance, int currentDistance) const;

    std::mt19937_64 rng_;
    World world_;

    Vec2i player_{};
    Vec2i facing_{};

    int hp_ = 10;
    int energy_ = 100;
    std::array<InventorySlot, kInventorySlotCount> inventorySlots_{};
    int hotbarSelection_ = 0;
    int axeLevel_ = 0;
    int pickaxeLevel_ = 0;

    bool miningActive_ = false;
    Vec2i miningTarget_{};
    TileType miningTile_ = TileType::Empty;
    float miningProgress_ = 0.0F;
    bool miningTargetOverrideActive_ = false;
    Vec2i miningTargetOverride_{};

    int steps_ = 0;
    int maxSteps_ = kDefaultMaxSteps;
    int starvationTicks_ = 0;

    bool done_ = false;
    bool reachedExit_ = false;

    std::vector<Mob> mobs_;
};

}  // namespace stoneforge
