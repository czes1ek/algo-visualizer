#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <string>
#include <vector>

#include "algoviz/search.hpp"

using namespace algoviz;

namespace {

/// Builds a grid from ASCII art: '#' wall, 'w' weight, 'S' start, 'G' goal, '.' empty.
Grid parse(const std::vector<std::string>& rows) {
    Grid g(static_cast<int>(rows[0].size()), static_cast<int>(rows.size()));
    int s = -1;
    int t = -1;
    for (int y = 0; y < g.height(); ++y) {
        for (int x = 0; x < g.width(); ++x) {
            const char c = rows[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
            const int i = g.index(x, y);
            if (c == 'S') s = i;
            if (c == 'G') t = i;
        }
    }
    // Move endpoints out of the way, then into place.
    g.setStart(s);
    g.setGoal(t);
    g.setStart(s);
    for (int y = 0; y < g.height(); ++y) {
        for (int x = 0; x < g.width(); ++x) {
            const char c = rows[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
            if (c == '#') g.set(g.index(x, y), Cell::Wall);
            if (c == 'w') g.set(g.index(x, y), Cell::Weight);
        }
    }
    return g;
}

bool pathIsValid(const Grid& g, const std::vector<int>& path) {
    if (path.empty() || path.front() != g.start() || path.back() != g.goal()) return false;
    for (std::size_t i = 1; i < path.size(); ++i) {
        if (!g.passable(path[i]) || g.manhattan(path[i - 1], path[i]) != 1) return false;
    }
    return true;
}

const std::vector<std::string> kOpenMaze = {
    "S....#....",
    ".###.#.##.",
    ".#...#..#.",
    ".#.###.##.",
    ".#.....#.G",
};

const std::vector<std::string> kWalledOff = {
    "S.#....",
    "..#....",
    "###..G.",
};

// The straight line is 4 moves through three weights (cost 16); the detour is 6 moves (cost 6).
const std::vector<std::string> kWeighted = {
    "SwwwG",
    ".....",
};

}  // namespace

TEST_CASE("every algorithm finds a valid path when one exists", "[search]") {
    const auto algo = GENERATE(Algorithm::BFS, Algorithm::DFS, Algorithm::Dijkstra, Algorithm::AStar,
                               Algorithm::Greedy);
    CAPTURE(name(algo));
    const Grid g = parse(kOpenMaze);
    Search s(g, algo);
    REQUIRE(s.run() == Status::Found);
    REQUIRE(pathIsValid(g, s.path()));
    REQUIRE(s.stats().pathLength == static_cast<int>(s.path().size()) - 1);
    REQUIRE(s.stats().visited == static_cast<int>(s.visitOrder().size()));
}

TEST_CASE("optimal algorithms find the known shortest path", "[search]") {
    const auto algo = GENERATE(Algorithm::BFS, Algorithm::Dijkstra, Algorithm::AStar);
    CAPTURE(name(algo));
    const Grid g = parse(kOpenMaze);
    Search s(g, algo);
    REQUIRE(s.run() == Status::Found);
    // Hand-checked: right 4, down 2, left 2, down 2, right 4, up 4, right 3, down 4.
    REQUIRE(s.stats().pathLength == 25);
    REQUIRE(s.stats().pathCost == 25);
}

TEST_CASE("unreachable goal reports NoPath and clears the frontier", "[search]") {
    const auto algo = GENERATE(Algorithm::BFS, Algorithm::DFS, Algorithm::Dijkstra, Algorithm::AStar,
                               Algorithm::Greedy);
    CAPTURE(name(algo));
    const Grid g = parse(kWalledOff);
    Search s(g, algo);
    REQUIRE(s.run() == Status::NoPath);
    REQUIRE(s.path().empty());
    REQUIRE(s.stats().frontier == 0);
    REQUIRE(s.stats().visited == 4);  // the 2x2 pocket around S
}

TEST_CASE("adjacent start and goal", "[search]") {
    const auto algo = GENERATE(Algorithm::BFS, Algorithm::DFS, Algorithm::Dijkstra, Algorithm::AStar,
                               Algorithm::Greedy);
    const Grid g = parse({"SG."});
    Search s(g, algo);
    REQUIRE(s.run() == Status::Found);
    REQUIRE(s.stats().pathLength == 1);
}

TEST_CASE("weights: Dijkstra and A* take the cheap detour, BFS takes the short line", "[search]") {
    const Grid g = parse(kWeighted);

    Search bfs(g, Algorithm::BFS);
    REQUIRE(bfs.run() == Status::Found);
    REQUIRE(bfs.stats().pathLength == 4);
    REQUIRE(bfs.stats().pathCost == 3 * kWeightCost + 1);

    for (auto algo : {Algorithm::Dijkstra, Algorithm::AStar}) {
        CAPTURE(name(algo));
        Search s(g, algo);
        REQUIRE(s.run() == Status::Found);
        REQUIRE(s.stats().pathLength == 6);  // down, 4 across, up
        REQUIRE(s.stats().pathCost == 6);
    }
}

TEST_CASE("A* expands no more nodes than Dijkstra on an open grid", "[search]") {
    Grid g(30, 20);
    Search d(g, Algorithm::Dijkstra);
    Search a(g, Algorithm::AStar);
    REQUIRE(d.run() == Status::Found);
    REQUIRE(a.run() == Status::Found);
    REQUIRE(a.stats().pathCost == d.stats().pathCost);
    REQUIRE(a.stats().visited < d.stats().visited);
}

TEST_CASE("searches are deterministic", "[search]") {
    const auto algo = GENERATE(Algorithm::BFS, Algorithm::DFS, Algorithm::Dijkstra, Algorithm::AStar,
                               Algorithm::Greedy);
    const Grid g = parse(kOpenMaze);
    Search a(g, algo);
    Search b(g, algo);
    a.run();
    b.run();
    REQUIRE(a.visitOrder() == b.visitOrder());
    REQUIRE(a.path() == b.path());
}

TEST_CASE("DFS explores Up first, BFS goes level by level", "[search]") {
    Grid g(5, 5);
    g.setStart(g.index(2, 2));
    g.setGoal(g.index(4, 4));

    Search dfs(g, Algorithm::DFS);
    dfs.step();
    dfs.step();
    REQUIRE(dfs.visitOrder()[1] == g.index(2, 1));  // went Up

    Search bfs(g, Algorithm::BFS);
    bfs.run(5);
    for (std::size_t i = 1; i < 5; ++i) {
        REQUIRE(g.manhattan(g.start(), bfs.visitOrder()[i]) == 1);
    }
}

TEST_CASE("step() animates one expansion at a time and stops when done", "[search]") {
    const Grid g = parse(kOpenMaze);
    Search s(g, Algorithm::AStar);
    REQUIRE(s.step() == Status::Running);
    REQUIRE(s.stats().visited == 1);
    REQUIRE(s.overlay()[static_cast<std::size_t>(g.start())] == Overlay::Visited);
    s.run();
    const auto steps = s.stats().steps;
    s.step();
    REQUIRE(s.stats().steps == steps);  // no-op after finishing
    REQUIRE(s.overlay()[static_cast<std::size_t>(g.goal())] == Overlay::Path);
}

TEST_CASE("optimality guarantees", "[search]") {
    REQUIRE(guaranteesOptimal(Algorithm::BFS, false));
    REQUIRE_FALSE(guaranteesOptimal(Algorithm::BFS, true));
    REQUIRE(guaranteesOptimal(Algorithm::AStar, true));
    REQUIRE(guaranteesOptimal(Algorithm::Dijkstra, true));
    REQUIRE_FALSE(guaranteesOptimal(Algorithm::DFS, false));
    REQUIRE_FALSE(guaranteesOptimal(Algorithm::Greedy, false));
}
