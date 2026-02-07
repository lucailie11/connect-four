
#include <bits/stdc++.h>
using namespace std;
using u64 = unsigned long long;
using namespace chrono;

static const int W = 7, H = 6;
static const int TIME_LIMIT_MS = 1000;
static mt19937_64 rng(1337);

// --- Bitboard-based Connect Four representation ---
struct Board {
    u64 X = 0, O = 0;

    // Drop a disc into column c for X if xTurn, else for O.
    bool play(int c, bool xTurn) {
        u64 m = X | O;
        for(int r = 0; r < H; ++r) {
            int idx = c*7 + r;
            if(!(m & (1ULL << idx))) {
                if(xTurn) X |= (1ULL << idx);
                else      O |= (1ULL << idx);
                return true;
            }
        }
        return false; // column full
    }

    // Check if bitboard b has a connect-four
    static bool win(u64 b) {
        // vertical
        if(b & (b>>1) & (b>>2) & (b>>3)) return true;
        // horizontal
        if(b & (b>>7)& (b>>14)& (b>>21)) return true;
        // diagonal /
        if(b & (b>>6)& (b>>12)& (b>>18)) return true;
        // diagonal \
        if(b & (b>>8)& (b>>16)& (b>>24)) return true;
        return false;
    }

    // +1 if X wins, -1 if O wins, 0 otherwise
    int result() const {
        if(win(X)) return +1;
        if(win(O)) return -1;
        return 0;
    }

    // List of non-full columns
    vector<int> legalMoves() const {
        vector<int> mv;
        u64 m = X | O;
        for(int c = 0; c < W; ++c) {
            // top cell index = c*7 + (H-1)
            if(!(m & (1ULL << (c*7 + (H-1))))) {
                mv.push_back(c);
            }
        }
        return mv;
    }

    // Load board from "state.in"
    static Board load(const char* path = "state.in") {
        ifstream cin(path);
        vector<string> rows(6);
        for(int i = 0; i < 6; ++i) {
            cin >> rows[i];
        }
        Board b;
        // rows[0] is top; bitboard row 0 is bottom
        for(int r = 0; r < 6; ++r) {
            for(int c = 0; c < 7; ++c) {
                char ch = rows[5 - r][c];
                if(ch == 'X') b.X |= (1ULL << (c*7 + r));
                if(ch == 'O') b.O |= (1ULL << (c*7 + r));
            }
        }
        return b;
    }
};

// Monte Carlo parameters
static const int MC_TIME_BUFFER = 20; // ms to spare at the end

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // 1) Load the current position
    Board root = Board::load();
    u64 m = root.X | root.O;
    bool xTurn = (__builtin_popcountll(m) % 2 == 0);

    // 2) Immediate win or block
    auto leg = root.legalMoves();
    for(int c : leg) {
        Board b = root;
        b.play(c, xTurn);
        if(b.result() == (xTurn ? +1 : -1)) {
            cout << (c + 1);
            return 0;
        }
    }
    for(int c : leg) {
        Board b = root;
        b.play(c, !xTurn);
        if(b.result() == (xTurn ? -1 : +1)) {
            cout << (c + 1);
            return 0;
        }
    }

    // 3) Monte Carlo playouts
    auto start = steady_clock::now();
    int N = leg.size();
    vector<int> wins(N, 0), plays(N, 0);

    while(true) {
        auto now = steady_clock::now();
        int elapsed = (int)duration_cast<milliseconds>(now - start).count();
        if(elapsed > TIME_LIMIT_MS - MC_TIME_BUFFER) break;

        // iterate through each candidate move
        for(int i = 0; i < N; ++i) {
            Board b = root;
            bool turn = xTurn;
            b.play(leg[i], turn);
            turn = !turn;

            // simulate until win or draw
            int res = b.result();
            while(res == 0) {
                auto moves = b.legalMoves();
                if(moves.empty()) break; // draw
                uniform_int_distribution<int> dist(0, moves.size() - 1);
                int mv = moves[dist(rng)];
                b.play(mv, turn);
                turn = !turn;
                res = b.result();
            }

            // record result
            if(res == +1 && xTurn)   wins[i]++;
            else if(res == -1 && !xTurn) wins[i]++;
            plays[i]++;
        }
    }

    // 4) Pick the move with highest win rate
    int bestIdx = 0;
    double bestRate = -1.0;
    for(int i = 0; i < N; ++i) {
        if(plays[i] == 0) continue;
        double rate = double(wins[i]) / plays[i];
        if(rate > bestRate) {
            bestRate = rate;
            bestIdx = i;
        }
    }

    // 5) Output the chosen move (1-based)
    cout << (leg[bestIdx] + 1);
    return 0;
}

