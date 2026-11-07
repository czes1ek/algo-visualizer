// WebAssembly bindings: exposes the core to JavaScript through embind.
// The page draws a per-cell code buffer that this Engine fills each frame.

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <cstdint>
#include <optional>
#include <vector>

#include "algoviz/grid.hpp"
#include "algoviz/maze.hpp"
#include "algoviz/search.hpp"

namespace {

using namespace algoviz;

// Codes shared with web/app.js.
enum Code : std::uint8_t {
    kEmpty = 0, kWall = 1, kWeight = 2, kFrontier = 3, kVisited = 4,
    kPath = 5, kStart = 6, kGoal = 7, kVisitedWeight = 8
};

class Engine {
public:
    Engine(int width, int height) : grid_(width, height), view_(static_cast<std::size_t>(grid_.size()), 0) {}
    Engine(const Engine&) = delete;  // search_ refers to grid_
    Engine& operator=(const Engine&) = delete;

    int width() const { return grid_.width(); }
    int height() const { return grid_.height(); }
    int start() const { return grid_.start(); }
    int goal() const { return grid_.goal(); }
    int cell(int i) const { return static_cast<int>(grid_.at(i)); }

    void setCell(int i, int type) {
        reset();
        grid_.set(i, static_cast<Cell>(type));
    }
    void setStart(int i) { reset(); grid_.setStart(i); }
    void setGoal(int i) { reset(); grid_.setGoal(i); }
    void clearAll() { reset(); grid_.clear(); }
    void maze(std::uint32_t seed) { reset(); generateMaze(grid_, seed); }
    void scatterWalls(double density, std::uint32_t seed) { reset(); algoviz::scatterWalls(grid_, density, seed); }
    void scatterWeights(double density, std::uint32_t seed) { reset(); algoviz::scatterWeights(grid_, density, seed); }

    /// Starts a fresh search with algorithm index 0..4.
    void begin(int algorithm) { search_.emplace(grid_, static_cast<Algorithm>(algorithm)); }
    void reset() { search_.reset(); }
    bool active() const { return search_.has_value(); }

    /// Runs up to n steps (n < 0: until done). Returns -1 idle, 0 running, 1 found, 2 no path.
    int step(int n) {
        if (!search_) return -1;
        return static_cast<int>(search_->run(n));
    }
    int status() const { return search_ ? static_cast<int>(search_->status()) : -1; }

    int visited() const { return search_ ? search_->stats().visited : 0; }
    int frontier() const { return search_ ? search_->stats().frontier : 0; }
    int pathLength() const { return search_ ? search_->stats().pathLength : 0; }
    int pathCost() const { return search_ ? search_->stats().pathCost : 0; }
    int steps() const { return search_ ? search_->stats().steps : 0; }
    bool optimal() const {
        if (!search_) return false;
        bool weights = false;
        for (int i = 0; i < grid_.size() && !weights; ++i) weights = grid_.at(i) == Cell::Weight;
        return guaranteesOptimal(search_->algorithm(), weights);
    }

    /// Fills and returns a zero-copy view of one code per cell.
    emscripten::val view() {
        const auto* overlay = search_ ? &search_->overlay() : nullptr;
        for (int i = 0; i < grid_.size(); ++i) {
            const Cell c = grid_.at(i);
            std::uint8_t code = c == Cell::Wall ? kWall : c == Cell::Weight ? kWeight : kEmpty;
            if (overlay && c != Cell::Wall) {
                switch ((*overlay)[static_cast<std::size_t>(i)]) {
                    case Overlay::Frontier: code = kFrontier; break;
                    case Overlay::Visited: code = c == Cell::Weight ? kVisitedWeight : kVisited; break;
                    case Overlay::Path: code = kPath; break;
                    case Overlay::None: break;
                }
            }
            if (i == grid_.start()) code = kStart;
            if (i == grid_.goal()) code = kGoal;
            view_[static_cast<std::size_t>(i)] = code;
        }
        return emscripten::val(emscripten::typed_memory_view(view_.size(), view_.data()));
    }

private:
    Grid grid_;
    std::optional<Search> search_;
    std::vector<std::uint8_t> view_;
};

}  // namespace

EMSCRIPTEN_BINDINGS(algoviz) {
    emscripten::class_<Engine>("Engine")
        .constructor<int, int>()
        .function("width", &Engine::width)
        .function("height", &Engine::height)
        .function("start", &Engine::start)
        .function("goal", &Engine::goal)
        .function("cell", &Engine::cell)
        .function("setCell", &Engine::setCell)
        .function("setStart", &Engine::setStart)
        .function("setGoal", &Engine::setGoal)
        .function("clearAll", &Engine::clearAll)
        .function("maze", &Engine::maze)
        .function("scatterWalls", &Engine::scatterWalls)
        .function("scatterWeights", &Engine::scatterWeights)
        .function("begin", &Engine::begin)
        .function("reset", &Engine::reset)
        .function("active", &Engine::active)
        .function("step", &Engine::step)
        .function("status", &Engine::status)
        .function("visited", &Engine::visited)
        .function("frontier", &Engine::frontier)
        .function("pathLength", &Engine::pathLength)
        .function("pathCost", &Engine::pathCost)
        .function("steps", &Engine::steps)
        .function("optimal", &Engine::optimal)
        .function("view", &Engine::view);
}
