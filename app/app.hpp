#pragma once

#include <SFML/Graphics.hpp>

#include <cstdint>
#include <optional>

#include "algoviz/grid.hpp"
#include "algoviz/search.hpp"

namespace algoviz::app {

/// The SFML 3 desktop front end: input, animation and drawing around the core.
class App {
public:
    App();
    /// Runs until the window closes. If smokeTestSeconds > 0, closes itself after that long.
    int run(float smokeTestSeconds = 0.f);
    /// Loads a seeded maze and starts A* (used for screenshots and the smoke test).
    void startDemo();

private:
    enum class Drag : std::uint8_t { None, Paint, Erase, MoveStart, MoveGoal };

    void handle(const sf::Event& event);
    void onKey(const sf::Event::KeyPressed& key);
    void onPointer(sf::Vector2i pos);
    [[nodiscard]] std::optional<int> cellAt(sf::Vector2i pos) const;

    void resetSearch();
    void ensureSearch();
    void editStarted();
    void tick();
    void draw();
    [[nodiscard]] bool gridHasWeights() const;

    static constexpr int kCols = 40;
    static constexpr int kRows = 30;
    static constexpr float kCell = 20.f;
    static constexpr float kPanelW = 280.f;

    sf::RenderWindow window_;
    Grid grid_;
    std::optional<Search> search_;
    Algorithm algorithm_ = Algorithm::AStar;
    bool running_ = false;
    int speedIndex_ = 2;
    Drag drag_ = Drag::None;
    std::uint32_t seed_ = 2026;
    sf::VertexArray verts_{sf::PrimitiveType::Triangles};
};

}  // namespace algoviz::app
