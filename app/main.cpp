#include <cstdlib>
#include <exception>
#include <iostream>
#include <string_view>

#include "app.hpp"

int main(int argc, char** argv) {
    // `algoviz --demo` starts with a maze and A* running.
    // `algoviz --smoke-test` does the same, then exits 0 once A* reaches the goal, or 1 if it
    // hasn't after 10 seconds (used for CI and local checks).
    bool smoke = false;
    bool demo = false;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);
        if (arg == "--smoke-test") smoke = true;
        if (arg == "--demo") demo = true;
    }
    try {
        algoviz::app::App app;
        if (demo || smoke) app.startDemo();
        return app.run(smoke);
    } catch (const std::exception& e) {
        std::cerr << "algoviz: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}
