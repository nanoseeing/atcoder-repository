#ifndef ONLINE_JUDGE
#define _GLIBCXX_DEBUG
#include <cpp-dump.hpp>
#else
#define cpp_dump(...)
#endif

#include <atcoder/dsu>
#include <bits/stdc++.h>

using namespace std;
using namespace atcoder;

struct Init {
    Init() {
        ios::sync_with_stdio(0);
        cin.tie(0);
        cout << setprecision(15);
    }
} init;

using ll = long long;
using ld = long double;
using pii = pair<int, int>;
using pll = pair<ll, ll>;
using vl = vector<ll>;
using vvl = vector<vector<ll>>;
#define all(x)        x.begin(), x.end()
#define rep(i, a, b)  for(ll i = (a); i < (b); ++i)
#define rrep(i, a, b) for(ll i = (a); i >= (b); --i)

template <typename T>
int my_upper_bound(const std::vector<T> &vec, const T &value) {
    int low = 0;
    int high = vec.size(); // vec.size()は範囲外を指して良い（STLのupper_boundもそう）

    while(low < high) {
        int mid = low + (high - low) / 2;
        if(value < vec[mid]) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }

    return low; // この位置が value より大きい最初の要素
}

tuple<vl, vl> helper(ll N, ll W, vl &values, vl &weights) {

    vl ws;
    vl vs;

    ll comb = 1 << N;

    rep(i, 0, comb) {
        ll sum_v = 0;
        ll sum_w = 0;
        rep(j, 0, N) {
            if(i >> j & 1) {
                sum_v += values[j];
                sum_w += weights[j];
            }
        }
        ws.push_back(sum_w);
        vs.push_back(sum_v);
    }

    vl inds;
    rep(i, 0, comb) {
        inds.push_back(i);
    }

    sort(inds.begin(), inds.end(), [&](int i, int j) {
        return ws[i] < ws[j];
    });

    vl sorted_ws(comb);
    vl sorted_vs(comb);
    rep(i, 0, comb) {
        sorted_ws[i] = ws[inds[i]];
        sorted_vs[i] = vs[inds[i]];
    }

    rep(i, 1, comb) {
        sorted_vs[i] = max(sorted_vs[i], sorted_vs[i - 1]);
    }
    return {sorted_ws, sorted_vs};
}

void solve1(ll N, ll W, vl &values, vl &weights) {
    ll n1 = N / 2;
    ll n2 = N - n1;
    vl vs1(n1), ws1(n1);
    vl vs2(n2), ws2(n2);
    rep(i, 0, n1) {
        vs1[i] = values[i];
        ws1[i] = weights[i];
    }
    rep(i, 0, n2) {
        vs2[i] = values[i + n1];
        ws2[i] = weights[i + n1];
    }

    auto [sorted_ws1, sorted_vs1] = helper(n1, W, vs1, ws1);
    auto [sorted_ws2, sorted_vs2] = helper(n2, W, vs2, ws2);

    ll ans = 0;
    rep(i, 0, (int)sorted_ws1.size()) {
        if(W - sorted_ws1[i] < 0)
            continue;
        ll ind = my_upper_bound(sorted_ws2, W - sorted_ws1[i]);
        if(ind <= 0)
            continue;
        ind--;
        ll total_v = sorted_vs1[i] + sorted_vs2[ind];
        ans = max(ans, total_v);
    };

    cpp_dump("SOLVE1");
    cout << ans << endl;
}

void solve2(ll N, ll W, vl &values, vl &weights) {
    // W <= 1000
    vvl dp = vvl(N + 1, vector<ll>(W + 1, 0));
    rep(i, 1, N + 1) {
        rep(w, 0, W + 1) {
            if(w - weights[i - 1] >= 0) {
                dp[i][w] = max(dp[i][w], dp[i - 1][w - weights[i - 1]] + values[i - 1]);
            }
            dp[i][w] = max(dp[i][w], dp[i - 1][w]);
        }
    }

    cpp_dump("SOLVE2");
    cout << *max_element(dp[N].begin(), dp[N].end()) << endl;
}

void solve3(ll N, ll W, vl &values, vl &weights) {
    // V <= 1000
    ll max_V = 1000 * (N + 1);
    vvl dp = vvl(N + 1, vector<ll>(max_V, 1e18));
    dp[0][0] = 0;
    rep(i, 1, N + 1) {
        rep(v, 0, max_V) {
            if(v - values[i - 1] >= 0)
                dp[i][v] = min(dp[i][v], dp[i - 1][v - values[i - 1]] + weights[i - 1]);
            dp[i][v] = min(dp[i][v], dp[i - 1][v]);
        }
    }

    ll ans = 0;
    rep(v, 0, max_V) {
        if(dp[N][v] <= W)
            ans = max(ans, v);
    }

    cpp_dump("SOLVE3");
    cout << ans << endl;
}
void solve() {
    ll N, W;
    cin >> N >> W;
    vl values(N), weights(N);
    rep(i, 0, N) {
        cin >> values[i] >> weights[i];
    }
    if(N == 1) {
        if(weights[0] <= W)
            cout << values[0] << endl;
        else
            cout << 0 << endl;
    } else if(N <= 30)
        solve1(N, W, values, weights);
    else if(W <= 1000 * 201)
        solve2(N, W, values, weights);
    else
        solve3(N, W, values, weights);
}

int main() {
    solve();
    return 0;
}