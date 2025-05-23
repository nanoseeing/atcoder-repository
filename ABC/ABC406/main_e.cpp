#if !__INCLUDE_LEVEL__
#include __FILE__

// ====================================
// Main
// ====================================
#include <atcoder/modint>
using namespace atcoder;

const ll INF = 1e18;
const ll MOD = 1e9 + 7; // 998244353
const vector<int> dx = {-1, 0, 1, 0};
const vector<int> dy = {0, -1, 0, 1};
const vector<string> ds = {"L", "U", "R", "D"};

using mint = atcoder::modint998244353;

const int MAX = 510000;
mint fac[MAX], finv[MAX], inv[MAX];

// テーブルを作る前処理
void COMinit() {
    const int MOD = mint::mod();
    fac[0] = fac[1] = 1;
    finv[0] = finv[1] = 1;
    inv[1] = 1;
    for(int i = 2; i < MAX; i++) {
        fac[i] = fac[i - 1] * i;
        inv[i] = MOD - inv[MOD % i] * (MOD / i);
        finv[i] = finv[i - 1] * inv[i];
    }
}

template <integral T1, integral T2>
mint COM(T1 n, T2 k) {
    if(n < k) return 0;
    if(n < 0 || k < 0) return 0;
    return fac[n] * finv[k] * finv[n - k];
}

int digit_higher_bit(ll x) {
    int rank = 0;
    while(x) {
        rank++;
        x >>= 1;
    }
    return rank;
}

void solve(ll N, ll K) {
    ll n_digit = digit_higher_bit(N) + 1;

    vl two = vl(61);
    rep(i, 0, 61) {
        two[i] = int_pow(2LL, ll(i));
    }
    vl two_sum = vl(61);
    two_sum[0] = 1;
    rep(i, 1, 61) {
        two_sum[i] += two[i];
    }

    mint ans = 0;
    ll remain = 0;
    ll k_cout = 0;

    rrep(d, n_digit - 1, 0) {
        if(N >> d & 1) {
            // 残りd-1桁からK-kcount選び, / dしたのが平均
            if(d > 0) {
                mint comb, comb2;
                comb = COM(d, K - k_cout);
                comb2 = comb * (K - k_cout) / d;
                ans += comb2 * (two[d] - 1);
                ans += (remain * comb);
            } else {
                if(k_cout == K) {
                    ans += remain;
                    cpp_dump(remain);
                }
            }

            k_cout++;
            remain += 1LL << d;
        }
    }

    if(__builtin_popcountll(N) == K) {
        ans += N;
    }
    cout << ans.val() << endl;
}
int main() {
    COMinit();

    ll T;
    cin >> T;
    rep(i, 0, T) {
        ll N, K;
        cin >> N >> K;
        solve(N, K);
    }
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
        exp >>= 1;
        if(exp == 0) break;
        base *= base;
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