#include <cstdlib>
#include <exception>
#include <iostream>
#include <string_view>

#include "app.hpp"

int main(int argc, char** argv) {
    // `algoviz --demo` starts with a maze and A* running.
    // `algoviz --smoke-test` does the same, then exits after 3 seconds (used for CI and local checks).
    float smoke = 0.f;
    bool demo = false;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);
        if (arg == "--smoke-test") smoke = 3.f;
        if (arg == "--demo") demo = true;
    }
    try {
        algoviz::app::App app;
        if (demo || smoke > 0.f) app.startDemo();
        return app.run(smoke);
    } catch (const std::exception& e) {
        std::cerr << "algoviz: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}
