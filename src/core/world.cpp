#include "stoneforge/world.hpp"

#include <algorithm>
#include <cmath>
#include <deque>
#include <vector>

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

    // Keep spawn area stable even when no guaranteed corridor is carved.
    for(int dy = -cfg.spawnClearRadius; dy <= cfg.spawnClearRadius; ++dy) {
        for(int dx = -cfg.spawnClearRadius; dx <= cfg.spawnClearRadius; ++dx) {
            setTile(spawn_.x + dx, spawn_.y + dy, TileType::Empty);
        }
    }

    exit_ = chooseExitPoint(cfg);

    if(cfg.forceGuaranteedPath) {
        carveGuaranteedPath();
    }

    if(cfg.enableFloodFillValidation) {
        const int radiusTiles = std::max(1, cfg.validationRadiusChunks) * kChunkSize;
        const int minX = std::min(spawn_.x, exit_.x) - radiusTiles;
        const int minY = std::min(spawn_.y, exit_.y) - radiusTiles;
        const int maxX = std::max(spawn_.x, exit_.x) + radiusTiles;
        const int maxY = std::max(spawn_.y, exit_.y) + radiusTiles;
        const bool reachable = validateReachabilityWindow(minX, minY, maxX, maxY);
        if(!reachable && cfg.guaranteedPathFallback) {
            carveGuaranteedPath();
        }
    }

    for(int dy = -cfg.exitClearRadius; dy <= cfg.exitClearRadius; ++dy) {
        for(int dx = -cfg.exitClearRadius; dx <= cfg.exitClearRadius; ++dx) {
            setTile(exit_.x + dx, exit_.y + dy, TileType::Empty);
        }
    }

    setTile(exit_.x, exit_.y, TileType::Exit);
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

Vec2i World::chooseExitPoint(const WorldGenConfig& cfg) const {
    if(!cfg.randomizeExitFromSpawn) {
        return cfg.exit;
    }

    const int minDist = std::max(1, cfg.exitMinDistance);
    const int maxDist = std::max(minDist, cfg.exitMaxDistance);
    const int searchMargin = std::max(16, cfg.exitClearRadius + 8);

    const int minX = spawn_.x - maxDist - searchMargin;
    const int maxX = spawn_.x + maxDist + searchMargin;
    const int minY = spawn_.y - maxDist - searchMargin;
    const int maxY = spawn_.y + maxDist + searchMargin;

    auto toKey = [](int x, int y) {
        const std::uint64_t hi = static_cast<std::uint64_t>(static_cast<std::uint32_t>(x));
        const std::uint64_t lo = static_cast<std::uint64_t>(static_cast<std::uint32_t>(y));
        return static_cast<std::int64_t>((hi << 32) | lo);
    };

    std::deque<Vec2i> queue;
    std::unordered_map<std::int64_t, bool> visited;
    std::vector<Vec2i> candidates;

    queue.push_back(spawn_);
    visited[toKey(spawn_.x, spawn_.y)] = true;

    constexpr std::array<Vec2i, 4> kDirs = {
        Vec2i{1, 0},
        Vec2i{-1, 0},
        Vec2i{0, 1},
        Vec2i{0, -1},
    };

    while(!queue.empty()) {
        const Vec2i current = queue.front();
        queue.pop_front();

        const int dx = current.x - spawn_.x;
        const int dy = current.y - spawn_.y;
        const int dist2 = dx * dx + dy * dy;
        const int min2 = minDist * minDist;
        const int max2 = maxDist * maxDist;
        if(dist2 >= min2 && dist2 <= max2) {
            candidates.push_back(current);
        }

        for(const Vec2i dir : kDirs) {
            const int nx = current.x + dir.x;
            const int ny = current.y + dir.y;
            if(nx < minX || nx > maxX || ny < minY || ny > maxY) {
                continue;
            }

            const std::int64_t key = toKey(nx, ny);
            if(visited.find(key) != visited.end()) {
                continue;
            }

            const int ndx = nx - spawn_.x;
            const int ndy = ny - spawn_.y;
            const int nd2 = ndx * ndx + ndy * ndy;
            if(nd2 > max2) {
                continue;
            }

            if(!isPassable(nx, ny)) {
                continue;
            }

            visited[key] = true;
            queue.push_back(Vec2i{nx, ny});
        }
    }

    if(!candidates.empty()) {
        const std::uint64_t mixed = mix(seed_ ^ cfg.biomeSalt ^ cfg.oreSalt ^ cfg.treeSalt);
        const std::size_t idx = static_cast<std::size_t>(mixed % static_cast<std::uint64_t>(candidates.size()));
        return candidates[idx];
    }

    return cfg.exit;
}

TileType World::sampleBaseTile(int worldX, int worldY, const WorldGenConfig& cfg) const {
    const double biome = noise01(worldX / 4, worldY / 4, cfg.biomeSalt);
    const double density = noise01(worldX, worldY, cfg.densitySalt);
    const double ore = noise01(worldX, worldY, cfg.oreSalt);
    const double trees = noise01(worldX, worldY, cfg.treeSalt);

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

    return tile;
}

