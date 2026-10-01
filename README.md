# Terminal Pong

A classic game of Pong that runs right in your terminal, rendered with ANSI
color escape codes — green paddle on the left, magenta on the right, a
bright yellow ball, and a cyan border.

```
LEFT: 2    (vs AI)    RIGHT: 1   [W/S] left  [Up/Down] right  [P] pause  [Q] quit
+----------------------------------------------------------------------------+
|#                             :                                             |
|#                             :                                             |
|#                             :                O                           #|
|                              :                                            #|
|                              :                                            #|
|                                                                            #|
+----------------------------------------------------------------------------+
```

## Features

- Full-color terminal rendering using raw ANSI escape sequences (no external
  dependencies).
- Single-player mode with a reactive AI opponent, or two-player mode on one
  keyboard.
- Smooth, non-blocking input handling — move your paddle without the game
  pausing to wait for a keypress.
- Classic Pong physics: the ball's bounce angle depends on where it hits the
  paddle, and it speeds up slightly with every return.
- Pause/resume, live score tracking, and a win screen at 5 points.

## Controls

| Key           | Action                          |
|---------------|----------------------------------|
| `W` / `S`     | Move the left paddle up / down  |
| `Up` / `Down` | Move the right paddle up / down (two-player mode only) |
| `P`           | Pause / resume                  |
| `R`           | Restart after a game ends       |
| `Q`           | Quit                             |
| `1` / `2`     | Choose Single Player / Two Player from the menu |

## Requirements

- A POSIX terminal (Linux or macOS).
- CMake 3.16+ and a C++17 compiler (g++ or clang++).

## Build & Run

```bash
cmake -S . -B build
cmake --build build
./build/pong
```

## Project Layout

```
src/
  game.hpp / game.cpp       Pure game state & physics (no terminal dependency)
  terminal.hpp / terminal.cpp  Raw mode, ANSI colors, non-blocking key reads
  main.cpp                  Menu, render loop, input handling
tests/
  test_game.cpp             Unit tests for the game logic
```

## Development

The game logic in `src/game.hpp`/`src/game.cpp` has no terminal dependency,
so it's unit tested directly:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Both g++ 12+ and clang++ 16+ are supported; pass
`-DCMAKE_CXX_COMPILER=clang++` to `cmake -S . -B build` to build with clang.

## License

MIT — see [LICENSE](LICENSE).
