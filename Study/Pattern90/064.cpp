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
    ll N, Q;
    cin >> N >> Q;
    vl A;
    rep(i, 0, N) {
        ll a;
        cin >> a;
        A.push_back(a);
    }

    ll ans = 0;
    vl diff_A;
    rep(i, 0, N - 1) {
        ll d = A[i + 1] - A[i];
        diff_A.push_back(d);
        ans += abs(d);
    }

    rep(i, 0, Q) {
        ll l, r, v;
        cin >> l >> r >> v;
        l--;
        r--;
        if(l > 0) {
            ans -= abs(diff_A[l - 1]);
            diff_A[l - 1] += v;
            ans += abs(diff_A[l - 1]);
        }
        if(r < N - 1) {
            ans -= abs(diff_A[r]);
            diff_A[r] -= v;
            ans += abs(diff_A[r]);
        }

        cout << ans << "\n";
    }
}

int main() {
    solve();
    return 0;
}