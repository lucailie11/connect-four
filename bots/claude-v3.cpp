#include <bits/stdc++.h>

using namespace std;
using namespace std::chrono;

const int TIME_LIMIT = 950;
const int ROWS = 6;
const int COLS = 7;
const int WIN = 4;
const int FREE = 0;
const int X = 1;
const int O = 2;
const int WIN_SCORE = 1000000;
const int MAX_DEPTH = 100;
const int INF = 1e9;

int board[ROWS][COLS];
int height[COLS];
int player;
int bestMoveGlobal;
int maxDepth;
bool timeUp;

steady_clock::time_point startTime;

void startTimer() {
    startTime = steady_clock::now();
    timeUp = false;
}

inline int getTime() {
    return (int)duration_cast<milliseconds>(steady_clock::now() - startTime).count();
}

bool canPlace(int c) { return height[c] < ROWS; }

int place(int c, int p) {
    int r = height[c];
    board[r][c] = p;
    height[c]++;
    return r;
}

void unplace(int c) {
    height[c]--;
    board[height[c]][c] = FREE;
}

bool checkWin(int r, int c) {
    int p = board[r][c];
    int cnt;

    cnt = 1;
    for (int d = 1; c + d < COLS && board[r][c + d] == p; d++) cnt++;
    for (int d = 1; c - d >= 0 && board[r][c - d] == p; d++) cnt++;
    if (cnt >= WIN) return true;

    cnt = 1;
    for (int d = 1; r + d < ROWS && board[r + d][c] == p; d++) cnt++;
    for (int d = 1; r - d >= 0 && board[r - d][c] == p; d++) cnt++;
    if (cnt >= WIN) return true;

    cnt = 1;
    for (int d = 1; r + d < ROWS && c + d < COLS && board[r + d][c + d] == p; d++) cnt++;
    for (int d = 1; r - d >= 0 && c - d >= 0 && board[r - d][c - d] == p; d++) cnt++;
    if (cnt >= WIN) return true;

    cnt = 1;
    for (int d = 1; r + d < ROWS && c - d >= 0 && board[r + d][c - d] == p; d++) cnt++;
    for (int d = 1; r - d >= 0 && c + d < COLS && board[r - d][c + d] == p; d++) cnt++;
    return cnt >= WIN;
}

int scoreWindow(int w[], int p) {
    int opp = (p == X ? O : X);
    int pc = 0, oc = 0;
    for (int i = 0; i < WIN; i++) {
        if (w[i] == p) pc++;
        else if (w[i] == opp) oc++;
    }
    if (oc > 0) return 0;
    if (pc == 4) return WIN_SCORE;
    if (pc == 3) return 100;
    if (pc == 2) return 5;
    return 0;
}

int eval() {
    int opp = (player == X ? O : X);
    int score = 0;
    int w[WIN];

    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c <= COLS - WIN; c++) {
            for (int k = 0; k < WIN; k++) w[k] = board[r][c + k];
            score += scoreWindow(w, player) - scoreWindow(w, opp);
        }

    for (int c = 0; c < COLS; c++)
        for (int r = 0; r <= ROWS - WIN; r++) {
            for (int k = 0; k < WIN; k++) w[k] = board[r + k][c];
            score += scoreWindow(w, player) - scoreWindow(w, opp);
        }

    for (int r = 0; r <= ROWS - WIN; r++)
        for (int c = 0; c <= COLS - WIN; c++) {
            for (int k = 0; k < WIN; k++) w[k] = board[r + k][c + k];
            score += scoreWindow(w, player) - scoreWindow(w, opp);
        }

    for (int r = 0; r <= ROWS - WIN; r++)
        for (int c = WIN - 1; c < COLS; c++) {
            for (int k = 0; k < WIN; k++) w[k] = board[r + k][c - k];
            score += scoreWindow(w, player) - scoreWindow(w, opp);
        }

    for (int r = 0; r < ROWS; r++) {
        if (board[r][3] == player) score += 6;
        else if (board[r][3] == opp) score -= 6;
        if (board[r][2] == player || board[r][4] == player) score += 2;
        else if (board[r][2] == opp || board[r][4] == opp) score -= 2;
    }

    return score;
}

// Order moves by static score of immediate placement (best first for maximizer)
vector<int> generateMoves(int curPlayer) {
    vector<pair<int, int>> scored;
    for (int c = 0; c < COLS; c++) {
        if (!canPlace(c)) continue;
        int r = place(c, curPlayer);
        int s = (curPlayer == player ? eval() : -eval());
        unplace(c);
        scored.push_back({s, c});
    }
    sort(scored.rbegin(), scored.rend());
    vector<int> moves;
    for (auto& p : scored) moves.push_back(p.second);
    return moves;
}

int alphabeta(int depth, int alpha, int beta, int curPlayer, int lastR, int lastC) {
    if (timeUp) return 0;
    if (depth % 4 == 0 && getTime() > TIME_LIMIT) {
        timeUp = true;
        return 0;
    }

    int prevPlayer = (curPlayer == X ? O : X);

    if (lastR >= 0 && checkWin(lastR, lastC)) {
        int mult = (prevPlayer == player ? 1 : -1);
        return mult * (WIN_SCORE + depth);
    }

    if (depth == 0) return eval();

    vector<int> moves = generateMoves(curPlayer);
    if (moves.empty()) return 0;

    if (curPlayer == player) {
        int best = -INF;
        for (int c : moves) {
            int r = place(c, curPlayer);
            int score = alphabeta(depth - 1, alpha, beta, prevPlayer, r, c);
            unplace(c);
            if (timeUp) return 0;
            if (score > best) {
                best = score;
                if (depth == maxDepth) bestMoveGlobal = c;
            }
            alpha = max(alpha, best);
            if (alpha >= beta) break;
        }
        return best;
    } else {
        int best = INF;
        for (int c : moves) {
            int r = place(c, curPlayer);
            int score = alphabeta(depth - 1, alpha, beta, player, r, c);
            unplace(c);
            if (timeUp) return 0;
            best = min(best, score);
            beta = min(beta, best);
            if (alpha >= beta) break;
        }
        return best;
    }
}

int main() {
    startTimer();

    player = X;
    for (int r = ROWS - 1; r >= 0; r--) {
        for (int c = 0; c < COLS; c++) {
            char ch;
            cin >> ch;
            board[r][c] = (ch == '.' ? FREE : (ch == 'X' ? X : O));
            if (board[r][c] != FREE) {
                player ^= X ^ O;
                height[c]++;
            }
        }
    }

    // Default to center column
    bestMoveGlobal = 3;
    for (int c = 3; c >= 0 && c < COLS; c = (c == 3 ? 2 : c - 1))
        if (canPlace(c)) { bestMoveGlobal = c; break; }
    for (int c = 0; c < COLS; c++)
        if (canPlace(c)) { bestMoveGlobal = c; break; }

    int move = bestMoveGlobal;
    maxDepth = 1;
    while (!timeUp) {
        alphabeta(maxDepth, -INF, INF, player, -1, -1);
        if (!timeUp) move = bestMoveGlobal;
        maxDepth++;
    }

    cout << move + 1 << "\n";
    return 0;
}
