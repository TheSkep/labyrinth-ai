# ZabZabAI — an AI for the Labyrinth board game

A program in C that plays a two-player version of Ravensburger's *Labyrinth*
against other programs, through a game server. Written for a C programming
contest in 2026.

## Result

Semi-finalist (top 4 of 50 programs) in the contest's single-elimination
tournament. To let the tournament finish on schedule, it was played at 2
seconds per move, half the time the AI normally gets.

## The game

The board is a grid of tiles with walls and passages. Each turn, a player first
slides the spare tile into a row or column, in one of four rotations, which shifts
the whole row and pushes another tile out; then moves their pawn anywhere it can
reach. The first player to collect all 24 treasures, in order, wins. In this
version both players know where every treasure is, and a push may not undo the
opponent's last one.

## How the AI chooses a move

- **Search.** Minimax with alpha-beta pruning over every legal insertion and
  rotation, for both players. The pawn move after each insertion is modelled
  greedily: take the treasure if it is reachable, otherwise get as close to it as
  possible.
- **Time.** Iterative deepening under a 4-second limit per move. A depth that
  runs out of time is discarded and the previous depth's choice is kept; the best
  move from the previous depth is searched first, so pruning cuts more.
- **Evaluation.** Treasures collected dominate. Below that, each pawn's distance
  to its next treasure is measured in walls: a 0-1 BFS in which an open passage
  costs 0 and crossing a wall costs 1, i.e. how many walls the board would have to
  shift away. The opponent's distance is weighted 1.4 times our own, which makes
  the AI favour blocking.
- **Playing the move.** After the chosen insertion, the pawn takes its treasure
  if reachable and chains the following ones while they lie within 9 extra steps;
  otherwise it moves to the reachable tile closest to its target.

## Files

| File | Content |
|---|---|
| `ZabZabAI.c` | The AI: search, evaluation, move construction |
| `Labyrinth.c`, `Labyrinth.h` | Board state, tile rotation and insertion, reachability, main game loop |
| `clientAPI.*`, `labyrinthAPI.*` | Server communication, provided by the contest organisers (T. Hilaire, J. Brajard; GPL) |

## Build and run

```bash
make
./Labyrinth
```

The program connects to a game server, `localhost` on port 1234 by default
(set in `main()` in `Labyrinth.c`), and plays one match after another. The
server was the contest's own and is not included here. `make debug` builds
the same program with AddressSanitizer and UndefinedBehaviorSanitizer.

## Licence

GPL-3.0 (see `LICENSE`), as the provided API is under the GPL.
