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

double World::biomeFieldForChunk(int cx, int cy) const {
    auto smoothstep = [](double t) {
        t = std::clamp(t, 0.0, 1.0);
        return t * t * (3.0 - 2.0 * t);
    };
    auto lerp = [](double a, double b, double t) {
        return a + (b - a) * t;
    };

    const auto& cfg = gameConfig().world;
    auto sampleValue = [&](double x, double y, std::uint64_t salt) {
        const int x0 = static_cast<int>(std::floor(x));
        const int y0 = static_cast<int>(std::floor(y));
        const int x1 = x0 + 1;
        const int y1 = y0 + 1;
        const double tx = smoothstep(x - static_cast<double>(x0));
        const double ty = smoothstep(y - static_cast<double>(y0));

        const double v00 = noise01(x0, y0, salt);
        const double v10 = noise01(x1, y0, salt);
        const double v01 = noise01(x0, y1, salt);
        const double v11 = noise01(x1, y1, salt);
        return lerp(lerp(v00, v10, tx), lerp(v01, v11, tx), ty);
    };

    // Chunk tags stay deterministic, but warped coordinates remove square-looking biome regions.
    double x = static_cast<double>(cx) * 0.22;
    double y = static_cast<double>(cy) * 0.22;

    const double warpX = sampleValue(x * 0.57 + 19.3, y * 0.57 - 7.1, cfg.biomeSalt ^ 0x9f4a7c15ULL);
    const double warpY = sampleValue(x * 0.57 - 11.8, y * 0.57 + 13.4, cfg.biomeSalt ^ 0xc2b2ae35ULL);
    x += (warpX - 0.5) * 2.8;
    y += (warpY - 0.5) * 2.8;

    const double n1 = sampleValue(x, y, cfg.biomeSalt ^ 0x31f2a3b6ULL);
    const double n2 = sampleValue(x * 2.1 - 3.7, y * 2.1 + 1.9, cfg.biomeSalt ^ 0x7e2d4c91ULL);
    const double n3 = sampleValue(x * 4.2 + 8.4, y * 4.2 - 5.6, cfg.biomeSalt ^ 0x4b9aa21dULL);

    const double combined = n1 * 0.62 + n2 * 0.28 + n3 * 0.10;
    return std::clamp(combined, 0.0, 1.0);
}

int World::biomeTagForChunk(int cx, int cy) const {
    const double n = biomeFieldForChunk(cx, cy);

    // 0=grasland, 1=wald, 2=wueste, 3=bergland, 4=steppe, 5=tundra, 6=hoelle
    if(n < 1.0 / 7.0) {
        return 0;
    }
    if(n < 2.0 / 7.0) {
        return 1;
    }
    if(n < 3.0 / 7.0) {
        return 2;
    }
    if(n < 4.0 / 7.0) {
        return 3;
    }
    if(n < 5.0 / 7.0) {
        return 4;
    }
    if(n < 6.0 / 7.0) {
        return 5;
    }
    return 6;
}

std::string_view World::biomeNameForTag(int tag) {
    switch(tag) {
        case 0:
            return "grasland";
        case 1:
            return "wald";
        case 2:
            return "wueste";
        case 3:
            return "bergland";
        case 4:
            return "steppe";
        case 5:
            return "tundra";
        case 6:
            return "hoelle";
        default:
            return "unbekannt";
    }
}

void World::placeBiomeStructure(int cx, int cy, Chunk& chunk) const {
    constexpr double kStructureChance = 0.10;

    const auto& cfg = gameConfig().world;
    const double roll = noise01(cx, cy, cfg.biomeSalt ^ 0x6c8f5a21ULL);
    if(roll >= kStructureChance) {
        return;
    }

    const int chunkMinX = cx * kChunkSize;
    const int chunkMinY = cy * kChunkSize;
    const int chunkMaxX = chunkMinX + kChunkSize - 1;
    const int chunkMaxY = chunkMinY + kChunkSize - 1;
    if(spawn_.x >= chunkMinX && spawn_.x <= chunkMaxX && spawn_.y >= chunkMinY && spawn_.y <= chunkMaxY) {
        return;
    }
    if(exit_.x >= chunkMinX && exit_.x <= chunkMaxX && exit_.y >= chunkMinY && exit_.y <= chunkMaxY) {
        return;
    }

    const int biomeTag = biomeTagForChunk(cx, cy);
    TileType structureTile = TileType::StructureGrassland;
    std::array<std::string_view, 5> pattern = {
        "..#..",
        ".###.",
        "#####",
        ".###.",
        "..#..",
    };

    switch(biomeTag) {
        case 0:
            structureTile = TileType::StructureGrassland;
            pattern = std::array<std::string_view, 5>{"#...#", ".#.#.", "#####", ".###.", "#...#"};
            break;
        case 1:
            structureTile = TileType::StructureForest;
            pattern = std::array<std::string_view, 5>{"..#..", ".###.", "##.##", "#...#", "#####"};
            break;
        case 2:
            structureTile = TileType::StructureDesert;
            pattern = std::array<std::string_view, 5>{"..#..", ".###.", "#####", ".###.", "..#.."};
            break;
        case 3:
            structureTile = TileType::StructureMountain;
            pattern = std::array<std::string_view, 5>{".###.", ".#.#.", "#####", "..#..", "..#.."};
            break;
        case 4:
            structureTile = TileType::StructureSteppe;
            pattern = std::array<std::string_view, 5>{"#...#", ".#.#.", "#####", ".#.#.", "#...#"};
            break;
        case 5:
            structureTile = TileType::StructureTundra;
            pattern = std::array<std::string_view, 5>{"..#..", ".###.", "##.##", "#...#", ".###."};
            break;
        case 6:
            structureTile = TileType::StructureHelle;
            pattern = std::array<std::string_view, 5>{".#.#.", "#####", ".###.", ".#.#.", "#####"};
            break;
        default:
            break;
    }

    const int maxOffsetX = std::max(0, kChunkSize - 5);
    const int maxOffsetY = std::max(0, kChunkSize - 5);
    const int originX = static_cast<int>(noise01(cx + 11, cy - 7, cfg.biomeSalt ^ 0x9b4a3d11ULL) * static_cast<double>(maxOffsetX + 1));
    const int originY = static_cast<int>(noise01(cx - 5, cy + 13, cfg.biomeSalt ^ 0xa27f19c5ULL) * static_cast<double>(maxOffsetY + 1));

    for(int y = 0; y < 5; ++y) {
        const std::string_view row = pattern[static_cast<std::size_t>(y)];
        for(int x = 0; x < 5 && x < static_cast<int>(row.size()); ++x) {
            if(row[static_cast<std::size_t>(x)] != '#') {
                continue;
            }
            chunk.tiles[(originY + y) * kChunkSize + (originX + x)] = structureTile;
        }
    }
}

