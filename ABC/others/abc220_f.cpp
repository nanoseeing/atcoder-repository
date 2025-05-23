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

ll N;

vvl edges;
vl tree_size;
vl ans;

pll dfs(ll v, ll pre) {
    ll es = 1;
    ll dist = 0;

    for(auto &to : edges[v]) {
        if(to == pre) {
            continue;
        }
        auto [e, d] = dfs(to, v);
        es += e;
        dist += d + e;
    }
    tree_size[v] = es;
    ans[v] = dist;
    return {es, dist};
}

void dfs2(ll v, ll pre) {
    cpp_dump(v, pre);
    for(auto &to : edges[v]) {
        if(to == pre) {
            continue;
        }
        ans[to] = ans[v] + (N - tree_size[to]) - tree_size[to];
        dfs2(to, v);
    }
}

void solve() {
    cin >> N;
    edges = input_edges(N, N - 1);
    tree_size = vl(N);
    ans = vl(N);

    dfs(0, -1);

    cpp_dump(tree_size);
    cpp_dump(ans);
    dfs2(0, -1);

    for(int i : irange(0LL, N, 1)) {
        cout << ans[i] << endl;
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
vector<vector<T> > input_edges(T N, T M) {
    vector<vector<T> > edges(N);
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