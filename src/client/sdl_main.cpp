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

#include "lecon/simulation.hpp"

namespace {

struct Color {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
};

Color tileColor(lecon::TileType type) {
    switch(type) {
        case lecon::TileType::Empty:
            return {40, 47, 56};
        case lecon::TileType::Wall:
            return {85, 88, 97};
        case lecon::TileType::Resource:
            return {183, 134, 45};
        case lecon::TileType::Exit:
            return {46, 204, 113};
        case lecon::TileType::Tree:
            return {91, 148, 78};
        case lecon::TileType::Workbench:
            return {156, 111, 72};
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
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        return 1;
    }

    constexpr int kWindowW = 1280;
    constexpr int kWindowH = 720;
    constexpr int kTileSize = 24;
    constexpr int kViewRadiusX = 24;
    constexpr int kViewRadiusY = 14;

    SDL_Window* window = SDL_CreateWindow(
        "Lecon 2D",
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

    lecon::Simulation sim;
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
            lecon::Action action = lecon::Action::Wait;

            if(state[SDL_SCANCODE_UP]) {
                action = lecon::Action::MoveUp;
            } else if(state[SDL_SCANCODE_DOWN]) {
                action = lecon::Action::MoveDown;
            } else if(state[SDL_SCANCODE_LEFT]) {
                action = lecon::Action::MoveLeft;
            } else if(state[SDL_SCANCODE_RIGHT]) {
                action = lecon::Action::MoveRight;
            } else if(state[SDL_SCANCODE_Z]) {
                action = lecon::Action::Mine;
            } else if(state[SDL_SCANCODE_X]) {
                action = lecon::Action::Place;
            } else if(state[SDL_SCANCODE_C]) {
                action = lecon::Action::Use;
            }

            if(!sim.done()) {
                sim.step(action);
            }

            std::string title = "Lecon 2D | HP=" + std::to_string(sim.hp()) +
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

        const lecon::Vec2i player = sim.playerPos();
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

        const lecon::Vec2i exit = sim.exitPos();
        fillRect(
            renderer,
            centerX + (exit.x - player.x) * kTileSize,
            centerY + (exit.y - player.y) * kTileSize,
            kTileSize - 1,
            kTileSize - 1,
            {46, 204, 113}
        );

        for(const auto& mob : sim.mobs()) {
            fillRect(
                renderer,
                centerX + (mob.pos.x - player.x) * kTileSize,
                centerY + (mob.pos.y - player.y) * kTileSize,
                kTileSize - 1,
                kTileSize - 1,
                {231, 76, 60}
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
