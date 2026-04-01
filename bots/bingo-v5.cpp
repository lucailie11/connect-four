#include <bits/stdc++.h>

// #define DEBUG

using namespace std;
using namespace std::chrono;

const int TIME_LIMIT = 950;
const int SEED = 10930;
const int ROWS = 6;
const int COLS = 7;
const int WIN = 4;
const int DIAGS = ROWS + COLS - 1;
const int COL_CONFIG = 729;
const int ROW_CONFIG = 2187;
const int DIAG_CONFIG = 729;
const int TYPES = 3;
const int FREE = 0;
const int X = 1;
const int O = 2;
const int MAX_VAL = (1 << 18) - 1;
const int MAX_DEPTH = 100;
const int DEPTHS_WITH_CENTER_ORDER = 2;
const long long INF = 1e12;

/*
 * POINT CONSTANTS
 */

const int WIN_POINTS = 1e6;
const int POINTS_FOR_WIN = WIN_POINTS / 2;
const int SCORE_COL_X_IN_A_ROW[ROWS + 1] = {
    1, 5, 20, 400, WIN_POINTS, 2 * WIN_POINTS, 3 * WIN_POINTS};
const int SCORE_ROW_POTENTIAL_4_USING_X[WIN + 1] = {1, 5, 60, 2000, WIN_POINTS};
const int SCORE_DIAG_POTENTIAL_4_USING_X[WIN + 1] = {1, 5, 20, 400, WIN_POINTS};
int coefDepth[MAX_DEPTH + 1];

void initCoefDepth() {
  for (int i = 0; i <= MAX_DEPTH; i++)
    coefDepth[i] = MAX_DEPTH + 1 - i;
}

mt19937 rnd(SEED);

bool timeUp;
int player;
int depth, maxDepth;
long long stateGlobal;
long long scoreGlobal;
int bestMoveOverall;
int height[COLS];
int p3[COLS + 1];
int initState[ROWS][COLS], crtState[ROWS][COLS];
int stateRow[ROWS], stateCol[COLS], stateDiag1[DIAGS], stateDiag2[DIAGS];
long long scoreRow[ROW_CONFIG], scoreCol[COL_CONFIG],
    scoreDiag[DIAGS][DIAG_CONFIG];
long long xorHash[ROWS][COLS][TYPES];

using Clock = chrono::steady_clock;
using TimePoint = Clock::time_point;

steady_clock::time_point startTime;

void startTimer() {
  startTime = steady_clock::now();
  timeUp = false;
}

inline int getTime() {
  steady_clock::time_point crtTime = steady_clock::now();
  milliseconds elapsed = duration_cast<milliseconds>(crtTime - startTime);
  int ms = static_cast<int>(elapsed.count());
  return ms;
}

struct HASH_TABLE {
  int sz = 0;
  vector<pair<long long, long long>> hashTable[MAX_VAL + 1];

  void clear() {
    for (int i = 0; i <= MAX_VAL; i++)
      hashTable[i].clear();
    sz = 0;
  }

  int getHash(long long key) { return key & MAX_VAL; }

  void insert(long long key, long long val) {
    int hashh = getHash(key);
    vector<pair<long long, long long>> &v = hashTable[hashh];

    int pos = 0;
    while (pos < (int)v.size() && v[pos].first < key)
      pos++;

    sz++;
    v.push_back({0, 0});
    for (int i = v.size() - 1; i > pos; i--)
      v[i] = v[i - 1];
    v[pos] = {key, val};
  }

  bool find(long long key) {
    int hashh = getHash(key);
    vector<pair<long long, long long>> &v = hashTable[hashh];

    pair<long long, long long> searched = {key, LONG_LONG_MIN};
    int pos = lower_bound(v.begin(), v.end(), searched) - v.begin();
    if (pos < (int)v.size() && v[pos].first == key)
      return true;
    return false;
  }

  long long get(long long key) {
    int hashh = getHash(key);
    vector<pair<long long, long long>> &v = hashTable[hashh];

    pair<long long, long long> searched = {key, LONG_LONG_MIN};
    int pos = lower_bound(v.begin(), v.end(), searched) - v.begin();
    assert(pos < (int)v.size() && v[pos].first == key);

    return v[pos].second;
  }
};

HASH_TABLE scoreStates, prevScoreStates;

