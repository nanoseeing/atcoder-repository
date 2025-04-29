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
    ll Q;
    cin >> Q;

    deque<ll> dq;
    rep(i, 0, Q) {
        ll t, x;
        cin >> t >> x;
        if(t == 1) {
            dq.push_front(x);
        } else if(t == 2) {
            dq.push_back(x);
        } else {
            cout << dq[x - 1] << endl;
        }
    }
}

int main() {
    solve();
    return 0;
}