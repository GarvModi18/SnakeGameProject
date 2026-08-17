# Lab 2_3 — Snake Game++ (design-first)

---

## 1. Tool and install route — [3]

| | |
|---|---|
| Agent used for run 2 | Claude Code (Opus 4.8) |
| `ubiquitous-language` install route | pasted `SKILL.md` — content read from the course repo into the session |
| `refactoring/` pack install route | pasted `SKILL.md` — `detect-code-smells` and `review-accuracy-calibration` read from the course repo into the session |

The course skills are not registered in this Claude Code build's skill list, so the native
`~/.claude/skills` / project `.claude/skills` mechanism was not used. I used the documented
paste fallback: the `SKILL.md` files were read from
`github.com/Ankush-Chander/IT643-software-design-and-testing` and applied in-session. Nothing
failed to install; the paste route was a deliberate choice, not a workaround for an error.

---

## 2. What I changed in the glossary — [4]

I dropped implementation/framework identifiers a naive pass promotes to "terms" (`frameCount`,
`rainbowPhase`, `GotoXY`, `SetColor`, `mciSendString` — `snake.cpp:166,45,82,77,105`). I split
the one word the code overloads in opposite directions: **Speed Level** (a difficulty tier,
higher = harder, `snake.cpp:161`) versus **Game Speed** (a millisecond delay, lower = faster,
`snake.cpp:165`). I flagged two real ambiguities the generator missed — "tail" meaning both the
body array and the single last-erased cell (`snake.cpp:163` vs `:180`), and Fruit vs Special
Fruit — and recorded the domain/code drift (UI "Red Heart"/"Blue Diamond" vs code
`fruit`/`specialFruit`) plus the fact that no `Snake` type exists in the base at all.

---

## 3. Smell delta — [6]

Reports: `lab2_3/audits/main.md` (the code as received, `6770cf0`) and
`lab2_3/audits/lab1-head.md` (after my Lab-1 PR, `68cc9e5`).

| | count | representative site (`file:line`) |
|---|---|---|
| Smells my Lab-1 PR **introduced** | 3 | `snake.cpp:1028` (Duplicate Code — P2 tail/collision block mirrors P1 at `:994`) |
| Smells my Lab-1 PR **left untouched** | 3 | `snake.cpp:37` (Dead Code — unused `HighScoreEntry`) |
| Smells my Lab-1 PR **removed** | 1 | `snake.cpp:148` → `snake.cpp:58` (Data Clumps — loose globals grouped into `SnakePlayer`) |

Introduced: Duplicate Code (`:1028`), Shotgun Surgery (17 `gameMode` sites, rep `:660`), Long
Method in `main()` (`:1208`). Left untouched: Long Method `Logic`/`Draw` (`:975`/`:738`),
triplicated fruit placement (`:689`), Dead Code (`:37`). Removed: the Data Clumps of the base.

---

## 4. Rejected candidates — [6]

| smell reported | `file:line` | why it does not hold |
|---|---|---|
| Switch Statements | `snake.cpp:896` | 4-value enum→delta map; polymorphic `Direction` classes would add a hierarchy and coupling to delete four `case` labels — a net loss |
| Large Class | `snake.cpp:1` | there is no class; the file is flat and procedural with one responsibility (run the game), so the size detector mis-fires |
| Primitive Obsession | `snake.cpp:77` | `ConsoleColor` maps 1:1 onto Win32 attribute bits; a wrapper type only couples game code harder to the console API |
| Data Class | `snake.cpp:58` | `SnakePlayer` is a deliberate plain aggregate mutated by the free functions; adding methods just relocates the same code |

---

## 5. Commit map — [7]

| # | sha | subject | what it is |
|---|---|---|---|
| 1 | fbbe90c | lab2_3: add ubiquitous-language glossary (Part B) | glossary |
| 2 | e74540d | lab2_3: add smell audits at main and Lab-1 HEAD (Part C) | smell report |
| 3 | b6397f6 | Refactor snake state into snakes[NUM_SNAKES] single source of truth | **the refactor, alone** |
| 4 | d786250 | Add second player: 2-player local multiplayer | **the feature, alone** |