void initP3() {
  p3[0] = 1;
  for (int i = 1; i <= COLS; i++)
    p3[i] = p3[i - 1] * 3;
}

void initXorHash() {
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      for (int a = 0; a < TYPES; a++)
        xorHash[r][c][a] = (long long)rnd() * INT_MAX + rnd();
    }
  }
}

void initScoreCol() {
  for (int state = 0; state < p3[ROWS]; state++) {
    vector<int> v(ROWS);
    for (int i = 0; i < ROWS; i++)
      v[i] = state / p3[i] % 3;

    vector<pair<int, int>> secv;
    int cons = 1;
    for (int i = 1; i < ROWS; i++) {
      if (v[i] == v[i - 1])
        cons++;
      else {
        secv.push_back({cons, v[i - 1]});
        cons = 1;
      }
    }
    secv.push_back({cons, v.back()});

    for (auto c : secv) {
      if (c.second == FREE)
        continue;
      if (c.first >= 4)
        scoreCol[state] +=
            (c.second == X ? 1 : -1) * (c.first - 3) * WIN_POINTS;
    }

    if (v[ROWS - 1] != FREE || v[0] == FREE)
      continue;

    int b = secv[secv.size() - 2].first, p = secv[secv.size() - 2].second;
    scoreCol[state] += (p == X ? 1 : -1) * SCORE_COL_X_IN_A_ROW[b];
  }
}

void initScoreRow() {
  for (int state = 0; state < p3[COLS]; state++) {
    vector<int> v(COLS);
    for (int i = 0; i < COLS; i++)
      v[i] = state / p3[i] % 3;

    for (int p : {X, O}) {
      for (int i = 0; i <= COLS - WIN; i++) {
        bool possible = true;
        int pp = 0;
        for (int j = i; j < i + WIN; j++) {
          possible &= (v[j] != (p ^ X ^ O));
          pp += (v[j] == p);
        }
        if (!possible)
          continue;

        scoreRow[state] +=
            (p == X ? 1 : -1) * SCORE_ROW_POTENTIAL_4_USING_X[pp];
      }
    }
  }
}

void initScoreDiag() {
  for (int d = 0; d < DIAGS; d++) {
    int len = min(d + 1, DIAGS - d);
    int firstRow = (d <= COLS ? 0 : d - COLS);
    for (int state = 0; state < p3[ROWS]; state++) {
      vector<int> v(len);
      bool bad = false;
      for (int i = 0; i < ROWS; i++) {
        int s = state / p3[i] % 3;
        if (i < firstRow || i >= firstRow + len)
          bad |= (s != FREE);
        else
          v[i - firstRow] = state / p3[i] % 3;
      }

      if (bad)
        continue;

      vector<int> playerss = {X, O};
      for (int p : playerss) {
        for (int i = 0; i <= len - WIN; i++) {
          bool possible = true;
          int pp = 0;
          for (int j = i; j < i + WIN; j++) {
            possible &= (v[j] != (p ^ X ^ O));
            pp += (v[j] == p);
          }

          if (!possible)
            continue;

          scoreDiag[d][state] +=
              (p == X ? 1 : -1) * SCORE_DIAG_POTENTIAL_4_USING_X[pp];
        }
      }
    }
  }
}

void readInput() {
  stateGlobal = 0;
  player = X;
  for (int r = ROWS - 1; r >= 0; r--) {
    for (int c = 0; c < COLS; c++) {
      char ch;
      cin >> ch;
      initState[r][c] = (ch == '.' ? FREE : (ch == 'X' ? X : O));
      if (initState[r][c] != FREE) {
        player ^= X ^ O;
        height[c]++;
      }

      int d1 = r + c, d2 = r - c + COLS - 1;
      stateGlobal ^= xorHash[r][c][initState[r][c]];
      stateRow[r] += p3[c] * initState[r][c];
      stateCol[c] += p3[r] * initState[r][c];
      stateDiag1[d1] += p3[r] * initState[r][c];
      stateDiag2[d2] += p3[r] * initState[r][c];

      crtState[r][c] = initState[r][c];
    }
  }

  for (int r = 0; r < ROWS; r++)
    scoreGlobal += scoreRow[stateRow[r]];
  for (int c = 0; c < COLS; c++)
    scoreGlobal += scoreCol[stateCol[c]];
  for (int d = 0; d < DIAGS; d++)
    scoreGlobal += scoreDiag[d][stateDiag1[d]] + scoreDiag[d][stateDiag2[d]];
}

