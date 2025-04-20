// https://atcoder.jp/contests/rco-contest-2019-qual/submissions/4241441
// に「 2-opt で結ぶ辺の片方の距離を平均付近に限定」と「多スタート」を追加したもの

#if 1
#pragma GCC optimize "O3"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <random>
#include <vector>
using namespace std;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;
template <class T>
inline T sq(T a) {
    return a * a;
}
template <class T>
inline bool chmin(T &a, const T &b) {
    if(b < a) {
        a = b;
        return true;
    }
    return false;
}
template <class T>
inline bool chmax(T &a, const T &b) {
    if(a < b) {
        a = b;
        return true;
    }
    return false;
}
void _u() {
    cerr << endl;
}
template <class H, class... T>
void _u(H &&h, T &&...t) {
    cerr << h << ", ";
    _u(move(t)...);
}
#define _polyfill(v)                v
#define _overload3(a, b, c, d, ...) d
#define _rep3(i, a, b)              for(i32 i = i32(a); i < i32(b); ++i)
#define _rep2(i, b)                 _rep3(i, 0, b)
#define _rep1(i)                    for(i32 i = 0;; ++i)
#define rep(...)                    _polyfill(_overload3(__VA_ARGS__, _rep3, _rep2, _rep1)(__VA_ARGS__))
#define A(a)                        begin(a), end(a)
#define S(a)                        i32((a).size())
#define U(...)                        \
    {                                 \
        cerr << #__VA_ARGS__ << ": "; \
        _u(__VA_ARGS__);              \
    }
#define lin U(__LINE__)
#define exi exit(0);
#endif

class timer {
    vector<timer> timers;
    int n = 0;

  public:
    double limit = 1.95;
    double t = 0;
    timer() {
    }
    timer(int size)
        : timers(size) {
    }
    bool elapses() const {
        return time() - t > limit;
    }
    void measure() {
        t = time() - t;
        ++n;
    }
    void measure(char id) {
        timers[id].measure();
    }
    void print() {
        if(n % 2)
            measure();
        for(int i = 0; i < 128; ++i) {
            if(timers[i].n)
                cerr << (char)i << ' ' << timers[i].t << 's' << endl;
        }
        cerr << "  " << t << 's' << endl;
    }
    static double time() {
#ifdef LOCAL
        return __rdtsc() / 3.3e9;
#else
        unsigned long long a, d;
        __asm__ volatile("rdtsc" : "=a"(a), "=d"(d));
        return (d << 32 | a) / 2.8e9;
#endif
    }
} timer(128);

constexpr bool Deterministic = 0;
class rando {
    unsigned y;

  public:
    rando(unsigned y)
        : y(y) {
    }
    unsigned next() {
        return y ^= (y ^= (y ^= y << 13) >> 17) << 5;
    }
    int next(int b) {
        return next() % b;
    }
    int next(int a, int b) {
        return next(b - a) + a;
    }
    double nextDouble(double b = 1) {
        return b * next() / 4294967296.0;
    }
    double nextDouble(double a, double b) {
        return nextDouble(b - a) + a;
    }
    int operator()(int b) {
        return next(b);
    }
} rando(Deterministic ? 2463534242 : random_device()());

constexpr i32 N = 200;
struct {
    f64 distanceDivN;
    f64 distance2DivN;
    i32 penalty;
    f64 penaltyInv;
    f64 distance;
} edges[N][N];
i32 paths[N + 1], bpaths[N + 1];
i32 p2i[N];
f64 av, av2, la;
f64 score, bscore = 1 << 30;
vector<i32> neighbors;
vector<i32> near[N];

inline void update() {
    if(chmin(bscore, score)) {
        memcpy(bpaths, paths, sizeof(paths));
        la = score * 0.5;
    }
}

