#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#elif __has_include(<SDL.h>)
#include <SDL.h>
#else
#error "SDL2 headers not found"
#endif

#include "stoneforge/game_config.hpp"
#include "stoneforge/simulation.hpp"

namespace {

struct Color {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
};

Color tileColor(stoneforge::TileType type) {
    switch(type) {
        case stoneforge::TileType::Empty:
            return {40, 47, 56};
        case stoneforge::TileType::Wall:
            return {85, 88, 97};
        case stoneforge::TileType::Resource:
            return {183, 134, 45};
        case stoneforge::TileType::Exit:
            return {46, 204, 113};
        case stoneforge::TileType::Tree:
            return {91, 148, 78};
        case stoneforge::TileType::Workbench:
            return {156, 111, 72};
        case stoneforge::TileType::WoodWall:
            return {164, 118, 79};
        case stoneforge::TileType::WoodLog:
            return {136, 97, 66};
    }
    return {255, 0, 0};
}

void fillRect(SDL_Renderer* renderer, int x, int y, int w, int h, Color color) {
    SDL_Rect rect{x, y, w, h};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &rect);
}

}  // namespace

int main(int, char**) {
    std::string gameConfigError;
    (void)stoneforge::loadGameConfigFile("assets/base/game_config.json", &gameConfigError);

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        return 1;
    }

    constexpr int kWindowW = 1280;
    constexpr int kWindowH = 720;
    constexpr int kTileSize = 24;
    constexpr int kViewRadiusX = 24;
    constexpr int kViewRadiusY = 14;

    SDL_Window* window = SDL_CreateWindow(
        "Stoneforge 2D",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        kWindowW,
        kWindowH,
        SDL_WINDOW_SHOWN
    );

    if(!window) {
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    stoneforge::Simulation sim;
    sim.reset(42);

    bool running = true;
    std::uint32_t lastStepTicks = SDL_GetTicks();

    while(running) {
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) {
                running = false;
            }
            if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_r) {
                sim.reset(42);
            }
        }

        const std::uint32_t now = SDL_GetTicks();
        if(now - lastStepTicks >= 120) {
            lastStepTicks = now;

            const Uint8* state = SDL_GetKeyboardState(nullptr);
            stoneforge::Action action = stoneforge::Action::Wait;

            if(state[SDL_SCANCODE_UP]) {
                action = stoneforge::Action::MoveUp;
            } else if(state[SDL_SCANCODE_DOWN]) {
                action = stoneforge::Action::MoveDown;
            } else if(state[SDL_SCANCODE_LEFT]) {
                action = stoneforge::Action::MoveLeft;
            } else if(state[SDL_SCANCODE_RIGHT]) {
                action = stoneforge::Action::MoveRight;
            } else if(state[SDL_SCANCODE_Z]) {
                action = stoneforge::Action::Mine;
            } else if(state[SDL_SCANCODE_X]) {
                action = stoneforge::Action::Place;
            } else if(state[SDL_SCANCODE_C]) {
                action = stoneforge::Action::Use;
            }

            if(!sim.done()) {
                sim.step(action);
            }

            std::string title = "Stoneforge 2D | HP=" + std::to_string(sim.hp()) +
                                " Energy=" + std::to_string(sim.energy()) +
                                " Inv=" + std::to_string(sim.inventory()) +
                                " Steps=" + std::to_string(sim.steps());
            if(sim.done()) {
                title += " | Episode done (R to reset)";
            }
            SDL_SetWindowTitle(window, title.c_str());
        }

        SDL_SetRenderDrawColor(renderer, 20, 22, 28, 255);
        SDL_RenderClear(renderer);

        const stoneforge::Vec2i player = sim.playerPos();
        const int centerX = kWindowW / 2;
        const int centerY = kWindowH / 2;

        for(int vy = -kViewRadiusY; vy <= kViewRadiusY; ++vy) {
            for(int vx = -kViewRadiusX; vx <= kViewRadiusX; ++vx) {
                const int wx = player.x + vx;
                const int wy = player.y + vy;

                const int px = centerX + vx * kTileSize;
                const int py = centerY + vy * kTileSize;

                fillRect(renderer, px, py, kTileSize - 1, kTileSize - 1, tileColor(sim.tileAt(wx, wy)));
            }
        }

        const stoneforge::Vec2i exit = sim.exitPos();
        fillRect(
            renderer,
            centerX + (exit.x - player.x) * kTileSize,
            centerY + (exit.y - player.y) * kTileSize,
            kTileSize - 1,
            kTileSize - 1,
            {46, 204, 113}
        );

        for(const auto& mob : sim.mobs()) {
            SDL_Color mobColor{231, 76, 60, 255};
            switch(mob.biomeTag) {
                case 0:
                    mobColor = SDL_Color{170, 208, 120, 255};
                    break;
                case 1:
                    mobColor = SDL_Color{106, 167, 108, 255};
                    break;
                case 2:
                    mobColor = SDL_Color{212, 176, 112, 255};
                    break;
                case 3:
                    mobColor = SDL_Color{150, 154, 166, 255};
                    break;
                case 4:
                    mobColor = SDL_Color{166, 179, 118, 255};
                    break;
                case 5:
                    mobColor = SDL_Color{164, 204, 226, 255};
                    break;
                case 6:
                    mobColor = SDL_Color{199, 103, 89, 255};
                    break;
                default:
                    break;
            }
            fillRect(
                renderer,
                centerX + (mob.pos.x - player.x) * kTileSize,
                centerY + (mob.pos.y - player.y) * kTileSize,
                kTileSize - 1,
                kTileSize - 1,
                mobColor
            );
        }

        fillRect(renderer, centerX, centerY, kTileSize - 1, kTileSize - 1, {52, 152, 219});

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
