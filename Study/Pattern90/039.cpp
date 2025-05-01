#include <bits/stdc++.h>

using namespace std;

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

ll dfs(vvl &dag, ll s) {
    if(dp[s] != -1)
        return dp[s];
    ll ret = 1;
    for(auto &to : dag[s]) {
        ret += dfs(dag, to);
    }
    return dp[s] = ret;
};

void solve() {
    ll N;
    cin >> N;

    vvl edges = vector<vector<ll>>(N);
    rep(i, 0, N - 1) {
        ll a, b;
        cin >> a >> b;
        a--;
        b--;
        edges[a].push_back(b);
        edges[b].push_back(a);
    }

    auto create_dag = [&](ll s, vvl &edges) {
        vector<bool> visited(N, false);
        vvl dag = vector<vector<ll>>(N);
        visited[s] = true;
        deque<ll> que;
        que.push_back(s);
        while(!que.empty()) {
            ll v = que.front();
            que.pop_front();
            for(auto &to : edges[v]) {
                if(!visited[to]) {
                    que.push_back(to);
                    dag[v].push_back(to);
                    visited[to] = true;
                }
            }
        }
        return dag;
    };

    rep(i, 0, N + 1) {
        dp[i] = -1;
    }

    auto dag = create_dag(0, edges);
    dfs(dag, 0);

    ll ans = 0;
    rep(from, 0, N) {
        for(auto &to : dag[from]) {
            ans += (N - dp[to]) * dp[to];
        }
    }
    cout << ans << endl;
}

int main() {
    solve();
    return 0;
}