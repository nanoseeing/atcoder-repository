#ifndef ONLINE_JUDGE
#define _GLIBCXX_DEBUG
#include <cpp-dump.hpp>
#else
#define cpp_dump(...)
#endif

#include <atcoder/scc>
#include <bits/stdc++.h>

using namespace std;
using namespace atcoder;

struct Init {
    Init() {
        ios::sync_with_stdio(0);
        cin.tie(0);
        cout << setprecision(15);
    }
} init;

using ll = long long;
using ld = long double;
using pii = pair<int, int>;
using pll = pair<ll, ll>;
using vl = vector<ll>;
using vvl = vector<vector<ll>>;
#define rep(i, a, b)  for(ll i = (a); i < (b); ++i)
#define rrep(i, a, b) for(ll i = (a); i >= (b); --i)

void solve() {
    ll N, M;
    cin >> N >> M;

    scc_graph graph(N);
    vvl edges(N);
    vvl redges(N);
    rep(i, 0, M) {
        ll u, v;
        cin >> u >> v;
        u--;
        v--;
        graph.add_edge(u, v);
        edges[u].push_back(v);
        redges[v].push_back(u);
    }

    auto x = graph.scc();

    vector<bool> visited(N, false);
    ll ans = 0;
    vl cycle_nodes;
    for(const auto &nodes : x) {
        if(nodes.size() > 1) {
            for(const auto &node : nodes) {
                cycle_nodes.push_back(node);
                ans += 1;
                visited[node] = true;
            }
        }
    }

    for(const auto &node : cycle_nodes) {
        deque<ll> q;
        q.push_front(node);

        while(!q.empty()) {
            auto v = q.front();
            q.pop_front();

            for(const auto &to : redges[v]) {
                if(!visited[to]) {
                    ans++;
                    q.push_back(to);
                    visited[to] = true;
                }
            }
        }
    }

    cout << ans << endl;
}

int main() {
    solve();
    return 0;
}