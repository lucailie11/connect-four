#!/usr/bin/env python3
import subprocess
import os
import shutil
import sys

HUMAN = "HUMAN"
PIECES = ['X', 'O']
WINS = ["WIN X", "WIN O"]
VALID = "VALID"
INVALID = "INVALID"

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
BIN  = os.path.join(ROOT, "bin")
TMP  = os.path.join(ROOT, "tmp")

MAKE_MOVE     = os.path.join(BIN, "make-move")
CURRENT_STATE = os.path.join(TMP, "current-state.txt")
TEMP_STATE    = os.path.join(TMP, "temp-state.txt")

class COLORS:
    RESET  = "\033[0m"
    BOLD   = "\033[1m"
    DIM    = "\033[2m"
    X      = "\033[92m"   # bright green  – player X
    O      = "\033[91m"   # bright red    – player O
    BOARD  = "\033[34m"   # blue          – board dots / borders
    PROMPT = "\033[96m"   # cyan          – input prompts
    INFO   = "\033[37m"   # white         – neutral info
    ERROR  = "\033[31m"   # red           – errors
    WIN    = "\033[95m"   # magenta       – win announcement
    MENU   = "\033[34m"   # blue          – menus

def color(color, text):
    return f"{color}{text}{COLORS.RESET}"

def bold(color, text):
    return f"{COLORS.BOLD}{color}{text}{COLORS.RESET}"

def init():
    if not os.path.isfile(MAKE_MOVE):
        print(color(COLORS.ERROR, "  Error: bin/make-move not found. Run 'make' first."))
        sys.exit(1)
    os.makedirs(TMP, exist_ok=True)
    board = (".......\n" * 6)
    with open(CURRENT_STATE, 'w') as f:
        f.write(board)
    with open(TEMP_STATE, 'w') as f:
        f.write(board)

def print_colored_board(text):
    col_header = "  " + " ".join(color(COLORS.DIM, str(i)) for i in range(1, 8))
    border     = color(COLORS.BOARD, "┌" + "─" * 15 + "┐")
    border_bot = color(COLORS.BOARD, "└" + "─" * 15 + "┘")

    print()
    print(col_header)
    print(border)

    for row in text.strip().split("\n"):
        rendered = ""
        for ch in row:
            if ch == 'X':
                rendered += bold(COLORS.X, "X") + color(COLORS.BOARD, " ")
            elif ch == 'O':
                rendered += bold(COLORS.O, "O") + color(COLORS.BOARD, " ")
            else:
                rendered += color(COLORS.BOARD, "· ")
        print(color(COLORS.BOARD, "│ ") + rendered.rstrip() + color(COLORS.BOARD, "│"))

    print(border_bot)
    print()

def update_and_print_state():
    shutil.copy(TEMP_STATE, CURRENT_STATE)
    with open(CURRENT_STATE) as infile:
        state = infile.read()
    print_colored_board(state)

def section(title):
    bar = color(COLORS.MENU, "─" * 38)
    print(f"\n{bar}\n {bold(COLORS.MENU, f"  {title}")} \n{bar}\n")

def choose_game_mode():
    section("Game Mode")
    print(f"  {color(COLORS.MENU, '1.')} Human vs Human")
    print(f"  {color(COLORS.MENU, '2.')} Human vs Bot")
    print(f"  {color(COLORS.MENU, '3.')} Bot vs Bot\n")
    game_mode = input(color(COLORS.PROMPT, "  Choose game mode [1-3]: "))
    while game_mode not in ['1', '2', '3']:
        print(color(COLORS.ERROR, "  Invalid choice. Please enter 1, 2, or 3."))
        game_mode = input(color(COLORS.PROMPT, "  Choose game mode [1-3]: "))
    print(color(COLORS.INFO, "\n  Game mode chosen.\n"))
    return int(game_mode)

def list_bots():
    return sorted(f for f in os.listdir(BIN) if f != 'make-move')

