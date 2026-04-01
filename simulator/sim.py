#!/usr/bin/env python3
import subprocess
import os

HUMAN = ""
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

PLAYERS = (HUMAN, HUMAN)

# ── Colors ────────────────────────────────────────────────────────────────────
class C:
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

def c(color, text):
    return f"{color}{text}{C.RESET}"

def bold(color, text):
    return f"{C.BOLD}{color}{text}{C.RESET}"

# ── Board ─────────────────────────────────────────────────────────────────────
def init():
    board = (".......\n" * 6)
    with open(CURRENT_STATE, 'w') as f:
        f.write(board)
    with open(TEMP_STATE, 'w') as f:
        f.write(board)

def print_colored_board(text):
    col_header = "  " + " ".join(c(C.DIM, str(i)) for i in range(1, 8))
    border     = c(C.BOARD, "┌" + "─" * 15 + "┐")
    border_bot = c(C.BOARD, "└" + "─" * 15 + "┘")

    print()
    print(col_header)
    print(border)

    for row in text.strip().split("\n"):
        rendered = ""
        for ch in row:
            if ch == 'X':
                rendered += bold(C.X, "X") + c(C.BOARD, " ")
            elif ch == 'O':
                rendered += bold(C.O, "O") + c(C.BOARD, " ")
            else:
                rendered += c(C.BOARD, "· ")
        print(c(C.BOARD, "│ ") + rendered.rstrip() + c(C.BOARD, "│"))

    print(border_bot)
    print()

def update_and_print_state():
    subprocess.run([f"cp {TEMP_STATE} {CURRENT_STATE}"], shell=True)
    with open(CURRENT_STATE) as infile:
        state = infile.read()
    print_colored_board(state)

# ── Setup ─────────────────────────────────────────────────────────────────────
def section(title):
    bar = c(C.MENU, "─" * 38)
    print(f"\n{bar}")
    print(bold(C.MENU, f"  {title}"))
    print(f"{bar}\n")

def choose_game_mode():
    section("Game Mode")
    print(f"  {c(C.MENU, '1.')} Human vs Human")
    print(f"  {c(C.MENU, '2.')} Human vs Bot")
    print(f"  {c(C.MENU, '3.')} Bot vs Bot\n")
    game_mode = input(c(C.PROMPT, "  Choose game mode [1-3]: "))
    while game_mode not in ['1', '2', '3']:
        print(c(C.ERROR, "  Invalid choice. Please enter 1, 2, or 3."))
        game_mode = input(c(C.PROMPT, "  Choose game mode [1-3]: "))
    print(c(C.INFO, "\n  Game mode chosen.\n"))
    return int(game_mode)

def list_bots():
    return sorted(
        f for f in os.listdir(BIN)
        if f not in ('make-move', '.gitkeep')
        and os.path.isfile(os.path.join(BIN, f))
        and os.access(os.path.join(BIN, f), os.X_OK)
    )

def get_bot():
    available = list_bots()
    if not available:
        print(c(C.ERROR, "  No bots found in bin/. Run 'make' first."))
        exit(1)

    section("Select Bot")
    for i, name in enumerate(available, 1):
        print(f"  {c(C.MENU, str(i) + '.')} {name}")
    print()

    choice = input(c(C.PROMPT, "  Pick a bot (number): ")).strip()
    while not choice.isdigit() or not (1 <= int(choice) <= len(available)):
        print(c(C.ERROR, f"  Please enter a number between 1 and {len(available)}."))
        choice = input(c(C.PROMPT, "  Pick a bot (number): ")).strip()

    return os.path.join(BIN, available[int(choice) - 1])

def get_players(game_mode):
    if game_mode == 1:
        return (HUMAN, HUMAN)
    if game_mode == 3:
        return (get_bot(), get_bot())

    print()
    first = input(c(C.PROMPT, "  Do you want to play first? [y/n]: "))
    while first not in ['y', 'Y', 'n', 'N']:
        print(c(C.ERROR, "  Please enter y or n."))
        first = input(c(C.PROMPT, "  Do you want to play first? [y/n]: "))

    if first in ('y', 'Y'):
        return (HUMAN, get_bot())
    return (get_bot(), HUMAN)

# ── Gameplay ──────────────────────────────────────────────────────────────────
def make_move(move):
    if not move.isdigit():
        return INVALID

    result = subprocess.run(
        [f"{MAKE_MOVE} {int(move)} < {CURRENT_STATE} > {TEMP_STATE}"],
        shell=True, capture_output=True, text=True
    )
    return result.stderr.strip()

def play_round_by_human(player_id):
    piece_color = C.X if player_id == 0 else C.O
    prompt = c(C.PROMPT, "  Enter column ") + bold(piece_color, f"[1-7]") + c(C.PROMPT, ": ")

    move = input(prompt)
    result = make_move(move)

    while result == INVALID:
        if not move.isdigit():
            print(c(C.ERROR, f"  \"{move}\" is not a valid column — please enter a number from 1 to 7."))
        elif not (1 <= int(move) <= 7):
            print(c(C.ERROR, f"  Column {move} is out of range — please enter a number from 1 to 7."))
        else:
            print(c(C.ERROR, f"  Column {move} is full — choose a different column."))
        move = input(prompt)
        result = make_move(move)

    return result

def play_round_by_bot(player, player_id):
    result = subprocess.run([f"{player} < {CURRENT_STATE}"], shell=True, capture_output=True, text=True)
    move = result.stdout.strip()
    if result.returncode == 0:
        print(c(C.INFO, f"  Bot plays column {move}"))
        return make_move(move)
    return WINS[player_id ^ 1]

def play_round(player, player_id):
    piece_color = C.X if player_id == 0 else C.O
    label = bold(piece_color, PIECES[player_id])
    kind  = "Human" if player == HUMAN else "Bot"
    print(c(C.INFO, f"  Player ") + label + c(C.INFO, f" ({kind}) to move"))

    if player == HUMAN:
        return play_round_by_human(player_id)
    return play_round_by_bot(player, player_id)

def announce_result(result):
    print()
    if result == "WIN X":
        print(bold(C.WIN, "  ★  Player X wins!  ★"))
    elif result == "WIN O":
        print(bold(C.WIN, "  ★  Player O wins!  ★"))
    else:
        print(bold(C.WIN, "  It's a draw!"))
    print()

# ── Main ──────────────────────────────────────────────────────────────────────
def start_game():
    init()
    game_mode = choose_game_mode()
    players = get_players(game_mode)
    global PLAYERS
    PLAYERS = players

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

start_game()
