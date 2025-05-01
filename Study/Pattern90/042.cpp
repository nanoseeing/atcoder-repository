#include <atcoder/dsu>
#include <bits/stdc++.h>

using namespace std;
using namespace atcoder;

#ifndef ONLINE_JUDGE
#define _GLIBCXX_DEBUG
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

ll dp[100000 + 5];
const int MOD = 1e9 + 7;

void solve() {
    ll K;
    cin >> K;

    if(K % 9 != 0) {
        cout << 0 << endl;
        return;
    }

    rep(i, 0, 10) {
        dp[i] = 1;
    }

    rep(i, 10, K + 3) {
        rep(j, 1, 10) {
            if(i - j >= 0) {
                dp[i] += dp[i - j];
                dp[i] %= MOD;
            }
        }
    }
    cout << dp[K] << endl;

    // dp[x] 和がxになる数の組み合わせ
    // dp[x] = dp[x-1] + dp[x-2] + ... + dp[x-9]
}

int main() {
    solve();
    return 0;
}