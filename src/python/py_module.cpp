#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "stoneforge/game_config.hpp"
#include "stoneforge/simulation.hpp"

namespace py = pybind11;

namespace {

std::filesystem::path resolveGameConfigPath() {
    const std::filesystem::path candidates[] = {
        "assets/base/game_config.json",
        "../assets/base/game_config.json",
        "../../assets/base/game_config.json",
    };

    for(const auto& candidate : candidates) {
        if(std::filesystem::exists(candidate)) {
            return candidate;
        }
    }

    return candidates[0];
}

std::vector<int> flattenObservation(const stoneforge::Observation& obs) {
    std::vector<int> out = obs.grid;
    out.push_back(obs.hp);
    out.push_back(obs.energy);
    out.push_back(obs.inventory);
    out.push_back(obs.exitDx);
    out.push_back(obs.exitDy);
    return out;
}

class StoneforgeCoreEnv {
public:
    explicit StoneforgeCoreEnv(std::uint64_t baseSeed = 42) : baseSeed_(baseSeed) {
        std::string configError;
        (void)stoneforge::loadGameConfigFile(resolveGameConfigPath(), &configError);
        sim_.reset(baseSeed_);
    }

    std::vector<int> reset(std::uint64_t seed) {
        sim_.reset(seed);
        return flattenObservation(sim_.getObservation());
    }

    py::tuple step(int action) {
        if(action < 0 || action >= stoneforge::Simulation::kActionCount) {
            throw std::runtime_error("action out of range");
        }

        const auto result = sim_.step(static_cast<stoneforge::Action>(action));
        const auto obs = flattenObservation(sim_.getObservation());

        py::dict info;
        info["reached_exit"] = result.reachedExit;
        info["step"] = result.step;
        info["bfs_distance"] = sim_.currentBfsDistanceToExit();

        const bool terminated = result.done;
        const bool truncated = false;

        return py::make_tuple(obs, result.reward, terminated, truncated, info);
    }

    int actionSpaceN() const {
        return stoneforge::Simulation::kActionCount;
    }

    int observationSize() const {
        return sim_.observationSize();
    }

    int currentBfsDistanceToExit() const {
        return sim_.currentBfsDistanceToExit();
    }

    bool isPathToExitReachable() const {
        return sim_.isPathToExitReachable();
    }

    // BFS-Distanz an einer relativen Position zum Spieler.
    // Gibt BFS(player + offset) zurück — oder Manhattan-Fallback wenn außerhalb BFS-Box.
    // Hauptnutzen: 4-direktionales Kraftfeld (bfs_at(0,-1), bfs_at(0,1), bfs_at(-1,0), bfs_at(1,0))
    // zeigt dem Agenten direkt welche Richtung den Pfad zum Exit wirklich verkürzt.
    int bfsDistanceAtOffset(int dx, int dy) const {
        const auto p = sim_.playerPos();
        return sim_.bfsDistanceAt(p.x + dx, p.y + dy);
    }

    py::tuple playerPos() const {
        const auto p = sim_.playerPos();
        return py::make_tuple(p.x, p.y);
    }

    int stepsWithoutProgress() const {
        return sim_.stepsWithoutProgress();
    }

    void configureWorldGeneration(int exitMinDistance, int exitMaxDistance, bool forceGuaranteedPath,
                                   bool disableMobs = false, bool disableEnergy = false) {
        auto& cfg = stoneforge::mutableGameConfig();
        cfg.world.exitMinDistance = std::max(1, exitMinDistance);
        cfg.world.exitMaxDistance = std::max(cfg.world.exitMinDistance, exitMaxDistance);
        cfg.world.forceGuaranteedPath = forceGuaranteedPath;
        cfg.gameplay.disableMobs = disableMobs;
        cfg.gameplay.disableEnergy = disableEnergy;
    }

private:
    std::uint64_t baseSeed_ = 42;
    stoneforge::Simulation sim_;
};

}  // namespace

PYBIND11_MODULE(stoneforge_sim, m) {
    m.doc() = "Stoneforge 2D simulation bindings";

    py::class_<StoneforgeCoreEnv>(m, "StoneforgeCoreEnv")
        .def(py::init<std::uint64_t>(), py::arg("base_seed") = 42)
        .def("reset", &StoneforgeCoreEnv::reset, py::arg("seed"))
        .def("step", &StoneforgeCoreEnv::step, py::arg("action"))
        .def("action_space_n", &StoneforgeCoreEnv::actionSpaceN)
        .def("observation_size", &StoneforgeCoreEnv::observationSize)
        .def("current_bfs_distance_to_exit", &StoneforgeCoreEnv::currentBfsDistanceToExit)
        .def("is_path_to_exit_reachable", &StoneforgeCoreEnv::isPathToExitReachable)
        .def("bfs_distance_at_offset", &StoneforgeCoreEnv::bfsDistanceAtOffset,
             py::arg("dx"), py::arg("dy"))
        .def("configure_world_generation", &StoneforgeCoreEnv::configureWorldGeneration,
               py::arg("exit_min_distance"), py::arg("exit_max_distance"),
               py::arg("force_guaranteed_path"), py::arg("disable_mobs") = false,
               py::arg("disable_energy") = false)
        .def("player_pos", &StoneforgeCoreEnv::playerPos)
        .def("steps_without_progress", &StoneforgeCoreEnv::stepsWithoutProgress);
}
