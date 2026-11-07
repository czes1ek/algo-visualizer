#include "algoviz/grid.hpp"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>

namespace algoviz {

Grid::Grid(int width, int height)
    : width_(width),
      height_(height),
      cells_(static_cast<std::size_t>(width > 0 && height > 0 ? width * height : 0), Cell::Empty),
      start_(0),
      goal_(0) {
    if (width < 2 || height < 1) {
        throw std::invalid_argument("Grid needs at least 2x1 cells");
    }
    // Default endpoints: middle row, 10% in from each side.
    const int row = height / 2;
    start_ = index(std::max(0, width / 10), row);
    goal_ = index(std::min(width - 1, width - 1 - width / 10), row);
}

bool Grid::inBounds(int x, int y) const noexcept {
    return x >= 0 && y >= 0 && x < width_ && y < height_;
}

void Grid::set(int i, Cell c) {
    if (i == start_ || i == goal_) {
        return;
    }
    cells_.at(static_cast<std::size_t>(i)) = c;
}

void Grid::moveEndpoint(int& endpoint, Cell& under, int other, int i) {
    if (i < 0 || i >= size() || i == other || i == endpoint) {
        return;
    }
    cells_[static_cast<std::size_t>(endpoint)] = under;
    under = cells_[static_cast<std::size_t>(i)];
    cells_[static_cast<std::size_t>(i)] = Cell::Empty;
    endpoint = i;
}

void Grid::setStart(int i) {
    moveEndpoint(start_, underStart_, goal_, i);
}

void Grid::setGoal(int i) {
    moveEndpoint(goal_, underGoal_, start_, i);
}

int Grid::manhattan(int a, int b) const noexcept {
    return std::abs(xOf(a) - xOf(b)) + std::abs(yOf(a) - yOf(b));
}

int Grid::neighbors(int i, std::array<int, 4>& out) const {
    const int x = xOf(i);
    const int y = yOf(i);
    constexpr std::array<int, 4> dx{0, 1, 0, -1};  // Up, Right, Down, Left
    constexpr std::array<int, 4> dy{-1, 0, 1, 0};
    int n = 0;
    for (std::size_t d = 0; d < 4; ++d) {
        const int nx = x + dx[d];
        const int ny = y + dy[d];
        if (inBounds(nx, ny)) {
            const int ni = index(nx, ny);
            if (passable(ni)) {
                out[static_cast<std::size_t>(n++)] = ni;
            }
        }
    }
    return n;
}

void Grid::clear() {
    std::fill(cells_.begin(), cells_.end(), Cell::Empty);
    underStart_ = Cell::Empty;
    underGoal_ = Cell::Empty;
}

void Grid::clearType(Cell type) {
    std::replace(cells_.begin(), cells_.end(), type, Cell::Empty);
    if (underStart_ == type) underStart_ = Cell::Empty;
    if (underGoal_ == type) underGoal_ = Cell::Empty;
}

}  // namespace algoviz
