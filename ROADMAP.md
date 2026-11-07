# ROADMAP — AlgoVisualizer

## Done (v1.0.0)
- Core engine: Grid, five step-wise searches, seeded maze, random walls and weights, stats.
- Catch2 suite: 20 test cases, all passing with warnings as errors (Apple Clang, macOS).
- SFML 3 desktop app (built-in pixel font, demo and smoke-test flags). Builds against SFML 3.0.2; smoke-tested headless under Xvfb.
- WebAssembly front end (embind + Canvas 2D, mouse and touch, Night Shift palette). Checked in headless Chrome at 1280 and 390 px wide: an A* maze solve reaches "Found", drag-drawing works, no horizontal scroll, no console errors.
- CI (Linux, macOS, Windows MSYS2) and the GitHub Pages deploy workflow.

## Next (ideas)
- 8-directional movement with the octile heuristic.
- Bidirectional BFS.
- Sanitizer (ASan/UBSan) CI job.
- Windows release zip with static linking.
- Animated GIF for the README.
