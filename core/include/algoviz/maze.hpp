#pragma once

#include <cstdint>

#include "algoviz/grid.hpp"

namespace algoviz {

/// Small deterministic PRNG (xorshift32), identical on every platform and in WASM.
class Rng {
public:
    explicit Rng(std::uint32_t seed) : state_(seed == 0 ? 0x9E3779B9u : seed) {}
    std::uint32_t next() noexcept {
        state_ ^= state_ << 13;
        state_ ^= state_ >> 17;
        state_ ^= state_ << 5;
        return state_;
    }
    /// Uniform-enough integer in [0, n).
    int below(int n) noexcept { return n <= 0 ? 0 : static_cast<int>(next() % static_cast<std::uint32_t>(n)); }
    /// True with probability p (0..1).
    bool chance(double p) noexcept { return static_cast<double>(next()) / 4294967296.0 < p; }

private:
    std::uint32_t state_;
};

/// Recursive-backtracker maze (iterative). Rooms sit on odd coordinates; the start
/// moves to the top-left room and the goal to the bottom-right room. The same seed
/// always gives the same maze, and every open cell is reachable from the start.
void generateMaze(Grid& grid, std::uint32_t seed);

/// Clears the grid, then turns each cell into a wall with probability density.
void scatterWalls(Grid& grid, double density, std::uint32_t seed);

/// Turns empty cells into weighted cells with probability density (walls are kept).
void scatterWeights(Grid& grid, double density, std::uint32_t seed);

}  // namespace algoviz
