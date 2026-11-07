# Changelog

All notable changes to this project are documented here. Format: [Keep a Changelog](https://keepachangelog.com/en/1.1.0/). Versioning: [SemVer](https://semver.org/).

## [1.0.0] - 2026-11-06
### Added
- C++20 core engine: grid with walls and weighted cells; step-wise BFS, DFS, Dijkstra, A* and Greedy Best-First with deterministic expansion order; stats (visited, frontier, path length, path cost, steps); optimality guarantee per algorithm.
- Seeded recursive-backtracker maze, random walls and random weights.
- SFML 3 desktop app with mouse drawing, draggable start and goal, keyboard controls, speed control, a stats panel and a built-in pixel font.
- WebAssembly build (Emscripten + embind) with a Canvas 2D front end, touch support and keyboard shortcuts, deployed to GitHub Pages.
- Catch2 unit tests; CI on Linux, macOS and Windows (MSYS2).
