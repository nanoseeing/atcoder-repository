#include <atcoder/dsu>
#include <bits/stdc++.h>

using namespace std;
using namespace atcoder;

#ifndef ONLINE_JUDGE
#define _GLIBCXX_DEBUG
#include <cpp-dump.hpp>
#else
#define cpp_dump(...)
#endif

struct Init {
    Init() {
        ios::sync_with_stdio(0);
        cin.tie(0);
        cout << setprecision(13);
    }
} init;

using ll = long long;
using ld = long double;
using pii = pair<int, int>;
using pll = pair<ll, ll>;
using vl = vector<ll>;
using vvl = vector<vector<ll>>;
#define rep(i, a, b)  for(int i = (a); i < (b); ++i)
#define rrep(i, a, b) for(int i = (a); i >= (b); --i)

ll const inf = 1e18;

void solve() {
    ll N, K;
    cin >> N >> K;

    vector<ll> X, Y;
    rep(i, 0, N) {
        ll x, y;
        cin >> x >> y;
        X.push_back(x);
        Y.push_back(y);
    }

    vvl dp(1 << N, vl(K + 1, inf));
    rep(i, 1, 1 << N) {
        vector<ll> tmp_x, tmp_y;
        rep(j, 0, N) {
            if(i & (1 << j)) {
                tmp_x.push_back(X[j]);
                tmp_y.push_back(Y[j]);
            }
        }
        ll group_size = tmp_x.size();
        if(group_size == 1) {
            dp[i][1] = 0;
            continue;
        }
        ll ret = 0;
        rep(j, 0, group_size) {
            rep(k, j + 1, group_size) {
                auto dx = tmp_x[j] - tmp_x[k];
                auto dy = tmp_y[j] - tmp_y[k];
                auto diff = dx * dx + dy * dy;
                ret = max(ret, diff);
            }
        }
        dp[i][1] = ret;
        cpp_dump(i, ret);
    }
    dp[0][0] = 0;

    cpp_dump(dp);
    // dp[i][j] = iの部分集合でj個のグループに分けたときの最大値
    rep(k, 0, K + 1) {
        rep(i, 0, (1 << N)) {
            int new_group = i;
            while(new_group != 0) {
                new_group = (new_group - 1) & i;
                int other_group = i - new_group;
                rep(k1, 0, k + 1) {
                    int k2 = k - k1;
                    dp[i][k] = min(dp[i][k], max(dp[other_group][k1], dp[new_group][k2]));
                }
            }
        }
    }
    cpp_dump(dp);

    cout << dp[(1 << N) - 1][K] << endl;
}

int main() {
    solve();
    return 0;
}