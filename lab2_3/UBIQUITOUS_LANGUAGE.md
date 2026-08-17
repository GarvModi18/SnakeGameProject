# Ubiquitous Language — Snake Game++

- Generated with the `ubiquitous-language` skill (`--code` mode) against `snake.cpp` at the `main` base
  (`6770cf0`, the single-player game), then reviewed by hand against the source. Line numbers are for that file.
- Evidence order followed by the skill: types/enums → lifecycle → I/O → identifier names. Generic programming
  and Win32/framework identifiers were **excluded** on review (see the last section).

---

## Core domain terms

| Term | Definition | Aliases to avoid | Code location (`file:line`) |
|---|---|---|---|
| **Snake** | The player-controlled creature: a `Head` plus an ordered run of `Tail` cells that moves one cell per tick and grows when it eats. In this base version there is exactly one. | "player" (there is no player object here) | snake.cpp:148, snake.cpp:163 |
| **Head** | The leading cell of the snake; the only cell tested for eating and collision. | "front", "tip" | snake.cpp:148, snake.cpp:733 |
| **Tail** | The snake's trailing body: the cells `tailX[0..nTail-1]`. Follows the head each tick. | "body" (used loosely), "trail" | snake.cpp:163, snake.cpp:164 |
| **Direction** | The heading the head moves next tick — one of `STOP/LEFT/RIGHT/UP/DOWN`. A 180° reversal is rejected. | "heading", "velocity" | snake.cpp:170, snake.cpp:178 |
| **Fruit** | The ordinary collectible (rendered ♥, "Red Heart"): +10 `Score` and grows the snake by one. Exactly one is on the arena at a time. | "food", "apple", "Red Heart" (UI-only label) | snake.cpp:148, snake.cpp:770 |
| **Special Fruit** | The bonus collectible (rendered ♦, "Blue Diamond"): grants `Slow Time`, gives **no** score. At most one on screen. | "power-up fruit", "Blue Diamond" (UI-only label) | snake.cpp:151, snake.cpp:828 |
| **Slow Time** | A temporary state that holds game speed steady for `SLOW_TIME_FRAMES` ticks after eating a Special Fruit. | "freeze", "power-up" (generic) | snake.cpp:157, snake.cpp:850 |
| **Score** | Points for the current run; +10 per Fruit. | "points" | snake.cpp:148, snake.cpp:933 |
| **High Score** | The best score persisted to `highscore.txt` together with a player name. | "record", "best" | snake.cpp:167, snake.cpp:183 |
| **Speed Level** | A difficulty **tier** shown to the player; increments every 5 fruits eaten. Higher = harder. | "level", "stage" | snake.cpp:161, snake.cpp:936 |
| **Game Speed** | The per-tick delay in milliseconds (`Sleep(gameSpeed)`); **lower = faster**. Distinct from Speed Level. | "speed" (ambiguous — see below) | snake.cpp:165, snake.cpp:989 |
| **Border Wrap** | A toggle: when on, leaving one edge re-enters the opposite edge; when off, hitting a wall ends the game. | "teleport", "wall mode" | snake.cpp:43, snake.cpp:906 |
| **Rainbow Trail** | A cosmetic toggle that cycles the tail's colours instead of a flat green. | "rainbow mode" | snake.cpp:44, snake.cpp:750 |
| **Arena** | The `width` × `height` play grid enclosed by the drawn border. | "board", "map", "screen" | snake.cpp:146, snake.cpp:147 |
| **Game Over** | The terminal state of a run (wall hit, self hit, or quit); ends the game loop. | "death", "lose" | snake.cpp:145, snake.cpp:921 |

## Relationships (cardinality)

- One **Arena** hosts one **Snake**, one **Fruit**, and zero-or-one **Special Fruit** at a time — snake.cpp:146.
- One **Snake** has one **Head** and zero-or-more **Tail** cells (`nTail`) — snake.cpp:164.
- One run has one live **Score**; one **High Score** persists across runs — snake.cpp:167.
- Eating **Fruit** raises **Score** and, every 5 fruits, raises **Speed Level** (which lowers **Game Speed**) — snake.cpp:936.

## How the terms are used (illustrative)

> "The **snake** eats the **fruit**, so the **tail** grows and the **score** goes up; every fifth fruit the
> **speed level** ticks up and the **game speed** drops. Grabbing the **special fruit** starts **slow time**.
> With **border wrap** off, touching the wall is **game over**."

## Flagged ambiguities (genuine)

- **"Speed" points two opposite ways.** *Speed Level* (snake.cpp:161) is a tier where **higher = harder**, while
  *Game Speed* (snake.cpp:165) is a millisecond delay where **lower = faster**. The same English word means
  opposite numeric directions — always qualify which one.
- **"Tail" is overloaded.** It means both the whole body array `tailX[]/nTail` (snake.cpp:163) *and* the single
  last-erased cell `lastTailX/lastTailY` (snake.cpp:180). Prefer "tail cells" vs "last tail cell".
- **"Fruit" vs "Special Fruit".** Both are "fruit" in code but are different domain objects with different
  effects (score+grow vs slow-time, no score) — snake.cpp:148 vs snake.cpp:151.

## Code drift (domain word ≠ code name)

- UI/README say **Red Heart** and **Blue Diamond**; the code says `fruit` and `specialFruit` — snake.cpp:148,
  snake.cpp:151. Keep the domain terms **Fruit** / **Special Fruit**; treat the colour/shape names as render labels.
- There is no **Snake** (or Player) type at all in this base version — the concept lives only as loose globals
  (snake.cpp:148, snake.cpp:163). This gap is exactly what the design-first refactor closes.

## Deliberately excluded on review (not domain terms)

`frameCount` (snake.cpp:166), `rainbowPhase` (snake.cpp:45), `nTail` (snake.cpp:164, an implementation counter for
Tail), `GotoXY`/`SetColor`/`ClearScreen` (snake.cpp:82/77/91, Win32 console helpers), `mciSendString` music calls
(snake.cpp:105) — these are implementation or framework identifiers, not concepts a player or designer would name.
