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

vector<string> create_str(vector<string> target_strs) {
    int n = target_strs.size();
    int m = 0;
    for(int i : range(n)) {
        m += target_strs[i].size();
    }
    int under_size = 16 - m;

    unordered_set<vector<int>> all_under_cnts;

    auto dfs = [&](auto f, vector<int> now_cnts) {
        all_under_cnts.insert(now_cnts);
        int s = accumulate(ALL(now_cnts), 0);
        if(under_size == s) return;
        for(int i : range(n - 1)) {
            auto new_cnts = now_cnts;
            new_cnts[i] += 1;
            if(!all_under_cnts.contains(new_cnts)) {
                f(f, new_cnts);
            }
        }
    };

    vector<int> s_cnts;
    for(int i : range(n - 1)) {
        s_cnts.push_back(1);
    }
    dfs(dfs, s_cnts);
    cpp_dump(all_under_cnts, n);

    vector<string> ret;
    for(auto cnts : all_under_cnts) {
        string now_s = "";
        for(int i : range(n - 1)) {
            now_s += target_strs[i];
            now_s += string(cnts[i], '_');
        }
        now_s += target_strs[n - 1];
        if(3 <= now_s.size() && now_s.size() <= 16) {
            ret.push_back(now_s);
        }
    }
    return ret;
}

void solve() {
    int N, M;
    cin >> N >> M;
    vector<string> S(N), T(M);

    vector<int> perm;
    unordered_set<string> t_set;
    for(int i : range(N)) {
        cin >> S[i];
        perm.push_back(i);
    }
    for(int i : range(M)) {
        cin >> T[i];
        t_set.insert(T[i]);
    }

    do {
        vector<string> target_strs;
        for(int i : range(N)) {
            target_strs.push_back(S[perm[i]]);
        }
        auto search_strs = create_str(target_strs);
        for(string s : search_strs) {
            cpp_dump(s);
            if(!t_set.contains(s)) {
                cout << s << endl;
                return;
            }
        }
    } while(next_permutation(ALL(perm)));

    cout << "-1" << endl;
}

int main() {
    solve();
}