# connect-four

A Connect Four bot playground. The simulator lets you pit bots against each other (or yourself), while `bots/` holds the bot implementations.

## Project structure

```
simulator/       # Game engine
  sim.py         # Main entry point
  make-move.cpp  # Move validator and board updater

bots/
  bingo-v1.cpp
  ...
  bingo-v6.cpp

  claude-v1.cpp 
  claude-v2.cpp
  claude-v3.cpp
```

## How to run

```bash
make      # builds bots and creates ./sim in the project root
./sim     # starts a game
```

You'll be prompted to choose a game mode:

1. **Human vs Human** — two players take turns in the terminal
2. **Human vs Bot** — play against a bot; you choose who goes first
3. **Bot vs Bot** — watch two bots compete

When a bot is needed, the simulator lists the available bots and you pick by number — no paths required.

## How the bingo bot works

bingo bots use **negamax with alpha-beta pruning** and progressively add optimizations across versions v1–v6:

- **Iterative deepening** — searches deeper and deeper until the 950ms time limit
- **Precomputed evaluation** — lookup tables for all possible row/column/diagonal configurations (3^n states) for fast scoring
- **Incremental scoring** — updates only the affected row/column/diagonals on each move instead of re-evaluating the full board
- **Transposition table** — caches board states via Zobrist hashing to avoid redundant evaluation
- **Move ordering** — center-first for shallow plies, static-score-ordered for deeper plies, improving pruning efficiency
- **Depth-weighted scoring** — prefers winning in fewer moves

## How the claude bots work

claude bots use **alpha-beta pruning** and progressively add optimizations across versions v1–v3:

- **Greedy win/block** — immediately takes a winning move or blocks the opponent's before doing any search
- **Move ordering** — center-first, then dynamic ordering by static score, improving pruning efficiency
- **Iterative deepening** — searches deeper and deeper until the 950ms time limit

## Requirements
- `python3` 
- `g++` with C++17 support
