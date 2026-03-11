#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>

#include "lecon/types.hpp"

namespace lecon {

class World {
public:
    static constexpr int kChunkSize = 32;

    explicit World(std::uint64_t seed = 0);

    void reset(std::uint64_t seed);

    TileType tileAt(int x, int y) const;
    void setTile(int x, int y, TileType tile);
    bool isPassable(int x, int y) const;

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

    Chunk& ensureChunk(int cx, int cy) const;
    void generateChunk(int cx, int cy, Chunk& chunk) const;
    void carveGuaranteedPath();

    std::uint64_t seed_ = 0;
    Vec2i spawn_ = {0, 0};
    Vec2i exit_ = {64, 64};
    mutable std::unordered_map<std::int64_t, Chunk> chunks_;
};

}  // namespace lecon