void World::runCellularSmoothingStage(int cx, int cy, Chunk& chunk, const WorldGenConfig& cfg) const {
    if(!cfg.enableCellularSmoothing || cfg.cellularIterations <= 0) {
        return;
    }

    const int halo = cfg.cellularIterations;
    const int width = kChunkSize + halo * 2;
    const int height = kChunkSize + halo * 2;
    auto index = [width](int x, int y) {
        return y * width + x;
    };

    std::vector<TileType> current(static_cast<std::size_t>(width * height), TileType::Empty);
    std::vector<TileType> next = current;

    for(int y = 0; y < height; ++y) {
        for(int x = 0; x < width; ++x) {
            const int worldX = cx * kChunkSize + (x - halo);
            const int worldY = cy * kChunkSize + (y - halo);
            current[static_cast<std::size_t>(index(x, y))] = sampleBaseTile(worldX, worldY, cfg);
        }
    }

    auto isSolid = [](TileType tile) {
        return !objectForTile(tile).isPassable();
    };

    for(int iteration = 0; iteration < cfg.cellularIterations; ++iteration) {
        for(int y = 0; y < height; ++y) {
            for(int x = 0; x < width; ++x) {
                int solidNeighbors = 0;
                for(int ny = y - 1; ny <= y + 1; ++ny) {
                    for(int nx = x - 1; nx <= x + 1; ++nx) {
                        if(nx == x && ny == y) {
                            continue;
                        }
                        if(nx < 0 || ny < 0 || nx >= width || ny >= height) {
                            ++solidNeighbors;
                            continue;
                        }
                        if(isSolid(current[static_cast<std::size_t>(index(nx, ny))])) {
                            ++solidNeighbors;
                        }
                    }
                }

                const TileType source = current[static_cast<std::size_t>(index(x, y))];
                const bool currentlySolid = isSolid(source);
                if(currentlySolid) {
                    next[static_cast<std::size_t>(index(x, y))] =
                        (solidNeighbors >= cfg.cellularSurvivalMinNeighbors) ? source : TileType::Empty;
                } else {
                    next[static_cast<std::size_t>(index(x, y))] =
                        (solidNeighbors >= cfg.cellularBirthMinNeighbors) ? TileType::Wall : source;
                }
            }
        }
        current.swap(next);
    }

    for(int ly = 0; ly < kChunkSize; ++ly) {
        for(int lx = 0; lx < kChunkSize; ++lx) {
            chunk.tiles[ly * kChunkSize + lx] =
                current[static_cast<std::size_t>(index(lx + halo, ly + halo))];
        }
    }
}

bool World::validateReachabilityWindow(int minX, int minY, int maxX, int maxY) const {
    if(spawn_.x < minX || spawn_.x > maxX || spawn_.y < minY || spawn_.y > maxY) {
        return false;
    }
    if(exit_.x < minX || exit_.x > maxX || exit_.y < minY || exit_.y > maxY) {
        return false;
    }

    auto withinBounds = [minX, minY, maxX, maxY](int x, int y) {
        return x >= minX && x <= maxX && y >= minY && y <= maxY;
    };

    auto toKey = [](int x, int y) {
        const std::uint64_t hi = static_cast<std::uint64_t>(static_cast<std::uint32_t>(x));
        const std::uint64_t lo = static_cast<std::uint64_t>(static_cast<std::uint32_t>(y));
        return static_cast<std::int64_t>((hi << 32) | lo);
    };

    std::deque<Vec2i> queue;
    std::unordered_map<std::int64_t, bool> visited;
    queue.push_back(spawn_);
    visited[toKey(spawn_.x, spawn_.y)] = true;

    constexpr std::array<Vec2i, 4> kDirs = {
        Vec2i{1, 0},
        Vec2i{-1, 0},
        Vec2i{0, 1},
        Vec2i{0, -1},
    };

    while(!queue.empty()) {
        const Vec2i current = queue.front();
        queue.pop_front();

        if(current.x == exit_.x && current.y == exit_.y) {
            return true;
        }

        for(const Vec2i dir : kDirs) {
            const int nx = current.x + dir.x;
            const int ny = current.y + dir.y;
            if(!withinBounds(nx, ny)) {
                continue;
            }

            const std::int64_t key = toKey(nx, ny);
            if(visited.find(key) != visited.end()) {
                continue;
            }

            if(nx == exit_.x && ny == exit_.y) {
                return true;
            }

            if(!isPassable(nx, ny)) {
                continue;
            }

            visited[key] = true;
            queue.push_back(Vec2i{nx, ny});
        }
    }

    return false;
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

    // Stage 1: deterministic base material sampling (noise-driven).
    for(int ly = 0; ly < kChunkSize; ++ly) {
        for(int lx = 0; lx < kChunkSize; ++lx) {
            const int wx = cx * kChunkSize + lx;
            const int wy = cy * kChunkSize + ly;
            chunk.tiles[ly * kChunkSize + lx] = sampleBaseTile(wx, wy, cfg);
        }
    }

    // Stage 2: optional local smoothing with deterministic halo sampling.
    runCellularSmoothingStage(cx, cy, chunk, cfg);
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
