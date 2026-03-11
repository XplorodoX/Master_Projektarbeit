#include "stoneforge/world.hpp"

#include <algorithm>

#include "stoneforge/game_config.hpp"
#include "stoneforge/object.hpp"

namespace stoneforge {

World::World(std::uint64_t seed) {
    reset(seed);
}

void World::reset(std::uint64_t seed) {
    seed_ = seed;
    chunks_.clear();

    const auto& cfg = gameConfig().world;
    spawn_ = cfg.spawn;
    exit_ = cfg.exit;

    carveGuaranteedPath();
}

TileType World::tileAt(int x, int y) const {
    const int cx = floorDiv(x, kChunkSize);
    const int cy = floorDiv(y, kChunkSize);
    const int lx = positiveMod(x, kChunkSize);
    const int ly = positiveMod(y, kChunkSize);

    Chunk& chunk = ensureChunk(cx, cy);
    return chunk.tiles[ly * kChunkSize + lx];
}

void World::setTile(int x, int y, TileType tile) {
    const int cx = floorDiv(x, kChunkSize);
    const int cy = floorDiv(y, kChunkSize);
    const int lx = positiveMod(x, kChunkSize);
    const int ly = positiveMod(y, kChunkSize);

    Chunk& chunk = ensureChunk(cx, cy);
    chunk.tiles[ly * kChunkSize + lx] = tile;
}

bool World::isPassable(int x, int y) const {
    const TileType tile = tileAt(x, y);
    return objectForTile(tile).isPassable();
}

Vec2i World::spawnPoint() const {
    return spawn_;
}

Vec2i World::exitPoint() const {
    return exit_;
}

int World::floorDiv(int value, int divisor) {
    const int q = value / divisor;
    const int r = value % divisor;
    return (r != 0 && ((r > 0) != (divisor > 0))) ? q - 1 : q;
}

int World::positiveMod(int value, int divisor) {
    int result = value % divisor;
    if(result < 0) {
        result += divisor;
    }
    return result;
}

std::int64_t World::chunkKey(int cx, int cy) {
    const std::uint64_t hi = static_cast<std::uint64_t>(static_cast<std::uint32_t>(cx));
    const std::uint64_t lo = static_cast<std::uint64_t>(static_cast<std::uint32_t>(cy));
    return static_cast<std::int64_t>((hi << 32) | lo);
}

std::uint64_t World::mix(std::uint64_t value) const {
    value ^= value >> 30;
    value *= 0xbf58476d1ce4e5b9ULL;
    value ^= value >> 27;
    value *= 0x94d049bb133111ebULL;
    value ^= value >> 31;
    return value;
}

double World::noise01(int worldX, int worldY, std::uint64_t salt) const {
    std::uint64_t value = static_cast<std::uint64_t>(worldX) * 0x9e3779b185ebca87ULL;
    value ^= static_cast<std::uint64_t>(worldY) * 0xc2b2ae3d27d4eb4fULL;
    value ^= seed_ * 0x165667b19e3779f9ULL;
    value ^= salt;
    value = mix(value);

    constexpr double kScale = 1.0 / static_cast<double>(0xFFFFFFFFFFFFULL);
    return static_cast<double>(value & 0xFFFFFFFFFFFFULL) * kScale;
}

World::Chunk& World::ensureChunk(int cx, int cy) const {
    const std::int64_t key = chunkKey(cx, cy);
    auto it = chunks_.find(key);
    if(it == chunks_.end()) {
        it = chunks_.emplace(key, Chunk{}).first;
    }

    if(!it->second.generated) {
        generateChunk(cx, cy, it->second);
        it->second.generated = true;
    }

    return it->second;
}

void World::generateChunk(int cx, int cy, Chunk& chunk) const {
    const auto& cfg = gameConfig().world;
    for(int ly = 0; ly < kChunkSize; ++ly) {
        for(int lx = 0; lx < kChunkSize; ++lx) {
            const int wx = cx * kChunkSize + lx;
            const int wy = cy * kChunkSize + ly;

            const double biome = noise01(wx / 4, wy / 4, cfg.biomeSalt);
            const double density = noise01(wx, wy, cfg.densitySalt);
            const double ore = noise01(wx, wy, cfg.oreSalt);
            const double trees = noise01(wx, wy, cfg.treeSalt);

            TileType tile = TileType::Empty;
            if(biome < cfg.coldBiomeMax) {
                if(density < cfg.coldWallThreshold) {
                    tile = TileType::Wall;
                }
                if(ore < cfg.coldOreThreshold) {
                    tile = TileType::Resource;
                }
            } else if(biome < cfg.warmBiomeMax) {
                if(density < cfg.warmWallThreshold) {
                    tile = TileType::Wall;
                }
                if(ore < cfg.warmOreThreshold) {
                    tile = TileType::Resource;
                }
                if(tile == TileType::Empty && trees < cfg.warmTreeThreshold) {
                    tile = TileType::Tree;
                }
            } else {
                if(density < cfg.mossWallThreshold) {
                    tile = TileType::Wall;
                }
                if(ore < cfg.mossOreThreshold) {
                    tile = TileType::Resource;
                }
                if(tile == TileType::Empty && trees < cfg.mossTreeThreshold) {
                    tile = TileType::Tree;
                }
            }

            chunk.tiles[ly * kChunkSize + lx] = tile;
        }
    }
}

void World::carveGuaranteedPath() {
    const auto& cfg = gameConfig().world;
    int x = spawn_.x;
    int y = spawn_.y;

    setTile(x, y, TileType::Empty);

    while(x != exit_.x) {
        x += (x < exit_.x) ? 1 : -1;
        setTile(x, y, TileType::Empty);
    }

    while(y != exit_.y) {
        y += (y < exit_.y) ? 1 : -1;
        setTile(x, y, TileType::Empty);
    }

    // Keep a small clear area around spawn for stable RL starts.
    for(int dy = -cfg.spawnClearRadius; dy <= cfg.spawnClearRadius; ++dy) {
        for(int dx = -cfg.spawnClearRadius; dx <= cfg.spawnClearRadius; ++dx) {
            setTile(spawn_.x + dx, spawn_.y + dy, TileType::Empty);
        }
    }

    // Keep the final goal clearly reachable.
    for(int dy = -cfg.exitClearRadius; dy <= cfg.exitClearRadius; ++dy) {
        for(int dx = -cfg.exitClearRadius; dx <= cfg.exitClearRadius; ++dx) {
            setTile(exit_.x + dx, exit_.y + dy, TileType::Empty);
        }
    }

    setTile(exit_.x, exit_.y, TileType::Exit);
}

}  // namespace stoneforge
