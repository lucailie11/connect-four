#include <bits/stdc++.h>

using namespace std;

const int ROWS = 6, COLS = 7;
const string VALID = "VALID";
const string INVALID = "INVALID";
const string DRAW = "DRAW";
const string WIN_X = "WIN X";
const string WIN_O = "WIN O";

bool is_valid_move(const vector<string> &board, int col) {
  return col >= 0 && col < COLS && board[0][col] == '.';
}

bool apply_move(vector<string> &board, int col, char player, int &row_played) {
  for (int r = ROWS - 1; r >= 0; --r) {
    if (board[r][col] == '.') {
      board[r][col] = player;
      row_played = r;
      return true;
    }
  }
  return false;
}

bool check_win(const vector<string> &b, int r, int c, char p) {
  const vector<pair<int, int>> dir = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
  for (auto [dr, dc] : dir) {
    int cnt = 1;
    for (int d : {-1, 1}) {
      int nr = r + d * dr, nc = c + d * dc;
      while (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS && b[nr][nc] == p)
        cnt++, nr += d * dr, nc += d * dc;
    }
    if (cnt >= 4)
      return true;
  }
  return false;
}

bool is_draw(const vector<string> &board) {
  for (int c = 0; c < COLS; ++c)
    if (board[0][c] == '.')
      return false;
  return true;
}

char current_player(const vector<string> &board) {
  int x = 0, o = 0;
  for (auto &row : board)
    for (char c : row)
      x += (c == 'X'), o += (c == 'O');
  return (x <= o ? 'X' : 'O');
}

int main(int argc, char *argv[]) {
  assert(argc == 2);
  int col = atoi(argv[1]) - 1;

  vector<string> board(ROWS);
  for (int i = 0; i < ROWS; ++i)
    cin >> board[i];

  char player = current_player(board);

  if (!is_valid_move(board, col)) {
    cerr << INVALID << "\n";
    return 0;
  }

  int row;
  apply_move(board, col, player, row);

  for (auto &row_str : board)
    cout << row_str << '\n';

  if (check_win(board, row, col, player)) {
    string win = (player == 'X' ? WIN_X : WIN_O);
    cerr << win << "\n";
  } else if (is_draw(board))
    cerr << DRAW << "\n";
  else
    cerr << VALID << "\n";

  return 0;
}
