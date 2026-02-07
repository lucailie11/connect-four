#include <bits/stdc++.h>
using namespace std;
using namespace chrono;

const int ROWS = 6, COLS = 7, INF = 1e9;
char ME, OP;
vector<string> board(ROWS);
auto start = steady_clock::now();
const int TIME_LIMIT = 900; // milliseconds

int countAligns(int r, int c, int dr, int dc, char p) {
    int cnt = 0;
    for (int i = 0; i < 4; ++i) {
        int nr = r + i * dr, nc = c + i * dc;
        if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) return 0;
        if (board[nr][nc] != p && board[nr][nc] != '.') return 0;
        if (board[nr][nc] == p) ++cnt;
    }
    return cnt;
}

int eval() {
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c)
            for (auto [dr, dc] : vector<pair<int, int>>{{0,1},{1,0},{1,1},{1,-1}})
                if (countAligns(r, c, dr, dc, OP) == 4) return -INF;

    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c)
            for (auto [dr, dc] : vector<pair<int, int>>{{0,1},{1,0},{1,1},{1,-1}})
                if (countAligns(r, c, dr, dc, ME) == 4) return INF;

    int score = 0;
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c)
            for (auto [dr, dc] : vector<pair<int, int>>{{0,1},{1,0},{1,1},{1,-1}}) {
                int me3 = (countAligns(r, c, dr, dc, ME) == 3);
                int me2 = (countAligns(r, c, dr, dc, ME) == 2);
                int op3 = (countAligns(r, c, dr, dc, OP) == 3);
                int op2 = (countAligns(r, c, dr, dc, OP) == 2);
                score += 100 * me3 + 10 * me2 - 100 * op3 - 10 * op2;
            }
    return score;
}

bool timeout() {
    return duration_cast<milliseconds>(steady_clock::now() - start).count() > TIME_LIMIT;
}

bool play(int c, char p) {
    for (int r = ROWS - 1; r >= 0; --r)
        if (board[r][c] == '.') {
            board[r][c] = p;
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

int alphabeta(int depth, int alpha, int beta, bool maximizing) {
    if (timeout()) return 0;
    int val = eval();
    if (abs(val) == INF || depth == 0) return val;

    if (maximizing) {
        int best = -INF;
        for (int c = 0; c < COLS; ++c) {
            if (board[0][c] != '.') continue;
            play(c, ME);
            best = max(best, alphabeta(depth - 1, alpha, beta, false));
            undo(c);
            alpha = max(alpha, best);
            if (beta <= alpha) break;
        }
        return best;
    } else {
        int best = INF;
        for (int c = 0; c < COLS; ++c) {
            if (board[0][c] != '.') continue;
            play(c, OP);
            best = min(best, alphabeta(depth - 1, alpha, beta, true));
            undo(c);
            beta = min(beta, best);
            if (beta <= alpha) break;
        }
        return best;
    }
}

int main() {
    for (int i = 0; i < ROWS; ++i) cin >> board[i];

    int x = 0, o = 0;
    for (auto& row : board)
        for (char c : row)
            x += (c == 'X'), o += (c == 'O');

    ME = (x <= o ? 'X' : 'O');
    OP = (ME == 'X' ? 'O' : 'X');

    start = steady_clock::now();
    int best_move = -1, best_score = -INF;

    for (int depth = 1; depth <= 10; ++depth) {
        if (timeout()) break;
        for (int c = 0; c < COLS; ++c) {
            if (board[0][c] != '.') continue;
            play(c, ME);
            int score = alphabeta(depth - 1, -INF, INF, false);
            undo(c);
            if (timeout()) break;
            if (score > best_score) {
                best_score = score;
                best_move = c;
            }
        }
    }

    if (best_move != -1)
        cout << (best_move + 1) << '\n';
    else {
        for (int c = 0; c < COLS; ++c)
            if (board[0][c] == '.') {
                cout << (c + 1) << '\n';
                break;
            }
    }

    return 0;
}

