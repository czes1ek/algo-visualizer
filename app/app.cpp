#include "app.hpp"

#include <array>
#include <string>

#include "algoviz/maze.hpp"
#include "pixel_font.hpp"

namespace algoviz::app {

namespace {

// Night Shift palette (matches casperbien.com and the web demo).
const sf::Color kBg{0x0B, 0x0B, 0x30};
const sf::Color kEmpty{0x14, 0x14, 0x3F};
const sf::Color kWall{0xD9, 0xD9, 0xF0};
const sf::Color kWeight{0x5B, 0x16, 0xC9};
const sf::Color kFrontier{0xFF, 0x4F, 0xD8};
const sf::Color kVisited{0x16, 0x52, 0xB8};
const sf::Color kPath{0xF7, 0xA2, 0x1B};
const sf::Color kStart{0x3E, 0xE0, 0x7A};
const sf::Color kGoal{0xFF, 0x7A, 0x45};
const sf::Color kSoft{0xD9, 0xD9, 0xF0};
const sf::Color kOrange{0xF7, 0xA2, 0x1B};

// Steps per frame for each speed setting; the last one means "instant".
constexpr std::array<int, 7> kSpeeds{1, 2, 4, 8, 16, 64, -1};

const char* statusText(Status s) {
    switch (s) {
        case Status::Running: return "RUNNING";
        case Status::Found: return "FOUND";
        case Status::NoPath: return "NO PATH";
    }
    return "";
}

}  // namespace

App::App()
    : window_(sf::VideoMode({static_cast<unsigned>(kCols * kCell + kPanelW), static_cast<unsigned>(kRows * kCell)}),
              "AlgoVisualizer", sf::Style::Titlebar | sf::Style::Close),
      grid_(kCols, kRows) {
    window_.setFramerateLimit(60);
}

int App::run(bool smokeTest) {
    sf::Clock clock;
    while (window_.isOpen()) {
        while (const std::optional event = window_.pollEvent()) {
            handle(*event);
        }
        tick();
        draw();
        if (smokeTest) {
            const float t = clock.getElapsedTime().asSeconds();
            const bool found = search_ && search_->status() == Status::Found;
            if ((found && t > 1.f) || t > 10.f) {
                window_.close();
                return found ? 0 : 1;
            }
        }
    }
    return 0;
}

void App::startDemo() {
    generateMaze(grid_, 7);
    algorithm_ = Algorithm::AStar;
    speedIndex_ = 4;  // 16 steps per frame: visibly animated, done well inside the smoke test
    resetSearch();
    ensureSearch();
    running_ = true;
}

bool App::gridHasWeights() const {
    for (int i = 0; i < grid_.size(); ++i) {
        if (grid_.at(i) == Cell::Weight) return true;
    }
    return false;
}

void App::resetSearch() {
    search_.reset();
    running_ = false;
}

void App::ensureSearch() {
    if (!search_) {
        search_.emplace(grid_, algorithm_);
    }
}

void App::editStarted() {
    // Editing pauses a run and clears the old overlay.
    resetSearch();
}

std::optional<int> App::cellAt(sf::Vector2i pos) const {
    const int x = static_cast<int>(static_cast<float>(pos.x) / kCell);
    const int y = static_cast<int>(static_cast<float>(pos.y) / kCell);
    if (pos.x < 0 || pos.y < 0 || !grid_.inBounds(x, y)) {
        return std::nullopt;
    }
    return grid_.index(x, y);
}

void App::handle(const sf::Event& event) {
    if (event.is<sf::Event::Closed>()) {
        window_.close();
    } else if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::W) weightKeyHeld_ = true;
        onKey(*key);
    } else if (const auto* released = event.getIf<sf::Event::KeyReleased>()) {
        if (released->code == sf::Keyboard::Key::W) weightKeyHeld_ = false;
    } else if (event.is<sf::Event::FocusLost>()) {
        weightKeyHeld_ = false;
        drag_ = Drag::None;
    } else if (const auto* press = event.getIf<sf::Event::MouseButtonPressed>()) {
        const auto cell = cellAt(press->position);
        if (!cell) return;
        editStarted();
        if (*cell == grid_.start()) {
            drag_ = Drag::MoveStart;
        } else if (*cell == grid_.goal()) {
            drag_ = Drag::MoveGoal;
        } else {
            drag_ = press->button == sf::Mouse::Button::Right ? Drag::Erase : Drag::Paint;
        }
        onPointer(press->position);
    } else if (const auto* move = event.getIf<sf::Event::MouseMoved>()) {
        if (drag_ != Drag::None) onPointer(move->position);
    } else if (event.is<sf::Event::MouseButtonReleased>()) {
        drag_ = Drag::None;
    }
}

void App::onPointer(sf::Vector2i pos) {
    const auto cell = cellAt(pos);
    if (!cell) return;
    switch (drag_) {
        case Drag::Paint:
            grid_.set(*cell, weightKeyHeld_ ? Cell::Weight : Cell::Wall);
            break;
        case Drag::Erase: grid_.set(*cell, Cell::Empty); break;
        case Drag::MoveStart: grid_.setStart(*cell); break;
        case Drag::MoveGoal: grid_.setGoal(*cell); break;
        case Drag::None: break;
    }
}

