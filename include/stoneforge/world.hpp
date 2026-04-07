#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <unordered_map>

#include "stoneforge/game_config.hpp"
#include "stoneforge/types.hpp"

namespace stoneforge {

class World {
public:
    static constexpr int kChunkSize = 32;

    explicit World(std::uint64_t seed = 0);

    void reset(std::uint64_t seed);

    TileType tileAt(int x, int y) const;
    void setTile(int x, int y, TileType tile);
    bool isPassable(int x, int y) const;
    int biomeTagAt(int x, int y) const;
    std::string_view biomeNameAt(int x, int y) const;
    bool isLakeAt(int x, int y) const;

    Vec2i spawnPoint() const;
    Vec2i exitPoint() const;

private:
    struct Chunk {
        std::array<TileType, kChunkSize * kChunkSize> tiles{};
        bool generated = false;
    };

    static int floorDiv(int value, int divisor);
    static int positiveMod(int value, int divisor);
    static std::int64_t chunkKey(int cx, int cy);

    std::uint64_t mix(std::uint64_t value) const;
    double noise01(int worldX, int worldY, std::uint64_t salt) const;
    double biomeFieldForChunk(int cx, int cy) const;
    int biomeTagForChunk(int cx, int cy) const;
    static std::string_view biomeNameForTag(int tag);
    bool lakeMaskAt(int worldX, int worldY) const;
    Vec2i chooseExitPoint(const WorldGenConfig& cfg) const;
    TileType sampleBaseTile(int worldX, int worldY, const WorldGenConfig& cfg) const;
    void runCellularSmoothingStage(int cx, int cy, Chunk& chunk, const WorldGenConfig& cfg) const;

    bool validateReachabilityWindow(int minX, int minY, int maxX, int maxY) const;

    Chunk& ensureChunk(int cx, int cy) const;
    void generateChunk(int cx, int cy, Chunk& chunk) const;
    void carveGuaranteedPath();

    std::uint64_t seed_ = 0;
    Vec2i spawn_ = {0, 0};
    Vec2i exit_ = {64, 64};
    mutable std::unordered_map<std::int64_t, Chunk> chunks_;
};

}  // namespace stoneforge
