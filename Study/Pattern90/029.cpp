#include <atcoder/lazysegtree>
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

using S = long long;
using F = long long;

const S INF = 8e18;
const F ID = 8e18;

S op(S a, S b) {
    return std::max(a, b);
}
S e() {
    return -INF;
}
S mapping(F f, S x) {
    return (f == ID ? x : f);
}
F composition(F f, F g) {
    return (f == ID ? g : f);
}
F id() {
    return ID;
}
void solve() {
    ll W, N;
    cin >> W >> N;

    atcoder::lazy_segtree<S, op, e, F, mapping, composition, id> seg(W + 1);
    seg.apply(0, W + 1, 0);

    vl ans;
    rep(i, 0, N) {
        ll L, R;
        cin >> L >> R;
        L--;
        R--;
        ll x = seg.prod(L, R + 1);
        seg.apply(L, R + 1, x + 1);
        ans.push_back(x + 1);
    }

    rep(i, 0, N) {
        cout << ans[i] << endl;
    }
}
int main() {
    solve();
    return 0;
}