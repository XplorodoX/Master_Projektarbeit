#include <cstdlib>
#include <string>
#include "stoneforge/client/render_engine.hpp"

int main(int argc, char** argv) {
    bool aiMode = false;
    bool aiDualMode = false;
    std::uint64_t aiSeed = 42;
    int winX = -1;
    int winY = -1;
    int winW = -1;
    int winH = -1;
    std::string winTitle;

    for(int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);
        if(arg == "--ai") {
            aiMode = true;
        } else if(arg == "--ai-dual") {
            aiMode = true;
            aiDualMode = true;
        } else if(arg == "--seed" && i + 1 < argc) {
            aiSeed = std::strtoull(argv[++i], nullptr, 10);
        } else if(arg == "--window-pos" && i + 2 < argc) {
            winX = std::atoi(argv[++i]);
            winY = std::atoi(argv[++i]);
        } else if(arg == "--window-size" && i + 2 < argc) {
            winW = std::atoi(argv[++i]);
            winH = std::atoi(argv[++i]);
        } else if(arg == "--title" && i + 1 < argc) {
            winTitle = argv[++i];
        }
    }
    stoneforge::client::RenderEngine engine;
    return engine.run(aiMode, aiDualMode, aiSeed, winX, winY, winW, winH, winTitle);
}
