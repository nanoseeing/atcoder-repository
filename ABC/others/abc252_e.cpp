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
T intpow(T base, T exp) {
    T result = 1;
    while(exp > 0) {
        if(exp & 1) result *= base;
        exp >>= 1;
        if(exp <= 0) break;
        base *= base;
    }
    return result;
}

template <typename T>
T modpow(T base, T exp, T mod) {
    T result = 1;
    while(exp > 0) {
        if(exp & 1) result = result * base % mod;
        exp >>= 1;
        if(exp <= 0) break;
        base = base * base % mod;
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

// ============================================================================
// Main
// ============================================================================
#include <atcoder/dsu>
#include <boost/format.hpp>
using namespace atcoder;

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

const vector<int> dir_dx = {-1, 0, 1, 0};
const vector<int> dir_dy = {0, -1, 0, 1};
const vector<string> dir_str = {"L", "U", "R", "D"};

const ll INF = 1e18;
const ll MOD = 1e9 + 7; // 998244353

// Edge
template <class T>
struct Edge {
    int to;
    T weight;
    Edge(int t, T w) : to(t), weight(w) {
    }
};

// Graph
template <class T>
struct Graph : vector<vector<Edge<T>>> {
    Graph() {
    }
    Graph(int N) : vector<vector<Edge<T>>>(N) {
    }
};

template <class T>
vector<pair<int, int>> dijkstra(const Graph<T> &G, int s) {
    vector<T> dp(G.size(), numeric_limits<T>::max());
    vector<int> prev(G.size(), -1);
    using Node = pair<T, int>;
    priority_queue<Node, vector<Node>, greater<Node>> que;

    dp[s] = 0;
    que.push(Node(0, s));

    while(!que.empty()) {
        const auto [cur, v] = que.top();
        que.pop();
        if(cur > dp[v]) continue;

        for(const auto &e : G[v]) {
            if(dp[e.to] > dp[v] + e.weight) {
                dp[e.to] = dp[v] + e.weight;
                prev[e.to] = v;
                que.push(Node(dp[e.to], e.to));
            }
        }
    }

    vector<pair<int, int>> ret_edges;
    for(int v : range(G.size())) {
        if(prev[v] != -1) {
            ret_edges.push_back({v, prev[v]});
        }
    }

    return ret_edges;
    // return make_pair(dp, prev);
}

vector<int> reconstruct(const vector<int> &prev, int s, int t) {
    vector<int> res;
    int v = t;
    do {
        res.push_back(v);
        v = prev[v];
    } while(v != s && v != -1);
    res.push_back(s);
    reverse(res.begin(), res.end());
    return res;
}

void solve() {
    int N, M;
    cin >> N >> M;

    vector<tuple<int, int, int>> edges(M);
    map<pair<int, int>, int> edge2id;

    Graph<ll> graph(N);
    for(int m : range(M)) {
        int ta, tb;
        ll c;
        cin >> ta >> tb >> c;
        int a = min(ta, tb);
        int b = max(ta, tb);
        a--, b--;
        graph[a].push_back(Edge(b, c));
        graph[b].push_back(Edge(a, c));
        edges[m] = {c, a, b};
        edge2id[{a, b}] = m;
    }

    auto ret = dijkstra(graph, 0);
    vector<int> ans;
    for(auto e : ret) {
        auto [v1, v2] = e;
        int a = min(v1, v2);
        int b = max(v1, v2);
        ans.push_back(edge2id[{a, b}]);
    }

    cpp_dump(ret);
    assert((int)ans.size() == N - 1);
    for(int i : range(N - 1)) {
        cout << ans[i] + 1 << " ";
    }
    cout << endl;
}

int main() {
    solve();
}