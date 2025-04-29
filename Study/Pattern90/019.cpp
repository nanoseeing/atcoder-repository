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

const int MAX_LR = 401;
ll dp[MAX_LR][MAX_LR];

ll dfs(ll l, ll r, vl &A) {
    if(dp[l][r] != -1) {
        return dp[l][r];
    }
    if(l + 1 == r) {
        return dp[l][r] = abs(A[l] - A[r]);
    }
    if(l == r) {
        return 1e9;
    }

    ll ans = 1e9;
    for(ll i = l; i < r; i++) {
        ans = min(ans, dfs(l, i, A) + dfs(i + 1, r, A));
    }
    ans = min(ans, dfs(l + 1, r - 1, A) + abs(A[l] - A[r]));
    return dp[l][r] = ans;
}

void solve() {
    ll N;
    cin >> N;
    vl A(N * 2);
    rep(i, 0, N * 2) {
        cin >> A[i];
    }

    rep(i, 0, MAX_LR) {
        rep(j, 0, MAX_LR) {
            dp[i][j] = -1;
        }
    }

    ll ans = dfs(0, N * 2 - 1, A);
    cout << ans << endl;
}

int main() {
    solve();
    return 0;
}