void App::onKey(const sf::Event::KeyPressed& key) {
    using K = sf::Keyboard::Key;
    switch (key.code) {
        case K::Escape: window_.close(); break;
        case K::Num1:
        case K::Num2:
        case K::Num3:
        case K::Num4:
        case K::Num5:
            algorithm_ = static_cast<Algorithm>(static_cast<int>(key.code) - static_cast<int>(K::Num1));
            resetSearch();
            break;
        case K::Space:
            if (search_ && search_->done()) resetSearch();
            ensureSearch();
            running_ = !running_;
            break;
        case K::S:
            if (search_ && search_->done()) resetSearch();
            ensureSearch();
            running_ = false;
            search_->step();
            break;
        case K::Up:
            if (speedIndex_ + 1 < static_cast<int>(kSpeeds.size())) ++speedIndex_;
            break;
        case K::Down:
            if (speedIndex_ > 0) --speedIndex_;
            break;
        case K::M:
            resetSearch();
            generateMaze(grid_, seed_++);
            break;
        case K::R:
            resetSearch();
            scatterWalls(grid_, 0.3, seed_++);
            break;
        case K::T:
            resetSearch();
            scatterWeights(grid_, 0.15, seed_++);
            break;
        case K::C:
            resetSearch();
            if (key.shift) grid_.clear();
            break;
        default: break;
    }
}

void App::tick() {
    if (!running_ || !search_) return;
    const int perFrame = kSpeeds[static_cast<std::size_t>(speedIndex_)];
    search_->run(perFrame);
    if (search_->done()) running_ = false;
}

void App::draw() {
    verts_.clear();
    const auto* overlay = search_ ? &search_->overlay() : nullptr;

    for (int i = 0; i < grid_.size(); ++i) {
        sf::Color c = kEmpty;
        switch (grid_.at(i)) {
            case Cell::Wall: c = kWall; break;
            case Cell::Weight: c = kWeight; break;
            case Cell::Empty: break;
        }
        if (overlay && grid_.at(i) != Cell::Wall) {
            switch ((*overlay)[static_cast<std::size_t>(i)]) {
                case Overlay::Frontier: c = kFrontier; break;
                case Overlay::Visited: c = grid_.at(i) == Cell::Weight ? sf::Color{0x3A, 0x30, 0xC0} : kVisited; break;
                case Overlay::Path: c = kPath; break;
                case Overlay::None: break;
            }
        }
        if (i == grid_.start()) c = kStart;
        if (i == grid_.goal()) c = kGoal;
        drawRect(verts_, static_cast<float>(grid_.xOf(i)) * kCell + 1.f, static_cast<float>(grid_.yOf(i)) * kCell + 1.f,
                 kCell - 2.f, kCell - 2.f, c);
    }

    // Side panel.
    const float px = kCols * kCell + 16.f;
    float y = 16.f;
    drawText(verts_, "ALGOVISUALIZER", px, y, 2.f, kOrange);
    y += 30.f;
    const auto line = [&](const std::string& label, const std::string& value, sf::Color color = kSoft) {
        drawText(verts_, label, px, y, 2.f, kSoft);
        drawText(verts_, value, px + 120.f, y, 2.f, color);
        y += 22.f;
    };
    line("ALGO", name(algorithm_), kOrange);
    const int speed = kSpeeds[static_cast<std::size_t>(speedIndex_)];
    line("SPEED", speed < 0 ? "INSTANT" : std::to_string(speed) + "/FRAME");
    const Stats stats = search_ ? search_->stats() : Stats{};
    const bool paused = search_ && !running_ && search_->status() == Status::Running;
    line("STATUS", search_ ? (paused ? "PAUSED" : statusText(search_->status())) : "IDLE",
         search_ && search_->status() == Status::Found ? kStart : kSoft);
    line("VISITED", std::to_string(stats.visited));
    line("FRONTIER", std::to_string(stats.frontier));
    line("PATH LEN", std::to_string(stats.pathLength));
    line("PATH COST", std::to_string(stats.pathCost));
    line("STEPS", std::to_string(stats.steps));
    if (search_ && search_->status() == Status::Found) {
        line("OPTIMAL?", guaranteesOptimal(algorithm_, gridHasWeights()) ? "GUARANTEED" : "NOT GUARANTEED");
    }

    y += 14.f;
    drawText(verts_, "CONTROLS", px, y, 2.f, kOrange);
    y += 24.f;
    constexpr std::array<const char*, 12> help{
        "1-5     ALGORITHM", "SPACE   RUN/PAUSE", "S       STEP",      "UP/DN   SPEED",
        "LMB     WALL",      "W+LMB   WEIGHT",    "RMB     ERASE",     "DRAG S/G MOVE",
        "M       MAZE",      "R       RANDOM WALLS", "T       WEIGHTS", "C/SHIFT+C CLEAR"};
    for (const char* h : help) {
        drawText(verts_, h, px, y, 1.5f, kSoft);
        y += 15.f;
    }

    window_.clear(kBg);
    window_.draw(verts_);
    window_.display();
}

}  // namespace algoviz::app
