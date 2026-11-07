#include "algoviz/search.hpp"

#include <algorithm>
#include <array>
#include <limits>

namespace algoviz {

namespace {
constexpr int kInf = std::numeric_limits<int>::max();
}  // namespace

const char* name(Algorithm a) noexcept {
    switch (a) {
        case Algorithm::BFS: return "BFS";
        case Algorithm::DFS: return "DFS";
        case Algorithm::Dijkstra: return "Dijkstra";
        case Algorithm::AStar: return "A*";
        case Algorithm::Greedy: return "Greedy Best-First";
    }
    return "?";
}

bool guaranteesOptimal(Algorithm a, bool gridHasWeights) noexcept {
    switch (a) {
        case Algorithm::BFS: return !gridHasWeights;
        case Algorithm::Dijkstra:
        case Algorithm::AStar: return true;
        case Algorithm::DFS:
        case Algorithm::Greedy: return false;
    }
    return false;
}

Search::Search(const Grid& grid, Algorithm algorithm)
    : grid_(grid),
      algorithm_(algorithm),
      g_(static_cast<std::size_t>(grid.size()), kInf),
      parent_(static_cast<std::size_t>(grid.size()), -1),
      closed_(static_cast<std::size_t>(grid.size()), false),
      overlay_(static_cast<std::size_t>(grid.size()), Overlay::None) {
    const int s = grid_.start();
    g_[static_cast<std::size_t>(s)] = 0;
    if (algorithm_ == Algorithm::BFS || algorithm_ == Algorithm::DFS) {
        list_.push_back(s);
    } else {
        heap_.emplace(heuristic(s), counter_++, s);
    }
    discover(s);
}

int Search::heuristic(int cell) const {
    switch (algorithm_) {
        case Algorithm::AStar: return g_[static_cast<std::size_t>(cell)] + grid_.manhattan(cell, grid_.goal());
        case Algorithm::Greedy: return grid_.manhattan(cell, grid_.goal());
        default: return g_[static_cast<std::size_t>(cell)];  // Dijkstra
    }
}

void Search::discover(int cell) {
    auto& o = overlay_[static_cast<std::size_t>(cell)];
    if (o == Overlay::None) {
        o = Overlay::Frontier;
        ++stats_.frontier;
    }
}

bool Search::popNext(int& node) {
    if (algorithm_ == Algorithm::BFS || algorithm_ == Algorithm::DFS) {
        while (!list_.empty()) {
            if (algorithm_ == Algorithm::BFS) {
                node = list_.front();
                list_.pop_front();
            } else {
                node = list_.back();
                list_.pop_back();
            }
            if (!closed_[static_cast<std::size_t>(node)]) {
                return true;
            }
        }
        return false;
    }
    while (!heap_.empty()) {
        node = std::get<2>(heap_.top());
        heap_.pop();
        if (!closed_[static_cast<std::size_t>(node)]) {
            return true;  // stale duplicates are skipped here
        }
    }
    return false;
}

Status Search::step() {
    if (done()) {
        return status_;
    }
    ++stats_.steps;
    int node = -1;
    if (!popNext(node)) {
        status_ = Status::NoPath;
        stats_.frontier = 0;
        return status_;
    }
    const auto un = static_cast<std::size_t>(node);
    closed_[un] = true;
    overlay_[un] = Overlay::Visited;
    --stats_.frontier;
    ++stats_.visited;
    visitOrder_.push_back(node);

    if (node == grid_.goal()) {
        buildPath();
        status_ = Status::Found;
        return status_;
    }
    expand(node);
    return status_;
}

Status Search::run(int maxSteps) {
    for (int i = 0; !done() && (maxSteps < 0 || i < maxSteps); ++i) {
        step();
    }
    return status_;
}

void Search::expand(int node) {
    std::array<int, 4> nb{};
    const int n = grid_.neighbors(node, nb);
    const int gNode = g_[static_cast<std::size_t>(node)];

    switch (algorithm_) {
        case Algorithm::BFS:
            for (int k = 0; k < n; ++k) {
                const int c = nb[static_cast<std::size_t>(k)];
                const auto uc = static_cast<std::size_t>(c);
                if (overlay_[uc] == Overlay::None) {
                    parent_[uc] = node;
                    g_[uc] = gNode + 1;
                    list_.push_back(c);
                    discover(c);
                }
            }
            break;
        case Algorithm::DFS:
            // Push in reverse so the stack pops Up first.
            for (int k = n - 1; k >= 0; --k) {
                const int c = nb[static_cast<std::size_t>(k)];
                const auto uc = static_cast<std::size_t>(c);
                if (!closed_[uc]) {
                    parent_[uc] = node;
                    g_[uc] = gNode + 1;
                    list_.push_back(c);
                    discover(c);
                }
            }
            break;
        case Algorithm::Dijkstra:
        case Algorithm::AStar:
            for (int k = 0; k < n; ++k) {
                const int c = nb[static_cast<std::size_t>(k)];
                const auto uc = static_cast<std::size_t>(c);
                const int ng = gNode + grid_.cost(c);
                if (!closed_[uc] && ng < g_[uc]) {
                    g_[uc] = ng;
                    parent_[uc] = node;
                    heap_.emplace(heuristic(c), counter_++, c);
                    discover(c);
                }
            }
            break;
        case Algorithm::Greedy:
            for (int k = 0; k < n; ++k) {
                const int c = nb[static_cast<std::size_t>(k)];
                const auto uc = static_cast<std::size_t>(c);
                if (overlay_[uc] == Overlay::None) {
                    g_[uc] = gNode + grid_.cost(c);
                    parent_[uc] = node;
                    heap_.emplace(heuristic(c), counter_++, c);
                    discover(c);
                }
            }
            break;
    }
}

void Search::buildPath() {
    path_.clear();
    for (int c = grid_.goal(); c != -1; c = parent_[static_cast<std::size_t>(c)]) {
        path_.push_back(c);
        if (c == grid_.start()) {
            break;
        }
    }
    std::reverse(path_.begin(), path_.end());
    stats_.pathLength = static_cast<int>(path_.size()) - 1;
    stats_.pathCost = 0;
    for (std::size_t i = 1; i < path_.size(); ++i) {
        stats_.pathCost += grid_.cost(path_[i]);
    }
    for (int c : path_) {
        overlay_[static_cast<std::size_t>(c)] = Overlay::Path;
    }
}

}  // namespace algoviz
