# Lab 4 — Group A

| | |
|---|---|
| Repository | GarvModi18/SnakeGameProject |
| Base tag | `lab4-base` at commit `6770cf0f123401c1ae5fa4bdaca204455acdf52f` |
| Pull request | https://github.com/GarvModi18/SnakeGameProject/pull/3 |

---

## 1. Five rules — [5]

Written before opening the source. Behaviour, with an observable outcome.

| # | Rule |
|---|---|
| 1 | Eating a regular fruit increases the player's score by 10, increases the snake's tail length by 1, and spawns a new fruit at an unoccupied coordinate. |
| 2 | When Border Wrap is disabled (OFF), moving the snake's head into or beyond any boundary wall causes game over (`gameOver = true`). |
| 3 | Moving the snake's head into any segment of its own tail causes game over (`gameOver = true`). |
| 4 | When Border Wrap is enabled (ON), moving the snake's head past a border boundary wraps it to the opposite boundary without causing game over. |
| 5 | When the game ends with a score strictly higher than the current high score, the game prompts the player to input their name and saves the new score and name to `highscore.txt`. |

---

## 2. What you could test, and what stopped you — [10]

No source changes in this part. Every `file:line` below is a line in `lab4-base`.

| # | Rule | Test written? | Blocking dependency (`file:line` + what it is) |
|---|---|---|---|
| 1 | Eating fruit increases score by 10 and tail length by 1 | Yes (partial) | `snake.cpp:947` — `rand()` called inside `Logic()` for new fruit position and `snake.cpp:831` — `rand()` for special fruit spawn |
| 2 | Wall collision (Border Wrap OFF) causes game over | Yes | None (testable in isolation via `Logic()` with `score <= highScore`) |
| 3 | Self-collision with own body causes game over | Yes | None (testable in isolation via `Logic()` with `score <= highScore`) |
| 4 | Border Wrap ON wraps head to opposite side | Yes | None (testable in isolation via `Logic()`) |
| 5 | High score prompts for player name and saves to file | No | `snake.cpp:247` — `_getch()` called inside `GetPlayerName()` infinite loop; and `snake.cpp:204` — `ofstream fileOut("highscore.txt")` hardcoded disk write |

> **Rules testable without modifying the source: 3 / 5** (4 / 5 for direct coordinate/score state updates; Rule 5 was fully blocked)

---

## 3. Coverage, and what it missed — [6]

| | |
|---|---|
| Line coverage | 24.68 % |
| Branch coverage | 33.02 % |
| Command used | `g++ --coverage -O0 -g tests.cpp -o tests.exe -lwinmm; .\tests.exe; gcov -b tests.cpp` |

**One rule that is executed by the suite but not verified by it:**

| | |
|---|---|
| Rule | Rule 1: Spawning a new fruit at an unoccupied coordinate after eating a fruit |
| Line that runs | `snake.cpp:947` — `fruitX = rand() % width; fruitY = rand() % height;` |
| The assertion that is missing | `assert(fruitX >= 0 && fruitX < width && fruitY >= 0 && fruitY < height);` (verifying that the newly spawned fruit coordinates are valid and unoccupied following fruit consumption) |

---

## 4. The seam — [10]

| | |
|---|---|
| Rule made testable | Rule 5: Prompting player name and persisting new high score upon game over |
| Commit 1 (seam) | `c88ba26f68a16f26c139292138bfc774820c2422` |
| Commit 2 (test) | `1fadb3254030e8e1b98c4e2312d57d18f6299238` |
| Seam kind | object |
| Enabling point | The `charSupplier` parameter in `GetPlayerName(int (*charSupplier)() = nullptr)` and the `inputCharProvider` delegate (plus filename parameter in `SaveHighScore`) |
| What production code gave up | Production code gave up its direct, hardcoded binding to the Windows C-runtime `_getch()` console input call and hardcoded string `"highscore.txt"`. In exchange, it now reads keystrokes through a configurable callable defaulting to `_getch()` and writes to a target filename defaulting to `"highscore.txt"`. |

---

## 5. The double — [4]

| | |
|---|---|
| What you passed through the seam | stub |
| The method under test | `GetPlayerName()` / `Logic()` (high-score branch) |

The collaborator (`charSupplier` / `TestStub_KeyReader`) was **asked a question** ("what is the next character of player input?") and returned scripted character data (`'C'`, `'h'`, `'a'`, `'m'`, `'p'`, `'i'`, `'o'`, `'n'`, `13`). Because it provides canned query responses for indirect input to the system under test rather than receiving commands to be verified by side-effect expectations, it is a **Stub**.

---

## 6. Two smells in your own tests — [5]

| | Smell | `file:line` | One-line fix |
|---|---|---|---|
| 1 | General fixture | `tests.cpp:14` | Encapsulate game state into a `Game` class and instantiate a fresh instance per test instead of resetting 18 shared mutable globals via `ResetGameState()`. |
| 2 | Mystery guest | `tests.cpp:212` | Pass an in-memory stream or a dedicated mock filesystem instead of reading and writing to temporary disk files (`test_save_load.txt`). |

---
