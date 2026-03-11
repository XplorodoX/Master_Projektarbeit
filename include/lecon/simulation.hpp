#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "lecon/types.hpp"
#include "lecon/world.hpp"

namespace lecon {

struct Observation {
    std::vector<int> grid;
    int hp = 0;
    int energy = 0;
    int inventory = 0;
};

struct Mob {
    Vec2i pos{};
    int hp = 1;
};

class Simulation {
public:
    static constexpr int kObservationRadius = 5;
    static constexpr int kActionCount = 9;
    static constexpr int kDefaultMaxSteps = 2500;

    Simulation();

    void reset(std::uint64_t seed);
    StepResult step(Action action);

    Observation getObservation() const;

    Vec2i playerPos() const;
    Vec2i exitPos() const;
    TileType tileAt(int x, int y) const;
    const std::vector<Mob>& mobs() const;

    int hp() const;
    int energy() const;
    int inventory() const;
    int wood() const;
    int ore() const;
    int axeLevel() const;
    int pickaxeLevel() const;
    bool isMining() const;
    Vec2i miningTarget() const;
    TileType miningTile() const;
    float miningProgress01() const;
    int steps() const;
    bool done() const;

private:
    bool tryMove(const Vec2i& delta);
    Vec2i actionDelta(Action action) const;
    void mineForward();
    void placeForward();
    void useAction();
    void updateMobs();
    void clearMiningProgress();
    float miningHardness(TileType tile) const;
    float miningSpeed(TileType tile) const;

    float computeReward(bool reachedExit, int hpBefore, int previousDistance, int currentDistance) const;

    std::mt19937_64 rng_;
    World world_;

    Vec2i player_{};
    Vec2i facing_{};

    int hp_ = 10;
    int energy_ = 100;
    int inventory_ = 0;
    int wood_ = 0;
    int ore_ = 0;
    int axeLevel_ = 0;
    int pickaxeLevel_ = 0;

    bool miningActive_ = false;
    Vec2i miningTarget_{};
    TileType miningTile_ = TileType::Empty;
    float miningProgress_ = 0.0F;

    int steps_ = 0;
    int maxSteps_ = kDefaultMaxSteps;

    bool done_ = false;
    bool reachedExit_ = false;

    std::vector<Mob> mobs_;
};

}  // namespace lecon
