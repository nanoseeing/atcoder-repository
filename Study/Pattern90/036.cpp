#include <atcoder/segtree>
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

ll op(ll a, ll b) {
    return max(a, b);
}

ll e() {
    return ll(-1e18);
}

void solve() {
    ll W, N;
    cin >> W >> N;

    vl L, R, V;
    rep(i, 0, N) {
        ll l, r, v;
        cin >> l >> r >> v;
        L.push_back(l);
        R.push_back(r);
        V.push_back(v);
    }

    // dp[i][w] : iまで見てちょうどw消費するときのvの最大
    // dp[i][w] = max(dp[i][w], dp[i - 1][w - L[i] ~ w - R[i]] + V[i]);
    // segtree: 区間最大、1点更新
    const ll max_W = 1e4 + 5;
    segtree<ll, op, e> seg(max_W);
    seg.set(0, 0);

    rep(i, 0, N) {
        auto pre_seg = seg;
        rep(j, 0, max_W) {
            if(j < L[i]) {
                pre_seg.set(j, seg.get(j));
            } else {
                ll left = max(ll(0), j - R[i]);
                ll right = max(ll(0), j - L[i]);
                ll max_v = seg.prod(left, right + 1) + V[i];
                pre_seg.set(j, max(seg.get(j), max_v));
            }
        }
        seg = pre_seg;
    }

    ll ans = seg.get(W);
    if(ans < 0) {
        cout << -1 << endl;
        return;
    }
    cout << ans << endl;
}

int main() {
    solve();
    return 0;
}