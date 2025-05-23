#if !__INCLUDE_LEVEL__
#include __FILE__

// ====================================
// Main
// ====================================
#include <atcoder/dsu>
using namespace atcoder;

const ll INF = 1e18;
const ll MOD = 1e9 + 7; // 998244353
const vector<int> dx = {-1, 0, 1, 0};
const vector<int> dy = {0, -1, 0, 1};
const vector<string> ds = {"L", "U", "R", "D"};

int H, W, Y;

void solve() {
    cin >> H >> W >> Y;
    vvi A(H, vi(W));
    for(int h : range(H)) {
        for(int w : range(W)) {
            cin >> A[h][w];
        }
    }

    vector<vector<bool>> pushed(H, vector<bool>(W, false));
    priority_queue<pair<int, pii>, vector<pair<int, pii>>, greater<>> que;
    for(int w : range(W)) {
        que.push({A[0][w], {0, w}});
        pushed[0][w] = true;
        if(H >= 2) {
            que.push({A[H - 1][w], {H - 1, w}});
            pushed[H - 1][w] = true;
        }
    }

    for(int h : range(1, H - 1)) {
        que.push({A[h][0], {h, 0}});
        pushed[h][0] = true;
        if(W >= 2) {
            que.push({A[h][W - 1], {h, W - 1}});
            pushed[h][W - 1] = true;
        }
    }

    ll ans = H * W;
    for(int y : irange(1, Y + 1)) {
        while(!que.empty() && que.top().first <= y) {
            auto [val, id_pair] = que.top();
            ans--;
            que.pop();
            auto [y, x] = id_pair;
            for(int i : irange(4)) {
                int nx = x + dx[i];
                int ny = y + dy[i];
                if(!outof(nx, ny, W, H) && !pushed[ny][nx]) {
                    que.push({A[ny][nx], {ny, nx}});
                    pushed[ny][nx] = true;
                }
            }
        }
        cout << ans << "\n";
    }
}

int main() {
    solve();
}

// ====================================
// Template
// ====================================

#else

#include <bits/stdc++.h>

#include <boost/format.hpp>
#include <boost/range/irange.hpp>
using namespace std;
using boost::irange;

// ====================================
// Judge環境切り替え
// ====================================
#ifndef ONLINE_JUDGE
#define _GLIBCXX_DEBUG
#include <cpp-dump.hpp> // https://zenn.dev/sassan/articles/19db660e4da0a4
#else
#define cpp_dump(...) ;
#endif

// ====================================
// マクロ
// ====================================

using ll = long long;
using ull = unsigned long long;
using pii = pair<int, int>;
using pll = pair<ll, ll>;

using vi = vector<int>;
using vvi = vector<vi>;
using vvvi = vector<vvi>;

using ll = long long;
using vll = vector<ll>;
using vvll = vector<vll>;
using vvvll = vector<vvll>;

#define rep(i, a, b)  for(int i = (a); i < (b); ++i)
#define rrep(i, a, b) for(int i = (a); i >= (b); --i)
#define ALL(obj)      (obj).begin(), (obj).end()
#define RALL(obj)     (obj).rbegin(), (obj).rend()

// ====================================
// 入力高速化
// ====================================
struct IOInit {
    IOInit() {
        ios::sync_with_stdio(0);
        cin.tie(0);
        cout << setprecision(15);
    }
} ioinit;

// ====================================
// 範囲for: [start, end) step
// ====================================
class range {
public:
    class Iterator {
    public:
        using value_type = int;
        int value, step;

        Iterator(int value, int step) : value(value), step(step) {
        }

        int operator*() const {
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

    // Range(N): same as Range(0, N)
    range(int end) : range(0, end, 1) {
    }

    // Range(start, end): step = +1 by default
    range(int start, int end) : range(start, end, 1) {
    }

    // Full constructor
    range(int start, int end, int step) : begin_(start), end_(end), step_(step) {
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

// ====================================
// Utils
// ====================================
template <typename T>
T int_pow(T base, long long exp) {
    T result = 1;
    while(exp > 0) {
        if(exp & 1) result *= base;
        base *= base;
        exp >>= 1;
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

// ====================================
// グリッド
// ====================================
template <integral T>
bool outof(T x, T y, T W, T H) {
    return x < 0 || x >= W or y < 0 || y >= H ? true : false;
}

// ====================================
// グラフ
// ====================================
template <integral T>
vector<vector<T>> input_edges(T N, T M) {
    vector<vector<T>> edges(N);
    for(int i : views::iota(0, M)) {
        int a, b;
        cin >> a >> b;
        a--;
        b--;
        edges[a].push_back(b);
        edges[b].push_back(a);
    }
    return edges;
}

// ====================================
// ハッシュ（https://qiita.com/hamamu/items/4d081751b69aa3bb3557）
// ====================================
template <class T>
size_t HashCombine(const size_t seed, const T &v) {
    return seed ^ (std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}
/* pair用 */
template <class T, class S>
struct std::hash<std::pair<T, S>> {
    size_t operator()(const std::pair<T, S> &keyval) const noexcept {
        return HashCombine(std::hash<T>()(keyval.first), keyval.second);
    }
};
/* complex用 */
template <class T>
struct std::hash<complex<T>> {
    size_t operator()(const complex<T> &x) const noexcept {
        size_t s = 0;
        s = HashCombine(s, x.real());
        s = HashCombine(s, x.imag());
        return s;
    }
};
/* vector用 */
template <class T>
struct std::hash<std::vector<T>> {
    size_t operator()(const std::vector<T> &keyval) const noexcept {
        size_t s = 0;
        for(auto &&v : keyval)
            s = HashCombine(s, v);
        return s;
    }
};
/* deque用 */
template <class T>
struct std::hash<std::deque<T>> {
    size_t operator()(const std::deque<T> &keyval) const noexcept {
        size_t s = 0;
        for(auto &&v : keyval)
            s = HashCombine(s, v);
        return s;
    }
};
/* tuple用 */
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

#endif