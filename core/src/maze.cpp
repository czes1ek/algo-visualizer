#include "algoviz/maze.hpp"

#include <array>
#include <utility>
#include <vector>

namespace algoviz {

void generateMaze(Grid& grid, std::uint32_t seed) {
    Rng rng(seed);
    const int w = grid.width();
    const int h = grid.height();

    // Rooms at odd coordinates inside a one-cell border.
    const int lastX = (w - 2) % 2 == 1 ? w - 2 : w - 3;
    const int lastY = (h - 2) % 2 == 1 ? h - 2 : h - 3;
    if (lastX < 1 || lastY < 1) {
        grid.clear();
        return;
    }

    // Park the endpoints on rooms before walling, so set() can't skip them.
    const int startCell = grid.index(1, 1);
    const int goalCell = grid.index(lastX, lastY);
    if (goalCell == startCell) {
        grid.clear();
        return;
    }
    grid.setGoal(grid.index(0, 0) == grid.start() ? grid.index(1, 0) : grid.index(0, 0));
    grid.setStart(startCell);
    grid.setGoal(goalCell);

    grid.clear();
    for (int i = 0; i < grid.size(); ++i) {
        grid.set(i, Cell::Wall);
    }

    std::vector<bool> seen(static_cast<std::size_t>(grid.size()), false);
    std::vector<std::pair<int, int>> stack;
    stack.emplace_back(1, 1);
    seen[static_cast<std::size_t>(grid.index(1, 1))] = true;
    grid.set(grid.index(1, 1), Cell::Empty);

    constexpr std::array<int, 4> dx{0, 2, 0, -2};
    constexpr std::array<int, 4> dy{-2, 0, 2, 0};

    while (!stack.empty()) {
        const auto [x, y] = stack.back();
        std::array<int, 4> options{};
        int count = 0;
        for (int d = 0; d < 4; ++d) {
            const int nx = x + dx[static_cast<std::size_t>(d)];
            const int ny = y + dy[static_cast<std::size_t>(d)];
            if (nx >= 1 && ny >= 1 && nx <= lastX && ny <= lastY &&
                !seen[static_cast<std::size_t>(grid.index(nx, ny))]) {
                options[static_cast<std::size_t>(count++)] = d;
            }
        }
        if (count == 0) {
            stack.pop_back();
            continue;
        }
        const auto d = static_cast<std::size_t>(options[static_cast<std::size_t>(rng.below(count))]);
        const int nx = x + dx[d];
        const int ny = y + dy[d];
        grid.set(grid.index(x + dx[d] / 2, y + dy[d] / 2), Cell::Empty);  // knock down the wall between
        grid.set(grid.index(nx, ny), Cell::Empty);
        seen[static_cast<std::size_t>(grid.index(nx, ny))] = true;
        stack.emplace_back(nx, ny);
    }
}

void scatterWalls(Grid& grid, double density, std::uint32_t seed) {
    Rng rng(seed);
    grid.clear();
    for (int i = 0; i < grid.size(); ++i) {
        if (rng.chance(density)) {
            grid.set(i, Cell::Wall);
        }
    }
}

void scatterWeights(Grid& grid, double density, std::uint32_t seed) {
    Rng rng(seed);
    for (int i = 0; i < grid.size(); ++i) {
        if (grid.at(i) == Cell::Empty && rng.chance(density)) {
            grid.set(i, Cell::Weight);
        }
    }
}

}  // namespace algoviz