void changeScore(int r, int c, int coef) {
  int d1 = r + c, d2 = r - c + COLS - 1;

  scoreGlobal -= scoreRow[stateRow[r]];
  scoreGlobal -= scoreCol[stateCol[c]];
  scoreGlobal -= scoreDiag[d1][stateDiag1[d1]];
  scoreGlobal -= scoreDiag[d2][stateDiag2[d2]];

  stateRow[r] += coef * (player * p3[c] - FREE * p3[c]);
  stateCol[c] += coef * (player * p3[r] - FREE * p3[r]);
  stateDiag1[d1] += coef * (player * p3[r] - FREE * p3[r]);
  stateDiag2[d2] += coef * (player * p3[r] - FREE * p3[r]);

  scoreGlobal += scoreRow[stateRow[r]];
  scoreGlobal += scoreCol[stateCol[c]];
  scoreGlobal += scoreDiag[d1][stateDiag1[d1]];
  scoreGlobal += scoreDiag[d2][stateDiag2[d2]];
}

vector<int> generateMovesByCenter() {
  vector<int> mvs;
  mvs = {3, 2, 4, 5, 1, 6, 0};
  return mvs;
}

vector<int> generateMovesByStaticScore() {
  vector<pair<long long, int>> moves;
  for (int c = 0; c < COLS; c++) {
    int r = height[c];
    if (r == ROWS)
      continue;

    changeScore(r, c, 1);
    moves.push_back({(player == X ? scoreGlobal : -scoreGlobal), c});
    changeScore(r, c, -1);
  }

  sort(moves.begin(), moves.end());
  reverse(moves.begin(), moves.end());
  vector<int> mvs;
  for (auto p : moves)
    mvs.push_back(p.second);

  return mvs;
}

long long alphabeta(long long alpha, long long beta) {
  if (timeUp)
    return 0;

  if (depth % 4 == 0 && getTime() > TIME_LIMIT) {
    timeUp = true;
    return 0;
  }

  if (depth == maxDepth || abs(scoreGlobal) >= POINTS_FOR_WIN) {
    long long score = (player == X ? 1 : -1) * scoreGlobal;
    score *= coefDepth[depth];
    scoreStates.insert(stateGlobal, score);
    return score;
  }

  if (scoreStates.find(stateGlobal))
    return scoreStates.get(stateGlobal);

  long long maxScore = -INF;
  long long bestMove = 0;
  vector<int> moves = (depth >= maxDepth - DEPTHS_WITH_CENTER_ORDER
                           ? generateMovesByCenter()
                           : generateMovesByStaticScore());
  int options = 0;
  for (int c : moves) {
    int r = height[c];
    if (r == ROWS)
      continue;

    options++;
    stateGlobal ^= xorHash[r][c][FREE] ^ xorHash[r][c][player];
    crtState[r][c] = player;
    changeScore(r, c, 1);

    height[c]++;
    player ^= X ^ O;
    depth++;

    long long nextScore = -alphabeta(alpha, beta);

    if (player == X)
      alpha = max(alpha, nextScore);
    else
      beta = min(beta, -nextScore);

    depth--;
    player ^= X ^ O;
    height[c]--;

    changeScore(r, c, -1);
    crtState[r][c] = FREE;
    stateGlobal ^= xorHash[r][c][FREE] ^ xorHash[r][c][player];

    if (nextScore > maxScore) {
      maxScore = nextScore;
      bestMove = c;
    }
  }
  if (options == 0)
    return 0;

  bestMoveOverall = bestMove;

  scoreStates.insert(stateGlobal, maxScore);

  return maxScore;
}

int main() {
  startTimer();

  initP3();
  initXorHash();
  initCoefDepth();

  initScoreCol();
  initScoreRow();
  initScoreDiag();

  readInput();

  maxDepth = 2;
  int move = 0;
  while (!timeUp) {
    scoreStates.clear();
    alphabeta(-INF, INF);
    if (!timeUp)
      move = bestMoveOverall;
    maxDepth++;
  }

  cout << move + 1 << "\n";

  return 0;
}
