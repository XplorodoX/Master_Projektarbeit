#include "stoneforge/client/render_fx.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace stoneforge::client {

namespace {

std::uint32_t hashU32(int x, int y, std::uint32_t salt) {
    std::uint32_t value = static_cast<std::uint32_t>(x) * 374761393u;
    value ^= static_cast<std::uint32_t>(y) * 668265263u;
    value ^= salt * 2246822519u;
    value = (value ^ (value >> 13)) * 1274126177u;
    return value ^ (value >> 16);
}

float hash01(int x, int y, std::uint32_t salt) {
    const std::uint32_t h = hashU32(x, y, salt);
    return static_cast<float>(h & 0x00FFFFFFu) / 16777215.0F;
}

}  // namespace

std::int64_t tileKey(int x, int y) {
    const auto ux = static_cast<std::uint64_t>(static_cast<std::uint32_t>(x));
    const auto uy = static_cast<std::uint64_t>(static_cast<std::uint32_t>(y));
    return static_cast<std::int64_t>((ux << 32) | uy);
}

void spawnParticles(
    std::vector<Particle>& particles,
    Vector2 world,
    int count,
    Color color,
    float speed,
    float life,
    float size,
    int salt
) {
    for(int i = 0; i < count; ++i) {
        const float ang = hash01(i, salt, 1701) * 6.2831853F;
        const float mag = speed * (0.35F + hash01(salt, i, 1703) * 0.9F);
        Particle p;
        p.worldPos = world;
        p.vel = Vector2{std::cos(ang) * mag, std::sin(ang) * mag};
        p.color = color;
        p.maxLife = life * (0.8F + hash01(i, salt, 1709) * 0.45F);
        p.life = p.maxLife;
        p.size = size * (0.75F + hash01(salt, i, 1717) * 0.5F);
        particles.push_back(p);
    }
}

void updateParticles(std::vector<Particle>& particles, float dt) {
    for(auto& p : particles) {
        p.life -= dt;
        p.worldPos.x += p.vel.x * dt;
        p.worldPos.y += p.vel.y * dt;
        p.vel.y += 0.8F * dt;
        p.vel.x *= 0.98F;
    }

    particles.erase(
        std::remove_if(
            particles.begin(),
            particles.end(),
            [](const Particle& p) { return p.life <= 0.0F; }
        ),
        particles.end()
    );
}

void drawParticles(const std::vector<Particle>& particles, const stoneforge::Vec2i& player, int centerX, int centerY, int tileSize) {
    for(const auto& p : particles) {
        const float lifeNorm = std::clamp(p.life / std::max(0.001F, p.maxLife), 0.0F, 1.0F);
        const int sx = centerX + static_cast<int>((p.worldPos.x - static_cast<float>(player.x)) * static_cast<float>(tileSize));
        const int sy = centerY + static_cast<int>((p.worldPos.y - static_cast<float>(player.y)) * static_cast<float>(tileSize));
        const float r = std::max(1.0F, p.size * (static_cast<float>(tileSize) * 0.12F) * lifeNorm);
        DrawCircle(sx, sy, r, Fade(p.color, 0.1F + lifeNorm * 0.9F));
    }
}

void updateCracks(std::unordered_map<std::int64_t, CrackInfo>& cracks, float dt) {
    for(auto it = cracks.begin(); it != cracks.end();) {
        it->second.ttl -= dt;
        it->second.strength = std::max(0.0F, it->second.strength - dt * 0.25F);
        if(it->second.ttl <= 0.0F || it->second.strength <= 0.01F) {
            it = cracks.erase(it);
        } else {
            ++it;
        }
    }
}

void drawCracks(
    const std::unordered_map<std::int64_t, CrackInfo>& cracks,
    const stoneforge::Vec2i& player,
    int centerX,
    int centerY,
    int tileSize,
    int viewRadiusX,
    int viewRadiusY,
    float t
) {
    for(const auto& [key, info] : cracks) {
        const int wx = static_cast<int>(static_cast<std::uint32_t>((static_cast<std::uint64_t>(key) >> 32) & 0xFFFFFFFFu));
        const int wy = static_cast<int>(static_cast<std::uint32_t>(static_cast<std::uint64_t>(key) & 0xFFFFFFFFu));

        const int dx = wx - player.x;
        const int dy = wy - player.y;
        if(dx < -viewRadiusX - 1 || dx > viewRadiusX + 1 || dy < -viewRadiusY - 1 || dy > viewRadiusY + 1) {
            continue;
        }

        const int px = centerX + dx * tileSize;
        const int py = centerY + dy * tileSize;

        const float alpha = std::clamp(info.strength, 0.0F, 1.0F);
        const Color c = Fade(Color{33, 26, 26, 255}, 0.22F + alpha * 0.65F);
        const int jitter = static_cast<int>(std::sin(t * 9.0F + static_cast<float>(wx + wy)) * 1.2F);
        const int cx = px + tileSize / 2;
        const int cy = py + tileSize / 2;

        // Keep the damage cue local and non-linear so it doesn't read as a world-spanning line artifact.
        DrawCircle(cx - tileSize / 6, cy - tileSize / 7 + jitter, std::max(1, tileSize / 16), c);
        DrawCircle(cx + tileSize / 8, cy - tileSize / 10 - jitter, std::max(1, tileSize / 18), Fade(c, 0.95F));
        DrawCircle(cx - tileSize / 12, cy + tileSize / 8, std::max(1, tileSize / 18), Fade(c, 0.75F));
        DrawCircle(cx + tileSize / 10, cy + tileSize / 9, std::max(1, tileSize / 20), Fade(c, 0.6F));
    }
}

}  // namespace stoneforge::client
