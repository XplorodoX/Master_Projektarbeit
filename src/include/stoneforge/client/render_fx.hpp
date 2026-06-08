#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include <raylib.h>

#include "stoneforge/types.hpp"

namespace stoneforge::client {

struct Particle {
    Vector2 worldPos{};
    Vector2 vel{};
    Color color{255, 255, 255, 255};
    float life = 0.0F;
    float maxLife = 1.0F;
    float size = 1.0F;
};

struct CrackInfo {
    float strength = 0.0F;
    float ttl = 0.0F;
};

std::int64_t tileKey(int x, int y);

void spawnParticles(
    std::vector<Particle>& particles,
    Vector2 world,
    int count,
    Color color,
    float speed,
    float life,
    float size,
    int salt
);

void updateParticles(std::vector<Particle>& particles, float dt);

void drawParticles(const std::vector<Particle>& particles, const stoneforge::Vec2i& player, int centerX, int centerY, int tileSize);

void updateCracks(std::unordered_map<std::int64_t, CrackInfo>& cracks, float dt);

void drawCracks(
    const std::unordered_map<std::int64_t, CrackInfo>& cracks,
    const stoneforge::Vec2i& player,
    int centerX,
    int centerY,
    int tileSize,
    int viewRadiusX,
    int viewRadiusY,
    float t
);

}  // namespace stoneforge::client
