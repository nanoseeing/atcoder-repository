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
    ll N;
    cin >> N;

    vl ans;
    vector<bool> used = vector(N, false);
    vvl edges = vector(N, vl());
    deque<ll> dq;
    rep(i, 0, N) {
        ll a, b;
        cin >> a >> b;
        a--;
        b--;
        if(a == i || b == i) {
            dq.push_back(i);
        } else {
            edges[a].push_back(i);
            edges[b].push_back(i);
        }
    }

    while(!dq.empty()) {
        ll x = dq.front();
        dq.pop_front();
        if(used[x]) {
            continue;
        }
        ans.push_back(x);
        used[x] = true;
        for(auto y : edges[x]) {
            if(used[y])
                continue;
            dq.push_back(y);
        }
    }

    if((int)ans.size() != N) {
        cout << -1 << endl;
        return;
    }
    rrep(i, N - 1, 0) {
        cout << ans[i] + 1 << "\n";
    }
}

int main() {
    solve();
    return 0;
}