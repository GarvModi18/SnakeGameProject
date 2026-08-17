# Smell audit — Lab-1 HEAD (after my Lab-1 PR)

- **State audited:** `snake.cpp` at commit `68cc9e5` ("Add second player"), the tip of branch
  `feat/multiplayer` — the completed Lab-1 feature (1-player + 2-player local multiplayer). This is the
  design-last result: the second snake was added directly on top of the single-player code.
- **How:** same `detect-code-smells` catalogue + `review-accuracy-calibration` as `main.md`.
- **Report only. No source was edited.** Line numbers are for `snake.cpp` as it stands at `68cc9e5`
  (verify with `git show 68cc9e5:snake.cpp`).

---

## Confirmed findings

#### L1 — Duplicate Code: per-snake logic copy-pasted in `Logic()` at snake.cpp:1028 (mirrors snake.cpp:994) and snake.cpp:1084 (mirrors snake.cpp:1062)
The tail-shift + move block is written once for P1 (snake.cpp:994–1026) and copied for P2 (snake.cpp:1028–1060);
the bounds + self-collision block is written once for P1 (snake.cpp:1062–1082) and copied for P2
(snake.cpp:1084–1104). ~110 lines that differ only in the `p1`/`p2` prefix.
- Confidence **C4** (two literal copies), severity **MEDIUM**. **INTRODUCED by Lab-1.** Fix: hold snakes in an
  array and run the block once in a loop.

#### L2 — Duplicate Code: per-snake rendering copy-pasted in `Draw()` at snake.cpp:785 (mirrors snake.cpp:768) and snake.cpp:829 (mirrors snake.cpp:803)
Head drawing is duplicated P1 snake.cpp:768–783 / P2 snake.cpp:785–800; tail drawing is duplicated
P1 snake.cpp:803–827 / P2 snake.cpp:829–839.
- Confidence **C4**, severity **MEDIUM**. **INTRODUCED by Lab-1.** Fix: draw each snake in a loop.

#### L3 — Shotgun Surgery: the mode/player decision is smeared across 17 sites at snake.cpp:660, 669, 700, 761, 786, 830, 857, 907, 957, 978, 1029, 1085, 1107, 1130, 1149, 1179, 1193
Every `gameMode == MODE_MULTI/MODE_SINGLE` test (17 of them) is a place that must change to touch how many
snakes exist or how a snake behaves — spread across `Setup`, `Draw`, `Logic`, and `main`.
- Confidence **C3**, severity **MEDIUM**. **INTRODUCED by Lab-1** (base had zero mode branches). Fix: replace the
  `gameMode` flag + `p1,p2` with `snakes[NUM_SNAKES]` and loop.

#### L4 — Long Method: `Logic()` at snake.cpp:975
`Logic()` now spans snake.cpp:975–1205 (~230 lines) — roughly double its `main`-state size — after the P2
tail-update, P2 collision, and inter-player collision blocks were pasted in.
- Confidence **C4**, severity **MEDIUM**. Worsened from `main`. Fix: Extract Method + array loop.

#### L5 — Long Method: `main()` at snake.cpp:1208
`main()` spans snake.cpp:1208–1325 (~118 lines): two full game-over screens are inlined, one per mode
(single at snake.cpp:1229–1252, multi at snake.cpp:1268–1316).
- Confidence **C3**, severity **MEDIUM**. **INTRODUCED/worsened by Lab-1.** Fix: Extract Method `ShowGameOver()`.

#### L6 — Long Method: `Draw()` at snake.cpp:738
`Draw()` spans snake.cpp:738–886 (~150 lines) after the duplicated P2 head/tail blocks and the two-branch HUD
(snake.cpp:857–884) were added.
- Confidence **C3**, severity **MEDIUM**. Worsened from `main`.

#### L7 — Duplicate Code: random free-cell placement still triplicated at snake.cpp:689, snake.cpp:944, snake.cpp:1167
Carried over unchanged from `main`: the same place-a-random-cell loop appears in `Setup()` snake.cpp:689–706,
`SpawnSpecialFruit()` snake.cpp:944–964, and `Logic()` snake.cpp:1167–1185 (each now also duplicated across
`p1`/`p2`).
- Confidence **C4**, severity **MEDIUM**. **LEFT UNTOUCHED** by Lab-1.

#### L8 — Dead Code: unused `HighScoreEntry` at snake.cpp:37 and unused `#include <vector>` at snake.cpp:2
Identical to `main`: declared/included, never used.
- Confidence **C4**, severity **LOW**. **LEFT UNTOUCHED** by Lab-1.

---

## What Lab-1 removed (honest note)

#### Removed — Data Clumps: the loose snake globals of `main` are now a struct at snake.cpp:58
The `headX/headY/tail/nTail/dir/score` clump that was scattered in `main` (main.md M4) is encapsulated by Lab-1
into `struct SnakePlayer` at snake.cpp:58 (fields at snake.cpp:60–67). So the Data Clumps smell from the base is
genuinely **removed** — but Lab-1 then created *two* named copies (`SnakePlayer p1, p2` at snake.cpp:83) instead
of an array, which is what re-introduced the duplication (L1 snake.cpp:1028) and the shotgun surgery (L3 snake.cpp:660).

---

## Considered and rejected (candidate smells that do not hold here)

#### R-D — Data Class on `SnakePlayer` at snake.cpp:58 — rejected
`SnakePlayer` (snake.cpp:58) is fields-only, so a detector flags it as data-only. Rejected: it is a plain
aggregate deliberately mutated by the free functions `Setup/Logic/Draw`; giving it methods here would just
relocate the same procedural code and is out of scope for this game's style.

#### R-E — Temporary Field on `p2` at snake.cpp:83 — rejected
In single-player, `p2` is set `isAlive=false, nTail=0` (snake.cpp:684) and never used, which superficially
resembles the smell. Rejected: `p2` (snake.cpp:83) is not a half-initialised field of an object — it is a
top-level game entity that is simply inactive in one mode; the guard `gameMode == MODE_MULTI` is explicit.
