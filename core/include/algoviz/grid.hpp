#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace algoviz {

enum class Cell : std::uint8_t { Empty = 0, Wall = 1, Weight = 2 };

/// Cost of stepping onto a weighted cell. Empty cells (and start/goal) cost 1.
inline constexpr int kWeightCost = 5;

/// A rectangular grid of cells, addressed by flat index `y * width + x`.
class Grid {
public:
    Grid(int width, int height);

    [[nodiscard]] int width() const noexcept { return width_; }
    [[nodiscard]] int height() const noexcept { return height_; }
    [[nodiscard]] int size() const noexcept { return width_ * height_; }

    [[nodiscard]] bool inBounds(int x, int y) const noexcept;
    [[nodiscard]] int index(int x, int y) const noexcept { return y * width_ + x; }
    [[nodiscard]] int xOf(int i) const noexcept { return i % width_; }
    [[nodiscard]] int yOf(int i) const noexcept { return i / width_; }

    [[nodiscard]] Cell at(int i) const { return cells_.at(static_cast<std::size_t>(i)); }
    /// Sets a cell. Start and goal cells always stay Empty.
    void set(int i, Cell c);

    [[nodiscard]] int start() const noexcept { return start_; }
    [[nodiscard]] int goal() const noexcept { return goal_; }
    /// Moves the start (or goal). Ignored if it would land on the other endpoint.
    /// The endpoint's cell reads as Empty while it sits there; the cell type it
    /// covered (wall or weight) comes back when it moves on, so dragging an
    /// endpoint across the grid doesn't erase what it passes over.
    void setStart(int i);
    void setGoal(int i);

    [[nodiscard]] bool passable(int i) const { return at(i) != Cell::Wall; }
    /// Cost of entering cell i.
    [[nodiscard]] int cost(int i) const { return at(i) == Cell::Weight ? kWeightCost : 1; }
    [[nodiscard]] int manhattan(int a, int b) const noexcept;

    /// Writes passable 4-neighbours of i into out, in the fixed order
    /// Up, Right, Down, Left. Returns how many were written.
    int neighbors(int i, std::array<int, 4>& out) const;

    /// Sets every cell to Empty, including the cells under the endpoints.
    void clear();
    /// Sets every cell of the given type to Empty, including under the endpoints.
    void clearType(Cell type);

private:
    void moveEndpoint(int& endpoint, Cell& under, int other, int i);

    int width_;
    int height_;
    std::vector<Cell> cells_;
    int start_;
    int goal_;
    Cell underStart_ = Cell::Empty;  // what the start covers
    Cell underGoal_ = Cell::Empty;   // what the goal covers
};

}  // namespace algoviz