int World::biomeTagAt(int x, int y) const {
    const int cx = floorDiv(x, kChunkSize);
    const int cy = floorDiv(y, kChunkSize);
    return biomeTagForChunk(cx, cy);
}

std::string_view World::biomeNameAt(int x, int y) const {
    return biomeNameForTag(biomeTagAt(x, y));
}

bool World::lakeMaskAt(int worldX, int worldY) const {
    const auto& cfg = gameConfig().world;

    // Blobby low-frequency mask; keeps lakes occasional and coherent.
    const double a = noise01(worldX / 7, worldY / 7, cfg.densitySalt ^ 0x77aa44ccULL);
    const double b = noise01(worldX / 3, worldY / 3, cfg.treeSalt ^ 0x11cc88ddULL);
    const double lakeScore = 0.75 * a + 0.25 * b;

    // Very rare lakes.
    return lakeScore > 0.86;
}

bool World::isLakeAt(int x, int y) const {
    // Never place lakes on spawn/exit tiles.
    if((x == spawn_.x && y == spawn_.y) || (x == exit_.x && y == exit_.y)) {
        return false;
    }
    return lakeMaskAt(x, y);
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
    const int cx = floorDiv(worldX, kChunkSize);
    const int cy = floorDiv(worldY, kChunkSize);
    const int biomeTag = biomeTagForChunk(cx, cy);

    const double density = noise01(worldX, worldY, cfg.densitySalt);
    const double ore = noise01(worldX, worldY, cfg.oreSalt);
    const double trees = noise01(worldX, worldY, cfg.treeSalt);

    double wallThreshold = 0.11;
    double oreThreshold = 0.03;
    double treeThreshold = 0.08;

    // Biome behavior profile requested by user.
    switch(biomeTag) {
        case 0:  // grasland: fewer trees
            wallThreshold = 0.10;
            oreThreshold = 0.03;
            treeThreshold = 0.05;
            break;
        case 1:  // wald: many trees, few stones
            wallThreshold = 0.07;
            oreThreshold = 0.015;
            treeThreshold = 0.23;
            break;
        case 2:  // wueste: many stones, no trees
            wallThreshold = 0.19;
            oreThreshold = 0.08;
            treeThreshold = 0.0;
            break;
        case 3:  // bergland: many stones
            wallThreshold = 0.25;
            oreThreshold = 0.10;
            treeThreshold = 0.02;
            break;
        case 4:  // steppe: grassland with a bit more trees
            wallThreshold = 0.11;
            oreThreshold = 0.03;
            treeThreshold = 0.09;
            break;
        case 5:  // tundra: normal stones and trees
            wallThreshold = 0.14;
            oreThreshold = 0.05;
            treeThreshold = 0.07;
            break;
        case 6:  // hoelle: many stones and some trees
            wallThreshold = 0.23;
            oreThreshold = 0.09;
            treeThreshold = 0.06;
            break;
        default:
            break;
    }

    TileType tile = TileType::Empty;

    // Lakes are obstacle-free regions.
    if(lakeMaskAt(worldX, worldY)) {
        return TileType::Empty;
    }

    if(density < wallThreshold) {
        tile = TileType::Wall;
    }
    // Ores only in bergland.
    if(biomeTag == 3 && ore < oreThreshold) {
        tile = TileType::Resource;
    }
    if(treeThreshold > 0.0 && tile == TileType::Empty && trees < treeThreshold) {
        tile = TileType::Tree;
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

    // Stage 3: rare biome-specific landmark structure.
    placeBiomeStructure(cx, cy, chunk);
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