inline void twoopt(i32 i, i32 j) {
    i32 i0 = paths[i - 1];
    i32 i1 = paths[i];
    i32 j0 = paths[j - 1];
    i32 j1 = paths[j];
    auto &ei = edges[i0][i1];
    auto &ej = edges[j0][j1];
    auto &e0 = edges[i0][j0];
    auto &e1 = edges[i1][j1];
    f64 nav = av + e0.distanceDivN + e1.distanceDivN - ei.distanceDivN - ej.distanceDivN;
    f64 nav2 = av2 + e0.distance2DivN + e1.distance2DivN - ei.distance2DivN - ej.distance2DivN;
    f64 diff = nav2 - sq(nav) - score;
    if(diff + (e0.penalty + e1.penalty - ei.penalty - ej.penalty) * la < 0) {
        score += diff;
        av = nav;
        av2 = nav2;
        if(i > j) {
            swap(i, j);
        }
        for(--j; i < j;) {
            p2i[paths[i]] = j;
            p2i[paths[j]] = i;
            swap(paths[i], paths[j]);
            ++i;
            --j;
        }
        update();
    }
}

i32 main() {
    timer.measure();
    {
        i32 _;
        cin >> _;
    }
    {
        vector<pair<i32, i32>> ps(N);
        for(auto &p : ps) {
            cin >> p.first >> p.second;
        }
        rep(j, 1, N) {
            rep(i, j) {
                auto &p = ps[i];
                auto &q = ps[j];
                auto &e = edges[i][j];
                auto &f = edges[j][i];
                f64 d = sq(p.first - q.first) + sq(p.second - q.second);
                e.distance2DivN = f.distance2DivN = d / N;
                e.distance = f.distance = sqrt(d);
                e.distanceDivN = f.distanceDivN = e.distance / N;
            }
        }
        rep(i, N) {
            rep(j, N) {
                if(i == j)
                    continue;
                auto &e = edges[i][j];
                if(230 < e.distance && e.distance < 350) {
                    near[i].emplace_back(j);
                }
            }
        }
    }
    i32 ms = 2;
    rep(mi, ms) {
        rep(i, N) {
            rep(j, N) {
                auto &e = edges[i][j];
                e.penalty = 0;
                e.penaltyInv = 1;
            }
        }
        neighbors.assign(N, 0);
        rep(i, N) {
            neighbors[i] = i + 1;
        }
        rep(i, N + 1) {
            paths[i] = i % N;
        }
        random_shuffle(paths + 1, paths + N, rando);
        rep(i, N + 1) {
            p2i[paths[i]] = i;
        }
        av = av2 = 0;
        rep(i, 1, N + 1) {
            auto &e = edges[paths[i - 1]][paths[i]];
            av += e.distanceDivN;
            av2 += e.distance2DivN;
        }
        score = av2 - sq(av);
        update();
        timer.limit = 1.95 * (mi + 1) / ms;
        while(!timer.elapses()) {
            for(i32 i : neighbors) {
                for(i32 p : near[paths[i]]) {
                    i32 j = p2i[p];
                    if(u32(i - j + 1) > 2) {
                        twoopt(i, j);
                    }
                }
                for(i32 p : near[paths[i - 1]]) {
                    i32 j = p ? p2i[p] + 1 : 1;
                    if(u32(i - j + 1) > 2) {
                        twoopt(i, j);
                    }
                }
            }
            neighbors.clear();
            f64 bd = 0;
            rep(i, 1, N + 1) {
                auto &e = edges[paths[i - 1]][paths[i]];
                f64 d = abs(e.distance - av) * e.penaltyInv;
                if(chmax(bd, d)) {
                    neighbors.clear();
                }
                if(bd < d + 1e-5) {
                    neighbors.emplace_back(i);
                }
            }
            for(i32 i : neighbors) {
                auto &e = edges[paths[i - 1]][paths[i]];
                auto &f = edges[paths[i]][paths[i - 1]];
                e.penalty = ++f.penalty;
                e.penaltyInv = f.penaltyInv = 1.0 / (f.penalty + 1);
            }
        }
    }
    score = bscore;
    memcpy(paths, bpaths, sizeof(paths));
    rep(i, N) {
        cout << paths[i] << endl;
    }
    return 0;
}
