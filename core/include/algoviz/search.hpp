#pragma once

#include <cstdint>
#include <deque>
#include <queue>
#include <tuple>
#include <vector>

#include "algoviz/grid.hpp"

namespace algoviz {

enum class Algorithm : std::uint8_t { BFS = 0, DFS, Dijkstra, AStar, Greedy };
inline constexpr int kAlgorithmCount = 5;

enum class Status : std::uint8_t { Running = 0, Found, NoPath };

/// Per-cell search overlay, read by the renderers.
enum class Overlay : std::uint8_t { None = 0, Frontier = 1, Visited = 2, Path = 3 };

struct Stats {
    int visited = 0;     ///< nodes popped and expanded
    int frontier = 0;    ///< nodes discovered but not yet expanded
    int pathLength = 0;  ///< moves from start to goal (0 if no path)
    int pathCost = 0;    ///< sum of entry costs along the path
    int steps = 0;       ///< step() calls that did work
};

[[nodiscard]] const char* name(Algorithm a) noexcept;
/// True if the algorithm always returns a cheapest path on this grid model.
/// BFS is optimal only by move count, so it is false when weights exist.
[[nodiscard]] bool guaranteesOptimal(Algorithm a, bool gridHasWeights) noexcept;

/// A search that runs one node expansion per step(), so it can be animated.
/// Deterministic: neighbours are expanded Up, Right, Down, Left, and heap ties
/// break by insertion order.
class Search {
public:
    Search(const Grid& grid, Algorithm algorithm);

    /// Expands one node. Returns the status afterwards.
    Status step();
    /// Steps until done or until maxSteps steps have run (maxSteps < 0: no limit).
    Status run(int maxSteps = -1);

    [[nodiscard]] Algorithm algorithm() const noexcept { return algorithm_; }
    [[nodiscard]] Status status() const noexcept { return status_; }
    [[nodiscard]] bool done() const noexcept { return status_ != Status::Running; }
    [[nodiscard]] const Stats& stats() const noexcept { return stats_; }
    /// Cells from start to goal inclusive; empty unless status() == Found.
    [[nodiscard]] const std::vector<int>& path() const noexcept { return path_; }
    [[nodiscard]] const std::vector<Overlay>& overlay() const noexcept { return overlay_; }
    /// Cells in the order they were expanded.
    [[nodiscard]] const std::vector<int>& visitOrder() const noexcept { return visitOrder_; }

private:
    using HeapEntry = std::tuple<int, std::uint64_t, int>;  // priority, insertion, cell

    bool popNext(int& node);
    void expand(int node);
    void discover(int cell);
    void buildPath();
    [[nodiscard]] int heuristic(int cell) const;

    const Grid& grid_;
    Algorithm algorithm_;
    Status status_ = Status::Running;
    Stats stats_;

    std::deque<int> list_;  // BFS queue / DFS stack
    std::priority_queue<HeapEntry, std::vector<HeapEntry>, std::greater<>> heap_;
    std::uint64_t counter_ = 0;

    std::vector<int> g_;
    std::vector<int> parent_;
    std::vector<bool> closed_;
    std::vector<Overlay> overlay_;
    std::vector<int> visitOrder_;
    std::vector<int> path_;
};

}  // namespace algoviz
