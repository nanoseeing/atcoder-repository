// ============================================================================
// Template
// ============================================================================
#include <bits/stdc++.h>
using namespace std;

// Judge環境切り替え
#ifndef ONLINE_JUDGE
#define _GLIBCXX_DEBUG
#include <cpp-dump.hpp> // https://zenn.dev/sassan/articles/19db660e4da0a4
#else
#define cpp_dump(...) ;
#endif

// IO高速化
struct IOInit {
    IOInit() {
        ios::sync_with_stdio(0);
        cin.tie(0);
        cout << setprecision(15);
    }
} ioinit;

// 範囲for: [start, end) step
class range {
public:
    class Iterator {
    public:
        using value_type = int;
        int value, step;

        template <integral T1, integral T2>
        Iterator(T1 value, T2 step) : value(value), step(step) {
        }

        auto operator*() const {
            return value;
        }

        Iterator &operator++() {
            value += step;
            return *this;
        }

        bool operator!=(const Iterator &other) const {
            return step > 0 ? value < other.value : value > other.value;
        }
    };

    template <integral T>
    range(T end) : range(0, end, 1) {
    }
    template <integral T1, integral T2>
    range(T1 start, T2 end) : range(start, end, 1) {
    }
    template <integral T1, integral T2, integral T3>
    range(T1 start, T2 end, T3 step) : begin_(start), end_(end), step_(step) {
        if(step == 0) {
            throw std::invalid_argument("Range step must not be 0");
        }
    }

    Iterator begin() const {
        return Iterator(begin_, step_);
    }
    Iterator end() const {
        return Iterator(end_, step_);
    }

private:
    int begin_, end_, step_;
};

// Utils
template <typename T>
T intpow(T base, T exp, optional<T> mod = nullopt) {
    T result = 1;
    while(exp > 0) {
        if(exp & 1) {
            if(mod) {
                result = result * base % *mod;
            } else {
                result = result * base;
            }
        }
        exp >>= 1;
        if(exp <= 0) break;
        if(mod) {
            base = base * base % *mod;
        } else {
            base = base * base;
        }
    }
    return result;
}

template <typename T1, typename T2>
inline bool chmin(T1 &a, const T2 &b) {
    bool compare = a > b;
    if(a > b) a = b;
    return compare;
}
template <typename T1, typename T2>
inline bool chmax(T1 &a, const T2 &b) {
    bool compare = a < b;
    if(a < b) a = b;
    return compare;
}

// Set / Multiset
template <typename Set, typename T>
bool erase(Set &s, const T &x) {
    auto itr = s.find(x);
    if(itr != s.end()) {
        s.erase(itr);
        return true;
    }
    return false;
}

// queue / deque (コピーを返すので少しだけ処理が遅いのが不満。)
template <typename Q>
auto pop(Q &q) -> decltype(q.front(), void(), typename Q::value_type{}) {
    auto val = std::move(q.front());
    q.pop_front();
    return val;
}

// priority_queue (同上)
template <typename Q>
auto pop(Q &q) -> decltype(q.top(), void(), typename Q::value_type{}) {
    auto val = std::move(q.top());
    q.pop();
    return val;
}

// グリッド
template <integral T>
bool outof(T x, T y, T W, T H) {
    return x < 0 || x >= W or y < 0 || y >= H ? true : false;
}

