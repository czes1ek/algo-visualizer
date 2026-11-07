#include <catch2/catch_test_macros.hpp>

#include "algoviz/grid.hpp"

using namespace algoviz;

TEST_CASE("grid indexes and bounds", "[grid]") {
    Grid g(10, 5);
    REQUIRE(g.size() == 50);
    REQUIRE(g.index(3, 2) == 23);
    REQUIRE(g.xOf(23) == 3);
    REQUIRE(g.yOf(23) == 2);
    REQUIRE(g.inBounds(9, 4));
    REQUIRE_FALSE(g.inBounds(10, 0));
    REQUIRE_FALSE(g.inBounds(-1, 0));
}

TEST_CASE("start and goal can never be walls or overlap", "[grid]") {
    Grid g(10, 5);
    g.set(g.start(), Cell::Wall);
    g.set(g.goal(), Cell::Wall);
    REQUIRE(g.at(g.start()) == Cell::Empty);
    REQUIRE(g.at(g.goal()) == Cell::Empty);

    const int oldGoal = g.goal();
    g.setGoal(g.start());  // ignored: would land on start
    REQUIRE(g.goal() == oldGoal);

    g.set(g.index(0, 0), Cell::Wall);
    g.setStart(g.index(0, 0));  // moving onto a wall clears it
    REQUIRE(g.start() == g.index(0, 0));
    REQUIRE(g.at(g.start()) == Cell::Empty);
}

TEST_CASE("neighbors come in Up, Right, Down, Left order and skip walls", "[grid]") {
    Grid g(3, 3);
    g.setStart(g.index(0, 0));
    g.setGoal(g.index(2, 2));
    std::array<int, 4> nb{};
    const int center = g.index(1, 1);
    REQUIRE(g.neighbors(center, nb) == 4);
    REQUIRE(nb[0] == g.index(1, 0));
    REQUIRE(nb[1] == g.index(2, 1));
    REQUIRE(nb[2] == g.index(1, 2));
    REQUIRE(nb[3] == g.index(0, 1));

    g.set(g.index(2, 1), Cell::Wall);
    REQUIRE(g.neighbors(center, nb) == 3);
    REQUIRE(nb[1] == g.index(1, 2));

    REQUIRE(g.neighbors(g.index(0, 0), nb) == 2);  // corner
}

TEST_CASE("weighted cells cost more", "[grid]") {
    Grid g(4, 1);
    g.set(1, Cell::Weight);
    REQUIRE(g.cost(1) == kWeightCost);
    REQUIRE(g.cost(2) == 1);
}
