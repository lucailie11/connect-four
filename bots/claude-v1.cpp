#include <bits/stdc++.h>

using namespace std;

const int ROWS = 6;
const int COLS = 7;
const int WIN = 4;
const int FREE = 0;
const int X = 1;
const int O = 2;
const int WIN_SCORE = 1000000;

int board[ROWS][COLS];
int height[COLS];
int player;

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
    if (pc == 3) return 50;
    if (pc == 2) return 3;
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

int main() {
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

    int opp = (player == X ? O : X);
    int order[] = {3, 2, 4, 1, 5, 0, 6};

    // Immediate win
    for (int i = 0; i < COLS; i++) {
        int c = order[i];
        if (!canPlace(c)) continue;
        int r = place(c, player);
        if (checkWin(r, c)) { cout << c + 1 << "\n"; return 0; }
        unplace(c);
    }

    // Block opponent win
    for (int i = 0; i < COLS; i++) {
        int c = order[i];
        if (!canPlace(c)) continue;
        int r = place(c, opp);
        if (checkWin(r, c)) { unplace(c); cout << c + 1 << "\n"; return 0; }
        unplace(c);
    }

    // Best heuristic move
    int bestScore = INT_MIN, bestCol = -1;
    for (int i = 0; i < COLS; i++) {
        int c = order[i];
        if (!canPlace(c)) continue;
        place(c, player);
        int score = eval();
        unplace(c);
        if (bestCol == -1 || score > bestScore) {
            bestScore = score;
            bestCol = c;
        }
    }

    cout << bestCol + 1 << "\n";
    return 0;
}
