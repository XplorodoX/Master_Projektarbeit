#include <cstdint>
#include <stdexcept>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "stoneforge/game_config.hpp"
#include "stoneforge/simulation.hpp"

namespace py = pybind11;

namespace {

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
        (void)stoneforge::loadGameConfigFile("assets/base/game_config.json", &configError);
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

    py::tuple playerPos() const {
        const auto p = sim_.playerPos();
        return py::make_tuple(p.x, p.y);
    }

    void configureWorldGeneration(int exitMinDistance, int exitMaxDistance, bool forceGuaranteedPath,
                                   bool disableMobs = false) {
        auto& cfg = stoneforge::mutableGameConfig();
        cfg.world.exitMinDistance = std::max(1, exitMinDistance);
        cfg.world.exitMaxDistance = std::max(cfg.world.exitMinDistance, exitMaxDistance);
        cfg.world.forceGuaranteedPath = forceGuaranteedPath;
        cfg.gameplay.disableMobs = disableMobs;
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
        .def("configure_world_generation", &StoneforgeCoreEnv::configureWorldGeneration,
             py::arg("exit_min_distance"), py::arg("exit_max_distance"),
             py::arg("force_guaranteed_path"), py::arg("disable_mobs") = false)
        .def("player_pos", &StoneforgeCoreEnv::playerPos);
}
