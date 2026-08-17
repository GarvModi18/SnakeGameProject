# Smell audit — `main` (the code as received)

- **State audited:** `snake.cpp` at commit `6770cf0` ("Revise README for Snake Game project") — the
  last commit by the original authors, before any of my Lab-1 work. This is a single-player game (one snake).
- **How:** `detect-code-smells` catalogue (Bloaters / OO-Abusers / Change-Preventers / Dispensables / Couplers),
  filtered through `review-accuracy-calibration` (confidence C1–C4, then severity per the calibration matrix).
- **Report only. No source was edited to produce this audit.** Line numbers are for `snake.cpp` as it stands at `6770cf0`.
- Confidence: **C4** provable, **C3** strong pattern, **C2** context-dependent. Code-smell severity is capped at
  MEDIUM by the calibration matrix (smells are maintainability, not correctness).

---

## Confirmed findings

#### M1 — Long Method: `Logic()` at snake.cpp:857
`Logic()` runs snake.cpp:857–971 (~115 lines) and carries at least eight jobs: direction latch (857),
slow-time countdown (860–868), speed recompute (869–870), tail shift (871–890), head move (896–903),
wall/self collision (905–920), game-over + high-score save (921–930), fruit eat/respawn (931–961),
special-fruit eat (962–970).
- Confidence **C3**, severity **MEDIUM**. Fix: Extract Method (`MoveSnake`, `CheckCollisions`, `HandleFruit`).

#### M2 — Long Method: `Draw()` at snake.cpp:708
`Draw()` runs snake.cpp:708–793 (~85 lines): redraw border (711–724), erase old tail (727–732), draw head
(733–744), draw tail (745–768), fruit (770–773), special fruit (774–780), HUD line (781–792).
- Confidence **C3**, severity **MEDIUM**. Fix: Extract Method (`DrawBorder`, `DrawSnake`, `DrawHud`).

#### M3 — Duplicate Code: random free-cell placement at snake.cpp:652, snake.cpp:834, snake.cpp:943
The "pick a random `(x,y)`; reject if it lands on the head, the tail, or the other fruit; repeat" loop is
written three times, almost verbatim: fruit spawn in `Setup()` snake.cpp:652–674, special-fruit spawn in
`SpawnSpecialFruit()` snake.cpp:834–846, fruit respawn in `Logic()` snake.cpp:943–960.
- Confidence **C4** (verified — three copies), severity **MEDIUM**. Fix: Extract Method `bool CellIsFree(x,y)` + a
  single `PlaceRandomly` helper.

#### M4 — Data Clumps: the snake is loose globals at snake.cpp:148, snake.cpp:163, snake.cpp:164, snake.cpp:178, snake.cpp:180
The state that *is* the snake is scattered across free globals that always travel together: `headX,headY`
snake.cpp:148, `tailX[],tailY[]` snake.cpp:163, `nTail` snake.cpp:164, `dir/lastDir` snake.cpp:178–179,
`lastTailX,lastTailY` snake.cpp:180, `score` snake.cpp:148. There is no `Snake` type; the "one snake" concept
is implicit in the fact that exactly one copy of each global exists.
- Confidence **C3**, severity **MEDIUM**. Fix: Introduce a `Snake` struct. **This is the clump the design-first
  commit 3 groups, and the reason the count of snakes has no single home.**

#### M5 — Dead Code: unused `HighScoreEntry` at snake.cpp:37 and unused `#include <vector>` at snake.cpp:2
`struct HighScoreEntry` is declared at snake.cpp:37 but never instantiated anywhere in the file; `<vector>` is
included at snake.cpp:2 but no `vector` is ever used. Both verified by full-file search.
- Confidence **C4**, severity **LOW**. Fix: delete the unused declaration at snake.cpp:37 and the unused include at snake.cpp:2.

#### M6 — Long Method: `Setup()` at snake.cpp:636
`Setup()` runs snake.cpp:636–706 (~70 lines): reset state, place fruit, hide cursor, paint border, start music.
- Confidence **C2**, severity **LOW**. Fix: Extract Method (`PaintBorder`, `PlaceFruit`).

---

## Considered and rejected (candidate smells that do not hold here)

#### R-A — Switch Statements on `dir` at snake.cpp:896 — rejected
The `switch(dir)` movement map and the head-glyph `if`-ladder at snake.cpp:735 look like the "Switch Statements"
OO-abuser, but a 4-value enum→delta mapping is not a polymorphism candidate. Replacing it with `Direction`
subclasses would add a class hierarchy and coupling to remove four `case` labels — a net loss.

#### R-B — Large Class on the whole file at snake.cpp:1 — rejected
A size-based detector flags the 1028-line file (snake.cpp:1) as an oversized class. There is no class — the
program is flat and procedural, with one cohesive responsibility (run one snake game). The label mis-fires on a
single-file C-style program; the real cost is the long methods already listed at snake.cpp:857 and snake.cpp:708.

#### R-C — Primitive Obsession on `ConsoleColor` at snake.cpp:77 — rejected
Passing `ConsoleColor` enums to `SetColor()` snake.cpp:77 could be read as Primitive Obsession. But `ConsoleColor`
maps 1:1 onto Win32 console attribute bits; wrapping it in a richer type would only couple game code more tightly
to the Windows console API without removing any duplication.
