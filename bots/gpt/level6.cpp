
#include <bits/stdc++.h>
using namespace std;
using u64 = unsigned long long;
using namespace chrono;

static const int W = 7, H = 6;
static const int TIME_LIMIT_MS = 1000;
static const int64_t WIN_BASE = 1<<20;
static const int TT_SIZE = 1<<20;
static const int TT_MASK = TT_SIZE - 1;

struct TTEntry { u64 key; int32_t val; int8_t dep; };
static TTEntry TT[TT_SIZE];
u64 Z[W*H][2];
int moveOrder[W] = {3,2,4,1,5,0,6};
steady_clock::time_point startTime;
int64_t nodes = 0;

// --- Helpers ------------------------------------------------
inline bool timedOut() {
    return duration_cast<milliseconds>(steady_clock::now() - startTime).count() > TIME_LIMIT_MS;
}
inline bool isWin(u64 b) {
    return (b & (b>>1) & (b>>2) & (b>>3)) ||
           (b & (b>>7) & (b>>14)& (b>>21))||
           (b & (b>>6) & (b>>12)& (b>>18))||
           (b & (b>>8) & (b>>16)& (b>>24));
}
inline int colH(u64 mask, int c) {
    for(int r=0;r<H;++r)
        if(!(mask & (1ULL<<(c*7 + r))))
            return r;
    return -1;
}
int64_t evaluate(u64 X, u64 O) {
    static const int dirs[4]={1,7,6,8};
    int64_t s=0;
    for(int c=0;c<W;++c)for(int r=0;r<H;++r){
        int idx=c*7+r; u64 bit=1ULL<<idx;
        if(X&bit) s += (7-abs(3-c));
        else if(O&bit) s -= (7-abs(3-c));
        for(int d:dirs){
            int cx=0,co=0,emp=0;
            for(int i=0;i<4;++i){
                int p = idx + i*d;
                int pc=p/7, pr=p%7;
                if(pc<0||pc>=W||pr<0||pr>=H){ cx=co=5; break; }
                u64 b2=1ULL<<p;
                if(X&b2) ++cx;
                else if(O&b2) ++co;
                else ++emp;
            }
            if(cx>0&&co>0) continue;
            if(cx==3&&emp==1) s+=400;
            if(cx==2&&emp==2) s+=50;
            if(co==3&&emp==1) s-=400;
            if(co==2&&emp==2) s-=50;
        }
    }
    return s;
}
inline u64 hashBoard(u64 X, u64 O) {
    u64 h=0, t=X;
    while(t) { int b=__builtin_ctzll(t); h ^= Z[b][0]; t &= t-1; }
    t = O;
    while(t) { int b=__builtin_ctzll(t); h ^= Z[b][1]; t &= t-1; }
    return h;
}
inline bool ttLookup(u64 k,int d,int32_t &o) {
    int i = k & TT_MASK; auto &e = TT[i];
    if(e.key==k && e.dep>=d){ o=e.val; return true; }
    return false;
}
inline void ttStore(u64 k,int d,int32_t v) {
    int i = k & TT_MASK;
    TT[i] = {k, v, (int8_t)d};
}

// return true if `playerIsX` has ≥2 immediate winning moves
bool hasFork(u64 X, u64 O, bool playerIsX) {
    u64 m = X|O; int threats=0;
    for(int c:moveOrder) {
        int h=colH(m,c);
        if(h<0) continue;
        u64 nb = 1ULL<<(c*7+h);
        if(playerIsX) {
            if(isWin(X|nb) && ++threats>=2) return true;
        } else {
            if(isWin(O|nb) && ++threats>=2) return true;
        }
    }
    return false;
}

