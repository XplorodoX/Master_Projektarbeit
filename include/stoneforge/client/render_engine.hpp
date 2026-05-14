#pragma once

#include <cstdint>

namespace stoneforge::client {

class RenderEngine {
public:
    int run(bool aiMode = false, bool aiDualMode = false, std::uint64_t aiSeed = 42,
            int winX = -1, int winY = -1, int winW = -1, int winH = -1,
            const std::string& winTitle = "", bool noMonsters = false);
};

}  // namespace stoneforge::client
