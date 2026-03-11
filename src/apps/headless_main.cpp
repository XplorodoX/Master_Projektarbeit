#include <cstdint>
#include <iostream>
#include <random>
#include <string>

#include "stoneforge/simulation.hpp"

namespace {

std::uint64_t parseU64(const char* text, std::uint64_t fallback) {
    try {
        return static_cast<std::uint64_t>(std::stoull(text));
    } catch(...) {
        return fallback;
    }
}

int parseInt(const char* text, int fallback) {
    try {
        return std::stoi(text);
    } catch(...) {
        return fallback;
    }
}

}  // namespace

int main(int argc, char** argv) {
    int episodes = 10;
    int maxSteps = stoneforge::Simulation::kDefaultMaxSteps;
    std::uint64_t seed = 42;

    for(int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if(arg == "--episodes" && i + 1 < argc) {
            episodes = parseInt(argv[++i], episodes);
        } else if(arg == "--max-steps" && i + 1 < argc) {
            maxSteps = parseInt(argv[++i], maxSteps);
        } else if(arg == "--seed" && i + 1 < argc) {
            seed = parseU64(argv[++i], seed);
        }
    }

    std::mt19937 rng(static_cast<unsigned int>(seed));
    std::uniform_int_distribution<int> actionDist(0, stoneforge::Simulation::kActionCount - 1);

    int successCount = 0;
    for(int episode = 0; episode < episodes; ++episode) {
        stoneforge::Simulation sim;
        sim.reset(seed + static_cast<std::uint64_t>(episode));

        float totalReward = 0.0F;
        bool reached = false;

        for(int step = 0; step < maxSteps && !sim.done(); ++step) {
            const auto action = static_cast<stoneforge::Action>(actionDist(rng));
            const auto result = sim.step(action);
            totalReward += result.reward;
            reached = reached || result.reachedExit;
        }

        if(reached) {
            ++successCount;
        }

        std::cout << "episode=" << episode
                  << " reward=" << totalReward
                  << " reached_exit=" << (reached ? 1 : 0)
                  << " hp=" << sim.hp()
                  << " steps=" << sim.steps() << '\n';
    }

    std::cout << "success_rate=" << static_cast<float>(successCount) / static_cast<float>(episodes) << '\n';
    return 0;
}