// negamax w/ PVS + TT + fork-detect + win-depth scoring
int32_t negamax(u64 X,u64 O,int depth,int32_t alpha,int32_t beta,int ply){
    if(timedOut()|| ++nodes > 50000000) return 0;
    bool xTurn = ((__builtin_popcountll(X|O)&1)==0);
    // if side to move already has a fork → immediate win
    if(hasFork(X,O, xTurn))
        return (xTurn?1:-1)*(WIN_BASE>>(ply-1));

    u64 key = hashBoard(X,O);
    int32_t tv;
    if(ttLookup(key, depth, tv)) return tv;

    if(isWin(O)) return -(WIN_BASE>>(ply-1));
    if(depth==0) return evaluate(X,O);

    u64 m = X|O;
    int32_t best = -INT_MAX;
    bool first=true;

    for(int c:moveOrder) {
        int h=colH(m,c);
        if(h<0) continue;
        u64 nb = 1ULL<<(c*7+h);
        u64 X2=X, O2=O;
        if(xTurn) X2|=nb; else O2|=nb;
        // prune moves that give opponent a fork
        if(hasFork(X2,O2, !xTurn)) continue;

        int32_t val;
        if(xTurn){
            if(isWin(X2)) val = WIN_BASE>>(ply-1);
            else if(first)
                val = -negamax(O2,X2,depth-1,-beta,-alpha,ply+1);
            else {
                val = -negamax(O2,X2,depth-1,-alpha-1,-alpha,ply+1);
                if(val>alpha && val<beta)
                    val = -negamax(O2,X2,depth-1,-beta,-val,ply+1);
            }
        } else {
            if(isWin(O2)) val = -(WIN_BASE>>(ply-1));
            else if(first)
                val = -negamax(X2,O2,depth-1,-beta,-alpha,ply+1);
            else {
                val = -negamax(X2,O2,depth-1,-alpha-1,-alpha,ply+1);
                if(val>alpha && val<beta)
                    val = -negamax(X2,O2,depth-1,-beta,-val,ply+1);
            }
        }
        first=false;
        best = max(best, val);
        alpha = max(alpha, val);
        if(alpha>=beta) break;
    }

    ttStore(key, depth, best);
    return best;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // init Zobrist
    mt19937_64 rng(123456);
    for(int i=0;i<W*H;i++){
        Z[i][0] = rng();
        Z[i][1] = rng();
    }

    // read state.in
    vector<string> rows(6);
    for(int i=0;i<6;i++) cin>>rows[i];
    u64 X=0,O=0;
    for(int r=0;r<6;r++)for(int c=0;c<7;c++){
        char ch = rows[5-r][c];
        if(ch=='X') X |= 1ULL<<(c*7+r);
        if(ch=='O') O |= 1ULL<<(c*7+r);
    }

    u64 mask = X|O;
    bool xTurn = ((__builtin_popcountll(mask)&1)==0);

    // immediate win/block
    for(int c:moveOrder){
        int h=colH(mask,c);
        if(h<0) continue;
        u64 nb=1ULL<<(c*7+h);
        if(xTurn && isWin(X|nb)){ ofstream("move.in")<<c+1; return 0; }
        if(!xTurn && isWin(O|nb)){ ofstream("move.in")<<c+1; return 0; }
    }

    // root move list, prune fork‐allowing moves
    vector<int> moves;
    for(int c:moveOrder){
        int h=colH(mask,c);
        if(h<0) continue;
        u64 X2=X,O2=O;
        u64 nb=1ULL<<(c*7+h);
        if(xTurn) X2|=nb; else O2|=nb;
        if(!hasFork(X2,O2, !xTurn)) moves.push_back(c);
    }
    if(moves.empty())
        for(int c:moveOrder) if(colH(mask,c)>=0) moves.push_back(c);

    startTime = steady_clock::now();
    int bestMove=-1;
    int32_t bestScore=-INT_MAX;

    // iterative deepening
    for(int depth=1; depth<=20; ++depth){
        if(timedOut()) break;
        nodes=0;
        bool improved=false;
        for(int c:moves){
            if(timedOut()) break;
            int h=colH(mask,c);
            u64 X2=X, O2=O;
            u64 nb=1ULL<<(c*7+h);
            if(xTurn) X2|=nb; else O2|=nb;

            int32_t sc = xTurn
                ? -negamax(O2,X2,depth-1,-INT_MAX,INT_MAX,2)
                : -negamax(X2,O2,depth-1,-INT_MAX,INT_MAX,2);
            if(timedOut()) break;
            if(sc>bestScore){
                bestScore=sc;
                bestMove=c;
                improved=true;
            }
        }
        if(!improved) break;
    }

    if(bestMove>=0) cout<<bestMove+1;
    else {
        for(int c:moveOrder)
            if(colH(mask,c)>=0){ cout<<c+1; break; }
    }
    return 0;
}

