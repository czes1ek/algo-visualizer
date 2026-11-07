# SPEC — AlgoVisualizer

## What it is
An interactive pathfinding visualizer written in modern C++. Draw walls and weighted terrain on a grid, place a start and a goal, then watch BFS, DFS, Dijkstra, A* and Greedy Best-First explore the grid step by step. One C++ engine runs two front ends:
- **Desktop:** native app on SFML 3 (Windows, Linux, macOS).
- **Browser:** the same C++ core compiled to WebAssembly with Emscripten, live on GitHub Pages so anyone can try it in one click, including on a phone.

**Audience:** recruiters and engineers looking at Casper's portfolio. They should understand it in 10 seconds and be able to play with it in 30.

**Repo:** `github.com/czes1ek/algo-visualizer` (public, MIT license)
**Live demo:** `https://czes1ek.github.io/algo-visualizer/`

## Algorithms (core — all required)
| Algorithm | Frontier | Weighted? | Shortest path guaranteed? |
|---|---|---|---|
| BFS | queue | no (treats every cell as cost 1) | yes, unweighted |
| DFS | stack | no | no |
| Dijkstra | min-heap on g | yes | yes |
| A* | min-heap on f = g + h, Manhattan heuristic | yes | yes (admissible heuristic) |
| Greedy Best-First | min-heap on h | yes (cost reported) | no |

- 4-directional movement. Neighbor order is Up, Right, Down, Left. Ties in a heap break by insertion order, so every run is deterministic and testable.
- Cell cost: empty = 1, weighted = 5 (configurable constant), wall = impassable. The start and goal cost 1.
- Each algorithm runs **one step at a time**. A step pops one node and reports what changed (frontier added, node visited, path found or exhausted). The UI animates those steps.
- *Stretch:* 8-directional movement toggle with the octile heuristic. Bidirectional BFS.

## Maze and terrain generation
- Recursive-backtracker maze (seeded, so a seed always gives the same maze).
- Random scatter of walls (density slider, default 30%).
- Random weighted terrain.
- Clear walls, clear search (keep walls), clear all.

## Interaction
| Action | Desktop | Browser |
|---|---|---|
| Draw wall | left-drag | mouse drag or touch drag (wall mode) |
| Erase | right-drag | erase mode |
| Draw weight | hold W + drag | weight mode |
| Move start / goal | drag the start or goal cell | same, mouse or touch |
| Pick algorithm | 1–5 | dropdown, and 1–5 |
| Run / pause | Space | button, and Space |
| Single step | S | button, and S |
| Speed | Up/Down arrows | slider (1 step per frame up to instant) |
| Maze | M | button |
| Clear search / all | C / Shift+C | buttons |

Editing the grid while a search is shown clears the search overlay. Editing during a run pauses it first.

## Stats panel (both front ends)
Algorithm name, status (idle / running / found / no path), nodes visited, frontier size, path length (cells), path cost, and steps taken. After a run, show whether the path is guaranteed optimal for that algorithm.

## Look
The browser demo uses the casperbien.com "Night Shift" palette so it matches the portfolio: page navy `#0B0B30`, orange `#F7A21B`, purple `#5B16C9`, headline orange `#FF7A45`, magenta `#FF4FD8`, badge blue `#1652B8`, soft text `#D9D9F0`, CRT green `#3EE07A`.
Suggested cell colors: empty `#14143F`, wall `#D9D9F0`, weight `#5B16C9`, frontier `#FF4FD8`, visited `#1652B8`, path `#F7A21B`, start `#3EE07A`, goal `#FF7A45`. The desktop app uses the same colors.
Use a monospace font for stats. The desktop app draws text with a built-in 5×7 pixel font (no font files). The browser uses system fonts only.

## Quality bars
- C++20, zero warnings with the flags in CLAUDE.md rule 7, on GCC (Linux, MSYS2 on Windows) and Apple Clang (macOS), all in CI.
- Unit tests (Catch2 v3) for every algorithm: finds the known-optimal path length on fixed grids (BFS, Dijkstra, A*); start == goal; goal walled off (no path); weighted grid where the cheapest path is longer in cells than the shortest (Dijkstra and A* take the cheap one, BFS doesn't); deterministic order (the same grid gives the same visit sequence); maze generator (seeded and reproducible; every open cell is reachable). An ASan/UBSan CI job is *stretch*.
- Desktop: holds 60 fps on the default 40×30 grid at max speed.
- Browser: works in current Chrome, Firefox and Safari, including iPhone Safari with touch. No horizontal scroll at 390px wide. The whole page (HTML, JS, WASM, CSS) is under 1 MB uncompressed. No third-party requests, no analytics, no cookies.
- Accessibility (browser): every control is a real `<button>`/`<select>`/`<input>` with a label, there's visible focus, and the stats panel is text (not drawn on the canvas).

## README requirements
A short pitch, then a **live demo link at the top**, then an animated GIF (or a screenshot if a GIF can't be made) of A* solving a maze. After that: features, an algorithm table, controls, build instructions (Windows MSYS2, Linux, macOS, Web), architecture summary with a small diagram, how testing works, and a credit line (rule 4). Add CI and license badges.

## Out of scope
Sorting visualizations, saving/loading grids to files, user accounts, any backend.
