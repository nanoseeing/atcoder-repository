#include <atcoder/scc>
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

vector<ll> lis(ll N, vl A) {
    vl dp(N);
    vl dist(N);
    rep(i, 0, N) {
        dp[i] = 1e9;
        dist[i] = -1;
    }

    auto binary_search = [&](ll x, ll max_r) {
        ll l = 0;
        ll r = max_r;
        while(l < r) {
            ll m = (l + r) / 2;
            if(dp[m] < x) {
                l = m + 1;
            } else {
                r = m;
            }
        }
        return l;
    };

    rep(i, 0, N) {
        ll idx = binary_search(A[i], N);
        dp[idx] = A[i];
        dist[i] = idx + 1;
    }

    return dist;
}

ll binary_search(ll x, vl dp, ll N) {
    ll l = 0;
    ll r = N;
    while(l < r) {
        ll m = (l + r) / 2;
        if(dp[m] <= x) {
            l = m + 1;
        } else {
            r = m;
        }
    }
    return l;
};

void solve() {
    ll N;
    cin >> N;
    vl A;
    rep(i, 0, N) {
        ll a;
        cin >> a;
        A.push_back(a);
    }

    auto dp = lis(N, A);
    auto rA = A;
    reverse(rA.begin(), rA.end());
    auto rdp = lis(N, rA);
    reverse(rdp.begin(), rdp.end());

    ll ans = 0;
    rep(i, 0, N) {
        ans = max(ans, dp[i] + rdp[i] - 1);
    }
    cout << ans << endl;

    // rep(i, 0, N) {
    //     cerr << dp[i] << " ";
    // }
    // cout << endl;
    // rep(i, 0, N) {
    //     cerr << rdp[i] << " ";
    // }
    // cout << endl;
}

int main() {
    solve();
    return 0;
}