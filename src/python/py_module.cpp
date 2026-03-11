#include <cstdint>
#include <stdexcept>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "lecon/simulation.hpp"

namespace py = pybind11;

namespace {

std::vector<int> flattenObservation(const lecon::Observation& obs) {
    std::vector<int> out = obs.grid;
    out.push_back(obs.hp);
    out.push_back(obs.energy);
    out.push_back(obs.inventory);
    return out;
}

class LeconCoreEnv {
public:
    explicit LeconCoreEnv(std::uint64_t baseSeed = 42) : baseSeed_(baseSeed) {
        sim_.reset(baseSeed_);
    }

    std::vector<int> reset(std::uint64_t seed) {
        sim_.reset(seed);
        return flattenObservation(sim_.getObservation());
    }

    py::tuple step(int action) {
        if(action < 0 || action >= lecon::Simulation::kActionCount) {
            throw std::runtime_error("action out of range");
        }

        const auto result = sim_.step(static_cast<lecon::Action>(action));
        const auto obs = flattenObservation(sim_.getObservation());

        py::dict info;
        info["reached_exit"] = result.reachedExit;
        info["step"] = result.step;

        const bool terminated = result.done;
        const bool truncated = false;

        return py::make_tuple(obs, result.reward, terminated, truncated, info);
    }

    int actionSpaceN() const {
        return lecon::Simulation::kActionCount;
    }

    int observationSize() const {
        const int side = 2 * lecon::Simulation::kObservationRadius + 1;
        return side * side + 3;
    }

private:
    std::uint64_t baseSeed_ = 42;
    lecon::Simulation sim_;
};

}  // namespace

PYBIND11_MODULE(lecon_sim, m) {
    m.doc() = "Lecon 2D simulation bindings";

    py::class_<LeconCoreEnv>(m, "LeconCoreEnv")
        .def(py::init<std::uint64_t>(), py::arg("base_seed") = 42)
        .def("reset", &LeconCoreEnv::reset, py::arg("seed"))
        .def("step", &LeconCoreEnv::step, py::arg("action"))
        .def("action_space_n", &LeconCoreEnv::actionSpaceN)
        .def("observation_size", &LeconCoreEnv::observationSize);
}
