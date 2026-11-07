#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <vector>

#include "algoviz/maze.hpp"
#include "algoviz/search.hpp"

using namespace algoviz;

namespace {
std::vector<Cell> snapshot(const Grid& g) {
    std::vector<Cell> cells;
    for (int i = 0; i < g.size(); ++i) cells.push_back(g.at(i));
    return cells;
}
}  // namespace

TEST_CASE("maze generation is reproducible from its seed", "[maze]") {
    Grid a(40, 30);
    Grid b(40, 30);
    Grid c(40, 30);
    generateMaze(a, 1234);
    generateMaze(b, 1234);
    generateMaze(c, 99);
    REQUIRE(snapshot(a) == snapshot(b));
    REQUIRE(snapshot(a) != snapshot(c));
}

TEST_CASE("every open maze cell is reachable from the start", "[maze]") {
    const auto size = GENERATE(std::pair{40, 30}, std::pair{41, 31}, std::pair{11, 7}, std::pair{64, 36});
    const auto seed = GENERATE(1u, 7u, 2026u);
    Grid g(size.first, size.second);
    generateMaze(g, seed);

    // Flood fill from the start, then check that no passable cell was missed.
    std::vector<bool> seen(static_cast<std::size_t>(g.size()), false);
    std::vector<int> queue{g.start()};
    seen[static_cast<std::size_t>(g.start())] = true;
    for (std::size_t q = 0; q < queue.size(); ++q) {
        std::array<int, 4> nb{};
        const int n = g.neighbors(queue[q], nb);
        for (int k = 0; k < n; ++k) {
            const auto c = static_cast<std::size_t>(nb[static_cast<std::size_t>(k)]);
            if (!seen[c]) {
                seen[c] = true;
                queue.push_back(nb[static_cast<std::size_t>(k)]);
            }
        }
    }
    for (int i = 0; i < g.size(); ++i) {
        if (g.passable(i)) {
            REQUIRE(seen[static_cast<std::size_t>(i)]);
        }
    }
    Search s(g, Algorithm::AStar);
    REQUIRE(s.run() == Status::Found);
}

TEST_CASE("maze keeps endpoints open and walls the border", "[maze]") {
    Grid g(40, 30);
    generateMaze(g, 42);
    REQUIRE(g.start() == g.index(1, 1));
    REQUIRE(g.at(g.start()) == Cell::Empty);
    REQUIRE(g.at(g.goal()) == Cell::Empty);
    for (int x = 0; x < g.width(); ++x) {
        REQUIRE(g.at(g.index(x, 0)) == Cell::Wall);
        REQUIRE(g.at(g.index(x, g.height() - 1)) == Cell::Wall);
    }
    for (int y = 0; y < g.height(); ++y) {
        REQUIRE(g.at(g.index(0, y)) == Cell::Wall);
        REQUIRE(g.at(g.index(g.width() - 1, y)) == Cell::Wall);
    }
}

TEST_CASE("scatter respects density roughly and spares endpoints", "[maze]") {
    Grid g(100, 100);
    scatterWalls(g, 0.3, 5);
    int walls = 0;
    for (int i = 0; i < g.size(); ++i) walls += g.at(i) == Cell::Wall ? 1 : 0;
    REQUIRE(walls > 2500);
    REQUIRE(walls < 3500);
    REQUIRE(g.at(g.start()) == Cell::Empty);

    scatterWeights(g, 0.2, 6);
    int weights = 0;
    for (int i = 0; i < g.size(); ++i) weights += g.at(i) == Cell::Weight ? 1 : 0;
    REQUIRE(weights > 0);
    REQUIRE(g.at(g.goal()) == Cell::Empty);
}
