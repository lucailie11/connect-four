#include <bits/stdc++.h>
using namespace std;
using namespace chrono;

const int ROWS = 6, COLS = 7, INF = 1e9;
const int TIME_LIMIT_MS = 1000;

char ME, OP;
vector<string> board(ROWS);
auto start = steady_clock::now();

mt19937_64 rng(42);
uint64_t zob[ROWS][COLS][2]; // 0: X, 1: O
unordered_map<uint64_t, int> transpo;

int center_order[COLS] = {3, 2, 4, 1, 5, 0, 6};

void init_zobrist() {
    for (int i = 0; i < ROWS; ++i)
        for (int j = 0; j < COLS; ++j)
            for (int k = 0; k < 2; ++k)
                zob[i][j][k] = rng();
}

uint64_t compute_hash() {
    uint64_t h = 0;
    for (int i = 0; i < ROWS; ++i)
        for (int j = 0; j < COLS; ++j) {
            if (board[i][j] == 'X') h ^= zob[i][j][0];
            if (board[i][j] == 'O') h ^= zob[i][j][1];
        }
    return h;
}

bool timeout() {
    return duration_cast<milliseconds>(steady_clock::now() - start).count() > TIME_LIMIT_MS;
}

bool play(int c, char p, int &row) {
    for (int r = ROWS - 1; r >= 0; --r)
        if (board[r][c] == '.') {
            board[r][c] = p;
            row = r;
            return true;
        }
    return false;
}

void undo(int c) {
    for (int r = 0; r < ROWS; ++r)
        if (board[r][c] != '.') {
            board[r][c] = '.';
            return;
        }
}

bool check_win(int r, int c, char p) {
    vector<pair<int, int>> dirs = {{0,1}, {1,0}, {1,1}, {1,-1}};
    for (auto [dr, dc] : dirs) {
        int cnt = 1;
        for (int d : {-1, 1}) {
            int nr = r + d*dr, nc = c + d*dc;
            while (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS && board[nr][nc] == p)
                cnt++, nr += d*dr, nc += d*dc;
        }
        if (cnt >= 4) return true;
    }
    return false;
}

int eval_positional() {
    int score = 0;
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c)
            if (board[r][c] == ME)
                score += (3 - abs(3 - c)) * 5;
            else if (board[r][c] == OP)
                score -= (3 - abs(3 - c)) * 5;
    return score;
}

int alphabeta(int depth, int alpha, int beta, bool maximizing, int ply) {
    if (timeout()) return 0;

    uint64_t h = compute_hash();
    if (transpo.count(h)) return transpo[h];

    if (depth == 0) return transpo[h] = eval_positional();

    if (maximizing) {
        int best = -INF;
        for (int c : center_order) {
            if (board[0][c] != '.') continue;
            int r;
            play(c, ME, r);
            if (check_win(r, c, ME)) {
                undo(c);
                return transpo[h] = INF - ply;
            }
            int sc = alphabeta(depth - 1, alpha, beta, false, ply + 1);
            undo(c);
            best = max(best, sc);
            alpha = max(alpha, sc);
            if (beta <= alpha) break;
        }
        return transpo[h] = best;
    } else {
        int best = INF;
        for (int c : center_order) {
            if (board[0][c] != '.') continue;
            int r;
            play(c, OP, r);
            if (check_win(r, c, OP)) {
                undo(c);
                return transpo[h] = -INF + ply;
            }
            int sc = alphabeta(depth - 1, alpha, beta, true, ply + 1);
            undo(c);
            best = min(best, sc);
            beta = min(beta, sc);
            if (beta <= alpha) break;
        }
        return transpo[h] = best;
    }
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    init_zobrist();

    for (int i = 0; i < ROWS; ++i) cin >> board[i];

    int x = 0, o = 0;
    for (auto& row : board)
        for (char c : row)
            x += (c == 'X'), o += (c == 'O');

    ME = (x <= o ? 'X' : 'O');
    OP = (ME == 'X' ? 'O' : 'X');

    start = steady_clock::now();
    int best_move = -1, best_score = -INF;

    // Immediate win or block
    for (int c : center_order) {
        if (board[0][c] != '.') continue;
        int r;
        play(c, ME, r);
        if (check_win(r, c, ME)) {
            cout << (c + 1) << '\n';
            return 0;
        }
        undo(c);
    }
    for (int c : center_order) {
        if (board[0][c] != '.') continue;
        int r;
        play(c, OP, r);
        if (check_win(r, c, OP)) {
            cout << (c + 1) << '\n';
            return 0;
        }
        undo(c);
    }

    // Iterative deepening
    for (int depth = 1; depth <= 42; ++depth) {
        if (timeout()) break;
        for (int c : center_order) {
            if (board[0][c] != '.') continue;
            int r;
            play(c, ME, r);
            int sc = check_win(r, c, ME) ? INF - 1 : alphabeta(depth - 1, -INF, INF, false, 1);
            undo(c);
            if (timeout()) break;
            if (sc > best_score) {
                best_score = sc;
                best_move = c;
            }
        }
    }

    if (best_move != -1)
        cout << (best_move + 1) << '\n';
    else {
        for (int c : center_order)
            if (board[0][c] == '.') {
                cout << (c + 1) << '\n';
                break;
            }
    }

    return 0;
}

