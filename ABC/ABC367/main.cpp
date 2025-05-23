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

void solve() {
    ll N;
    cin >> N;

    vl A(N), B(N);
    rep(i, 0, N) {
        cin >> A[i] >> B[i];
        A[i]--;
    }

    // dp[1500][1500] 人iまでを決めて、チーム0とチーム1の強さをx,yにしたときの変更回数最小。

    ll init_a = 0, init_b = 0;
    rep(i, 0, N) {
        if(A[i] == 0) {
            init_a += B[i];
        } else if(A[i] == 1) {
            init_b += B[i];
        }
    }

    ll MAX = 1505;
    vvl dp = vvl(MAX, vl(MAX, 1e9));
    dp[init_a][init_b] = 0;

    rep(i, 0, N) {
        vvl new_dp = dp;
        rep(x, 0, MAX) {
            rep(y, 0, MAX) {
                ll a = A[i];
                ll b = B[i];
                if(a == 0) {
                    // -> 1
                    if(x + b < MAX && y - b >= 0) chmin(new_dp[x][y], dp[x + b][y - b] + 1);
                    // -> 2
                    if(x + b < MAX) chmin(new_dp[x][y], dp[x + b][y] + 1);
                }
                if(a == 1) {
                    // -> 0
                    if(x - b >= 0 && y + b < MAX) chmin(new_dp[x][y], dp[x - b][y + b] + 1);
                    // -> 2
                    if(y + b < MAX) chmin(new_dp[x][y], dp[x][y + b] + 1);
                }
                if(a == 2) {
                    // -> 0
                    if(x - b >= 0) chmin(new_dp[x][y], dp[x - b][y] + 1);
                    // -> 1
                    if(y - b >= 0) chmin(new_dp[x][y], dp[x][y - b] + 1);
                }
            }
        }
        swap(new_dp, dp);
    }

    ll total = accumulate(ALL(B), 0LL);
    if(total % 3 != 0) {
        cout << -1 << endl;
        return;
    }

    ll obj_num = total / 3;
    if(10000 < dp[obj_num][obj_num]) {
        cout << -1 << endl;
    } else {
        cout << dp[obj_num][obj_num] << endl;
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
using namespace std;

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
using pll = pair<ll, ll>;
using vl = vector<ll>;
using vvl = vector<vector<ll> >;
using vvvl = vector<vector<vector<ll> > >;

#define rep(i, a, b)  for(ll i = (a); i < (b); ++i)
#define rrep(i, a, b) for(ll i = (a); i >= (b); --i)
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
vector<vector<int> > input_edges(T N, T M) {
    vector<vector<int> > edges(N);
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
struct std::hash<std::pair<T, S> > {
    size_t operator()(const std::pair<T, S> &keyval) const noexcept {
        return HashCombine(std::hash<T>()(keyval.first), keyval.second);
    }
};
/* complex用 */
template <class T>
struct std::hash<complex<T> > {
    size_t operator()(const complex<T> &x) const noexcept {
        size_t s = 0;
        s = HashCombine(s, x.real());
        s = HashCombine(s, x.imag());
        return s;
    }
};
/* vector用 */
template <class T>
struct std::hash<std::vector<T> > {
    size_t operator()(const std::vector<T> &keyval) const noexcept {
        size_t s = 0;
        for(auto &&v : keyval)
            s = HashCombine(s, v);
        return s;
    }
};
/* deque用 */
template <class T>
struct std::hash<std::deque<T> > {
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
struct std::hash<std::tuple<Args...> > {
    size_t operator()(const tuple<Args...> &keyval) const noexcept {
        return HashTupleCore<tuple_size<tuple<Args...> >::value>()(keyval);
    }
};

#endif