def get_bot():
    available = list_bots()
    if not available:
        print(color(COLORS.ERROR, "  No bots found in bin/. Run 'make' first."))
        sys.exit(1)

    section("Select Bot")
    for i, name in enumerate(available, 1):
        print(f"  {color(COLORS.MENU, str(i) + '.')} {name}")
    print()

    choice = input(color(COLORS.PROMPT, "  Pick a bot (number): ")).strip()
    while not choice.isdigit() or not (1 <= int(choice) <= len(available)):
        print(color(COLORS.ERROR, f"  Please enter a number between 1 and {len(available)}."))
        choice = input(color(COLORS.PROMPT, "  Pick a bot (number): ")).strip()

    return os.path.join(BIN, available[int(choice) - 1])

def get_players(game_mode):
    if game_mode == 1:
        return (HUMAN, HUMAN)
    if game_mode == 3:
        return (get_bot(), get_bot())

    print()
    first = input(color(COLORS.PROMPT, "  Do you want to play first? [y/n]: "))
    while first not in ['y', 'Y', 'n', 'N']:
        print(color(COLORS.ERROR, "  Please enter y or n."))
        first = input(color(COLORS.PROMPT, "  Do you want to play first? [y/n]: "))

    if first in ('y', 'Y'):
        return (HUMAN, get_bot())
    return (get_bot(), HUMAN)

def make_move(move):
    if not move.isdigit():
        return INVALID

    with open(CURRENT_STATE) as stdin, open(TEMP_STATE, 'w') as stdout:
        result = subprocess.run(
            [MAKE_MOVE, str(int(move))], stdin=stdin, stdout=stdout, stderr=subprocess.PIPE, text=True
        )
    return result.stderr.strip()

def play_round_by_human(player_id):
    piece_color = COLORS.X if player_id == 0 else COLORS.O
    prompt = color(COLORS.PROMPT, "  Enter column ") + bold(piece_color, f"[1-7]") + color(COLORS.PROMPT, ": ")

    move = input(prompt)
    result = make_move(move)

    while result == INVALID:
        if not move.isdigit():
            print(color(COLORS.ERROR, f"  \"{move}\" is not a valid column — please enter a number from 1 to 7."))
        elif not (1 <= int(move) <= 7):
            print(color(COLORS.ERROR, f"  Column {move} is out of range — please enter a number from 1 to 7."))
        else:
            print(color(COLORS.ERROR, f"  Column {move} is full — choose a different column."))
        move = input(prompt)
        result = make_move(move)

    return result

def play_round_by_bot(player, player_id):
    with open(CURRENT_STATE) as stdin:
        result = subprocess.run([player], stdin=stdin, capture_output=True, text=True)
    move = result.stdout.strip()
    if result.returncode == 0:
        print(color(COLORS.INFO, f"  Bot plays column {move}"))
        return make_move(move)
    print(color(COLORS.ERROR, f"  Bot crashed (exit code {result.returncode}) — opponent wins."))
    return WINS[player_id ^ 1]

def play_round(player, player_id):
    piece_color = COLORS.X if player_id == 0 else COLORS.O
    label = bold(piece_color, PIECES[player_id])
    kind  = "Human" if player == HUMAN else "Bot"
    print(color(COLORS.INFO, f"  Player ") + label + color(COLORS.INFO, f" ({kind}) to move"))

    if player == HUMAN:
        return play_round_by_human(player_id)
    return play_round_by_bot(player, player_id)

def announce_result(result):
    print()
    if result == WINS[0]:
        print(bold(COLORS.WIN, "  ★  Player X wins!  ★"))
    elif result == WINS[1]:
        print(bold(COLORS.WIN, "  ★  Player O wins!  ★"))
    else:
        print(bold(COLORS.WIN, "  It's a draw!"))
    print()

def start_game():
    init()
    game_mode = choose_game_mode()
    players = get_players(game_mode)

    section("Game Start")
    update_and_print_state()

    round_num = 0
    result = play_round(players[round_num % 2], round_num % 2)
    while result == VALID:
        update_and_print_state()
        round_num += 1
        result = play_round(players[round_num % 2], round_num % 2)

    update_and_print_state()
    announce_result(result)

if __name__ == "__main__":
    start_game()
