import subprocess
import atexit
import os

HUMAN = ""
PIECES = ['X', 'O']
WINS = ["WIN X", "WIN O"]
VALID = "VALID"
INVALID = "INVALID"
LINES = 30
PLAYERS = (HUMAN, HUMAN)

def cleanup():
    files_to_delete = ["temp-state.txt", "current-state.txt", "make-move"] + list(PLAYERS)
    for file in files_to_delete:
        if os.path.exists(file):
            os.remove(file)

atexit.register(cleanup)

def init():
    subprocess.run(["g++ make-move.cpp -o make-move"], shell=True)
    subprocess.run(["cat > current-state.txt < initial-state.txt"], shell=True)
    subprocess.run(["cat > temp-state.txt < initial-state.txt"], shell=True)

def print_colored_board(text):
    GREEN  = "\033[32m"
    RED    = "\033[31m"
    YELLOW = "\033[35m"
    RESET  = "\033[0m"

    colored_text = YELLOW + text
    colored_text = colored_text.replace("X", f"{GREEN}X{YELLOW}")
    colored_text = colored_text.replace("O", f"{RED}O{YELLOW}")
    print(colored_text + RESET)

def update_and_print_state():
    subprocess.run(["cat > current-state.txt < temp-state.txt"], shell=True)

    with open("current-state.txt") as infile:
        state = infile.read()
        print_colored_board(state)
    print(LINES * '-')

def choose_game_mode():
    print("1. Human vs Human\n2. Human vs Bot\n3. Bot vs Bot")
    game_mode = input("Choose game mode: ")
    while game_mode not in ['1', '2', '3']:
        game_mode = input("Choose game mode: ")
    print("Game mode chosen succesfully\n")
    return int(game_mode)

def get_bot():
    bot = input("Enter bot adress: ")
    print(bot)
    botname = bot.split('/')[-1].split('.')[0]
    result = subprocess.run([f"g++ {bot} -o {botname}"], shell=True)
    while not result.returncode == 0:
        print(result)
        bot = input("Enter bot adress: ")
        botname = bot.split('/')[-1].split('.')[0]
        result = subprocess.run([f"g++ {bot} -o {botname}"], shell=True)
    
    print(f"Succesfully created bot {botname}")

    return botname

def get_players(game_mode):
    if game_mode == 1:
        return (HUMAN, HUMAN)
    if game_mode == 3:
        return (get_bot(), get_bot())

    first = input("Do you want to play first? [y/n]")
    while first not in ['y', 'Y', 'n', 'N']:
        first = input("Do you want to play first? [y/n]")

    if first == 'y' or first == 'Y':
        return (HUMAN, get_bot())
    return (get_bot(), HUMAN)

def make_move(move):
    if not move.isdigit():
        print("Please enter a number")
        return INVALID

    result = subprocess.run([f"./make-move {int(move)} < current-state.txt > temp-state.txt"], shell=True, capture_output=True, text=True)
    return result.stderr.strip()


def play_round_by_human():
    move = input("Enter move (1-7): ")
    result = make_move(move)
    while result == INVALID:
        move = input("Enter move (1-7): ")
        result = make_move(move)
    return result

def play_round_by_bot(player, player_id):
    result = subprocess.run([f"./{player} < current-state.txt"], shell=True, capture_output=True, text=True)
    move = result.stdout.strip()
    if result.returncode == 0:
        return make_move(move)
    return WINS[player_id ^ 1]

def play_round(player, player_id):
    print(f"Player {PIECES[player_id]} is moving")

    if player == HUMAN:
        return play_round_by_human()
    return play_round_by_bot(player, player_id)

def start_game():
    init()
    game_mode = choose_game_mode()
    players = get_players(game_mode)
    global PLAYERS
    PLAYERS = players

    round_num = 0
    result = play_round(players[round_num % 2], round_num % 2)
    while result == VALID:
        update_and_print_state()
        round_num += 1
        result = play_round(players[round_num % 2], round_num % 2)

    update_and_print_state()
    print(result)

start_game()
