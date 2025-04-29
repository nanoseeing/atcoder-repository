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

void solve() {
    ll N, K;
    cin >> N >> K;
    vl A;
    rep(i, 0, N) {
        ll a;
        cin >> a;
        a--;
        A.push_back(a);
    }

    ll dp[70][N];
    rep(i, 0, N) {
        dp[0][i] = A[i];
    }

    rep(i, 1, 70) {
        rep(j, 0, N) {
            dp[i][j] = dp[i - 1][dp[i - 1][j]];
        }
    }

    ll start = 0;
    ll k = K;
    ll cnt = 0;
    while(k > 0) {
        if(k & 1) {
            start = dp[cnt][start];
        }
        k >>= 1;
        cnt++;
    }

    cout << start + 1 << endl;
}

int main() {
    solve();
    return 0;
}