// ハッシュ（https://qiita.com/hamamu/items/4d081751b69aa3bb3557）
template <class T>
size_t HashCombine(const size_t seed, const T &v) {
    return seed ^ (std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}
template <class T, class S>
struct std::hash<std::pair<T, S>> {
    size_t operator()(const std::pair<T, S> &keyval) const noexcept {
        return HashCombine(std::hash<T>()(keyval.first), keyval.second);
    }
};
template <class T>
struct std::hash<std::vector<T>> {
    size_t operator()(const std::vector<T> &keyval) const noexcept {
        size_t s = 0;
        for(auto &&v : keyval)
            s = HashCombine(s, v);
        return s;
    }
};
template <int N>
struct HashTupleCore {
    template <class Tuple>
    size_t operator()(const Tuple &keyval) const noexcept {
        size_t s = HashTupleCore<N - 1>()(keyval);
        return HashCombine(s, std::get<N - 1>(keyval));
    }
};
template <>
struct HashTupleCore<0> {
    template <class Tuple>
    size_t operator()(const Tuple &keyval) const noexcept {
        return 0;
    }
};
template <class... Args>
struct std::hash<std::tuple<Args...>> {
    size_t operator()(const tuple<Args...> &keyval) const noexcept {
        return HashTupleCore<tuple_size<tuple<Args...>>::value>()(keyval);
    }
};

// ============================================================================
// Main
// ============================================================================

#include <atcoder/all>
#include <boost/format.hpp>
using namespace atcoder;

using ll = long long;
using i128 = __int128_t;
using mint = modint1000000007; // modint998244353

#define ALL(obj)  (obj).begin(), (obj).end()
#define RALL(obj) (obj).rbegin(), (obj).rend()

const vector<int> dir_dx = {-1, 0, 1, 0};
const vector<int> dir_dy = {0, -1, 0, 1};
const vector<string> dir_str = {"L", "U", "R", "D"};

const int INF = 1e9 + 7;
const long long INF_LL = 1e18 + 7;
const long long MOD = 1e9 + 7; // 998244353

using Poly = vector<vector<int>>;

Poly rotate90Clockwise(Poly &matrix) {
    int N = matrix.size();
    for(int i = 0; i < N; ++i) {
        for(int j = i + 1; j < N; ++j) {
            std::swap(matrix[i][j], matrix[j][i]);
        }
    }
    for(int i = 0; i < N; ++i) {
        std::reverse(matrix[i].begin(), matrix[i].end());
    }
    return matrix;
}

bool help(Poly &grid, Poly &target_poly, int dy, int dx, int rot) {
    Poly poly = target_poly;
    for(int i : range(rot)) {
        poly = rotate90Clockwise(poly);
    }

    int N = 4;
    for(int y : range(N)) {
        for(int x : range(N)) {
            int ny = y + dy;
            int nx = x + dx;
            if(outof(nx, ny, N, N)) {
                if(poly[y][x] == 1) {
                    return true;
                }
            } else {
                if(poly[y][x] == 1 && grid[ny][nx] == 1) {
                    return true;
                } else if(poly[y][x] == 1) {
                    grid[ny][nx] = 1;
                }
            }
        }
    }
    return false;
}

void solve() {
    vector<Poly> polys;

    int N = 4;
    int cnt = 0;
    for(int i : range(3)) {
        Poly poly(N, vector<int>(N));
        for(int y : range(N)) {
            string s;
            cin >> s;
            for(int x : range(N)) {
                if(s[x] == '.') poly[y][x] = 0;
                if(s[x] == '#') {
                    poly[y][x] = 1;
                    cnt++;
                }
            }
        }
        polys.push_back(poly);
    };

    if(cnt != 16) {
        cout << "No" << endl;
        return;
    }

    for(int dx1 : range(-3, 4)) {
        for(int dy1 : range(-3, 4)) {
            for(int dx2 : range(-3, 4)) {
                for(int dy2 : range(-3, 4)) {
                    for(int dx3 : range(-3, 4)) {
                        for(int dy3 : range(-3, 4)) {
                            for(int rot1 : range(0, 4)) {
                                for(int rot2 : range(0, 4)) {
                                    for(int rot3 : range(0, 4)) {
                                        Poly grid(N, vector<int>(N));
                                        if(help(grid, polys[0], dy1, dx1, rot1)) break;
                                        if(help(grid, polys[1], dy2, dx2, rot2)) break;
                                        if(help(grid, polys[2], dy3, dx3, rot3)) break;
                                        cout << "Yes" << endl;
                                        return;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    cout << "No" << endl;
}

int main() {
    solve();
}