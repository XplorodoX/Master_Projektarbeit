#include "lecon/simulation.hpp"

#include <algorithm>
#include <cmath>

namespace lecon {

namespace {

int manhattanDistance(const Vec2i& a, const Vec2i& b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
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
    inventory_ = 0;
    wood_ = 0;
    ore_ = 0;
    axeLevel_ = 0;
    pickaxeLevel_ = 0;
    clearMiningProgress();

    steps_ = 0;
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

    energy_ = std::max(0, energy_ - 1);
    updateMobs();

    if(energy_ <= 0) {
        hp_ -= 1;
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
    out.inventory = inventory_;

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
    return inventory_;
}

int Simulation::wood() const {
    return wood_;
}

int Simulation::ore() const {
    return ore_;
}

int Simulation::axeLevel() const {
    return axeLevel_;
}

int Simulation::pickaxeLevel() const {
    return pickaxeLevel_;
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
    const Vec2i target{player_.x + facing_.x, player_.y + facing_.y};
    const TileType tile = world_.tileAt(target.x, target.y);

    if(tile != TileType::Resource && tile != TileType::Tree) {
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
        world_.setTile(target.x, target.y, TileType::Empty);
        ore_ += 1;
        inventory_ = wood_ + ore_;
    } else if(tile == TileType::Tree) {
        world_.setTile(target.x, target.y, TileType::Empty);
        wood_ += 1;
        inventory_ = wood_ + ore_;
    }

    clearMiningProgress();
}

void Simulation::placeForward() {
    if(inventory_ <= 0) {
        return;
    }

    const Vec2i target{player_.x + facing_.x, player_.y + facing_.y};
    const TileType tile = world_.tileAt(target.x, target.y);

    if(tile == TileType::Empty) {
        world_.setTile(target.x, target.y, TileType::Wall);
        if(wood_ > 0) {
            wood_ -= 1;
        } else if(ore_ > 0) {
            ore_ -= 1;
        }
        inventory_ = std::max(0, wood_ + ore_);
        energy_ = std::max(0, energy_ - 1);
    }
}

void Simulation::useAction() {
    // Craft tools from inventory; better tools speed up mining.
    if(axeLevel_ < 1 && wood_ >= 4) {
        wood_ -= 4;
        axeLevel_ = 1;
    } else if(pickaxeLevel_ < 1 && wood_ >= 5) {
        wood_ -= 5;
        pickaxeLevel_ = 1;
    } else if(pickaxeLevel_ < 2 && wood_ >= 2 && ore_ >= 6) {
        wood_ -= 2;
        ore_ -= 6;
        pickaxeLevel_ = 2;
    } else if(axeLevel_ < 2 && wood_ >= 2 && ore_ >= 4) {
        wood_ -= 2;
        ore_ -= 4;
        axeLevel_ = 2;
    }

    inventory_ = wood_ + ore_;
    energy_ = std::max(0, energy_ - 1);
}

void Simulation::clearMiningProgress() {
    miningActive_ = false;
    miningTarget_ = {0, 0};
    miningTile_ = TileType::Empty;
    miningProgress_ = 0.0F;
}

float Simulation::miningHardness(TileType tile) const {
    switch(tile) {
        case TileType::Tree:
            return 2.2F;
        case TileType::Resource:
            return 6.0F;
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

    return 0.1F;
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
