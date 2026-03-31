# 4-in-a-row

A Connect Four AI playground. The simulator lets you pit bots against each other (or yourself), while the `bots/` directory holds various AI implementations of varying strength and strategy.

## Project structure

```
simulator/       # Game engine
  sim.py         # Main entry point
  make-move.cpp  # Move validator and board updater

bots/
  me/            # Personal bot (bingo series)
    bingo.cpp    # Current version
    bingo-v1.cpp # Fixed-depth minimax
    bingo-v2.cpp # + iterative deepening with time limit
    bingo-v3.cpp # + refined move ordering
    bingo-v4.cpp # + depth-weighted scoring
  gpt/           # GPT-generated reference bots
    level1.cpp   # Basic minimax + alpha-beta
    level3.cpp   # + Zobrist hashing + iterative deepening
    level4.cpp
    level5.cpp
    level6.cpp
    level7.cpp   # Monte Carlo / bitboard approach
```

## How to run

```bash
cd simulator
python sim.py
```

You'll be prompted to choose a game mode:

1. **Human vs Human** — two players take turns in the terminal
2. **Human vs Bot** — play against a bot; you choose who goes first
3. **Bot vs Bot** — watch two bots compete

For any bot slot, you'll be asked for the path to a `.cpp` file. The simulator compiles it automatically. Example:

```
Enter bot address: ../bots/me/bingo.cpp
```

## How the bingo bot works

`bingo.cpp` uses **minimax with alpha-beta pruning** and several optimizations:

- **Iterative deepening** — searches deeper and deeper until the 950ms time limit
- **Transposition table** — caches board states via Zobrist hashing to avoid redundant evaluation
- **Move ordering** — tries center columns first and orders by static score, improving pruning efficiency
- **Precomputed evaluation** — lookup tables for all possible row/column/diagonal configurations (3^7 states) for fast scoring
- **Depth-weighted scoring** — prefers winning in fewer moves

## Requirements

- Python 3
- `g++` with C++17 support
