# 4-in-a-row

A Connect Four AI playground. The simulator lets you pit bots against each other (or yourself), while `bots/` holds the AI implementations.

## Project structure

```
simulator/       # Game engine
  sim.py         # Main entry point
  make-move.cpp  # Move validator and board updater

bots/
  bingo-v1.cpp   # Fixed-depth minimax
  bingo-v2.cpp   # + iterative deepening with time limit
  bingo-v3.cpp   # + refined move ordering
  bingo-v4.cpp   # + depth-weighted scoring
  bingo-v5.cpp   # Current version
```

## How to run

```bash
make      # builds bots and creates ./sim in the project root
./sim     # start the simulator
```

You'll be prompted to choose a game mode:

1. **Human vs Human** — two players take turns in the terminal
2. **Human vs Bot** — play against a bot; you choose who goes first
3. **Bot vs Bot** — watch two bots compete

When a bot is needed, the simulator lists the available bots and you pick by number — no paths required.

## How the bingo bot works

`bingo-v5.cpp` uses **minimax with alpha-beta pruning** and several optimizations:

- **Iterative deepening** — searches deeper and deeper until the 950ms time limit
- **Transposition table** — caches board states via Zobrist hashing to avoid redundant evaluation
- **Move ordering** — tries center columns first and orders by static score, improving pruning efficiency
- **Precomputed evaluation** — lookup tables for all possible row/column/diagonal configurations (3^7 states) for fast scoring
- **Depth-weighted scoring** — prefers winning in fewer moves

## Requirements
- `python3` 
- `g++` with C++17 support
