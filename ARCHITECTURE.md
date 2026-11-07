# ARCHITECTURE — AlgoVisualizer

```
             ┌──────────────── core/ (pure C++20, no deps) ────────────────┐
             │ Grid · Search (BFS/DFS/Dijkstra/A*/Greedy) · maze · Stats   │
             └───────────────┬───────────────────────────┬─────────────────┘
                             │                           │
            app/ (SFML 3 desktop)                web/ (Emscripten + embind)
            App · pixel_font                     Engine → JS Canvas 2D → GitHub Pages
            tests/ (Catch2 v3) ── tests core only
```

## Repo layout
```
CMakeLists.txt  .clang-format  .gitignore  LICENSE
CLAUDE.md README.md SPEC.md ARCHITECTURE.md ROADMAP.md CHANGELOG.md
.claude/settings.json   .githooks/commit-msg   .github/workflows/{ci,pages}.yml
core/include/algoviz/{grid,search,maze}.hpp   core/src/{grid,search,maze}.cpp
app/{main.cpp, app.hpp, app.cpp, pixel_font.hpp, pixel_font.cpp}
web/{bindings.cpp, index.html, app.js, style.css}
tests/{test_grid,test_search,test_maze}.cpp
docs/media/web-demo.png
```

## Core design
- `Grid`: width, height, `std::vector<Cell>` (Empty, Wall, Weight), and start and goal indices. Cells use the flat index `y * w + x`. Start and goal can never become walls.
- `Search`: one class. The frontier is a `std::deque` (BFS queue, DFS stack) or a min-heap of `(priority, insertion counter, cell)`. `step()` pops and expands exactly one node. Duplicate heap entries are skipped when popped (lazy deletion).
- Per-cell `Overlay` (None, Frontier, Visited, Path) is what the renderers draw. The web build turns it into one code per cell and shares it with JS through `typed_memory_view`.
- `Rng` is xorshift32, so mazes are identical on every platform and in WASM. `std::uniform_int_distribution` isn't portable across standard libraries.

## Decision log
| # | Decision | Why | Status |
|---|---|---|---|
| D1 | C++20, CMake ≥ 3.24, Ninja | Modern standard; one build for every target | Locked |
| D2 | Core has no dependencies; front ends are thin | Lets the same engine run native and in WASM, and makes it testable | Locked |
| D3 | SFML 3: use the system package if `find_package(SFML 3)` finds it, otherwise FetchContent SFML 3.0.2 (audio and network off) | Fast local builds; reproducible CI | Decided |
| D4 | Web: Emscripten + embind, vanilla JS Canvas 2D (no SFML, no framework) | SFML doesn't target the web; keeps the page tiny | Locked |
| D5 | Emscripten flags: `-O3 -sMODULARIZE -sEXPORT_ES6 -sENVIRONMENT=web -sALLOW_MEMORY_GROWTH -sFILESYSTEM=0 -lembind`, single-threaded | No COOP/COEP headers needed on Pages | Decided |
| D6 | Hosting: GitHub Pages via `actions/deploy-pages` | Free; the live link sits next to the code | Locked |
| D7 | Tests: Catch2 v3.7.1 via FetchContent | Standard, and quick to set up | Decided |
| D8 | CI: Linux and macOS (build, test, Linux smoke test under Xvfb) plus Windows MSYS2 UCRT64 | Covers every platform Casper uses | Decided |
| D9 | MIT license | Standard for portfolio code | Decided |
| D10 | Fresh repo; the 2026 Windows prototype isn't included | The rebuild was done remotely, away from the PC that holds the prototype | Decided |
| D11 | No AI attribution: `attribution` set to empty plus a commit-msg hook | Casper's standing rule | Locked |
| D12 | Public repo `czes1ek/algo-visualizer` | Requested | Locked |
| D13 | One `Search` class with a switch per algorithm, not a class hierarchy | The five algorithms differ only in frontier type and priority; one class is shorter and easier to test | Decided |
| D14 | Built-in 5×7 pixel font in the desktop app | No binary font asset or license file to ship; fits the retro look | Decided |
| D15 | Endpoints can't overlap; `setGoal(start)` is ignored | Removes the start == goal edge case at the source | Decided |