---

## 6. Two-run measurement — [4]

Run 1 is the Lab-1 second-player feature (`6770cf0`→`68cc9e5`, `snake.cpp`). Run 2 is commit 4 alone.

| | Run 1 (Lab 1) | Run 2 (commit 4) |
|---|---|---|
| Smells introduced | 3 | 1 |
| Lines changed, `git diff --shortstat -w` | 522+ / 224− | 182+ / 56− |
| Lines changed, **raw** (no `-w`) | 558+ / 260− | 203+ / 77− |
| Functions reached | 10 | 7 |
| Prompts to working code | 1 | 1 |
| Wall-clock time | not recorded (not in Lab-1 `LLM-LOG.md`) | one automated session; not separately timed |

Commit 3 (the refactor) on its own: 141+ / 92− lines `-w`, 200+ / 151− raw.

Run-1 numbers are reconstructed from git history because the Lab-1 `LLM-LOG.md` recorded only
"prompts to working code = 1" (no line counts or timing). Functions reached = distinct functions
whose body the feature diff changes, measured the same way for both runs.

---

## 7. Analysis Q1–Q2 — [5]

**Q1 — which smell did commit 3 fix?** **Data Clumps** (`audits/main.md` M4; the "removed" row
of section 3). The snake's head, tail, direction and score were loose globals
(`snake.cpp:148,163,178`), so "how many snakes" was implicit in there being one copy of each.
That clump is exactly why the design-last Lab-1 PR had to introduce **Duplicate Code**
(`snake.cpp:1028` mirrors `:994`) and 17-site Shotgun Surgery. Before: adding a snake meant
duplicating every global and every loop over it — ~110 near-identical lines. After commit 3: the
state is one `Snake` struct, the count is the single constant `NUM_SNAKES`, and per-snake logic
runs once in a loop.

**Q2 — commit 4 vs the Lab-1 diff.** The feature shrank from **522+/224− (-w), 10 functions** to
**182+/56−, 7 functions**. Movement, growth, wall/self-collision, tail-update and fruit handling
were **not** re-duplicated — they already run once over `snakes[]`, so commit 4 only changed the
count and added the genuinely two-player parts (WASD for P2, inter-snake collision, winner
screen): 7 mode-branch sites vs 17, and 1 introduced smell vs 3. What did **not** improve: commit
3 was not free (141+/92− -w), so the design-first *total* is not much smaller than Lab-1's; and
`main()` still grew into a 116-line **Long Method**, because both approaches inline the game-over
screens — commit 4 introduced the same smell Lab-1 did. Net: the feature is smaller, lower-risk
and duplication-free, bought with a separate refactor step.

---

## 8. Analysis Q3–Q4 — [5]

**Q3 — did the Lab-1 assistant suggest restructuring first?** No. The Lab-1 `LLM-LOG.md` records
one feature-completion prompt — "chnage it accordingly and if its alredy there dont change" — and
the assistant only "verified existing 2-player logic" and adjusted controls, reaching working
code in 1 prompt. It never proposed grouping state, removing duplication, or a single source of
truth. To make it restructure first, the prompt would have had to ask for that — a smell/design
review and a "refactor to one source of truth before adding the player" instruction — rather than
"make it work / don't change what exists."

**Q4 — how do I know commit 3 preserved behaviour?** Honestly, I do not have execution-level
proof. There is no test suite. It rests on three things: the change is mechanical (each snake
global became `snakes[0].field`; each per-snake block became a `for (s < NUM_SNAKES)` loop that
runs exactly once when `NUM_SNAKES == 1`); the result compiles cleanly with `g++ -lwinmm`; and
diff review shows no altered logic. I did **not** play-test exhaustively — the game reads the
console with `_getch` and drives the Win32 console, which I could not script here. Stronger
evidence would be extracting `Logic` into a pure function and unit-testing base vs refactor over
seeded input sequences.

## If you did not finish

Finished: four ordered commits, both audits, glossary, report, and the checker all present;
`check-lab2_3.sh` passes with no failures.
