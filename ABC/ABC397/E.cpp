#include <atcoder/dsu>
#include <bits/stdc++.h>

using namespace std;
using namespace atcoder;

#ifndef ONLINE_JUDGE
#define _GLIBCXX_DEBUG
#include <cpp-dump.hpp>
#else
#define cpp_dump(...)
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

tuple<vvl, vector<ll>> create_root_graph(const vvl &edges, ll n, ll s0) {
    vvl root_graph(n);

    deque<ll> que;
    vector<ll> depth(n, -1);
    depth[s0] = 0;
    que.push_back(s0);
    while(!que.empty()) {
        ll v = que.front();
        que.pop_front();
        ll cnt = 0;
        for(ll u : edges[v]) {
            if(depth[u] == -1) {
                depth[u] = depth[v] + 1;
                root_graph[v].push_back(u);
                que.push_back(u);
            }
        }
    }
    return {root_graph, depth};
}

void solve() {
    ll N, K;
    cin >> N >> K;

    ll V = N * K;
    ll M = N * K - 1;
    vvl edges(V);
    rep(i, 0, M) {
        ll a, b;
        cin >> a >> b;
        a--;
        b--;
        edges[a].push_back(b);
        edges[b].push_back(a);
    }

    if(K == 1) {
        cout << "Yes" << endl;
        return;
    }

    // 根付き木の後ろから始める。
    // 深さd-1の頂点をもってきて、深さdの頂点に書かれた数字の総和を計算
    // == Kであれば0にする。
    // < K のとき、加算
    // > K のとき、No
    auto [root_graph, depth] = create_root_graph(edges, V, 0);
    auto max_depth = *max_element(depth.begin(), depth.end());
    vvl search_nodes(max_depth + 1);
    for(ll i = 0; i < V; i++) {
        search_nodes[depth[i]].push_back(i);
    }
    vl v_num(V, 1);
    rep(d, 0, max_depth + 1) {
        auto start_nodes = search_nodes[max_depth - d];
        for(ll v : start_nodes) {
            ll sum = v_num[v];
            ll non_zero_cnt = 0;
            for(ll u : root_graph[v]) {
                sum += v_num[u];
                if(v_num[u] != 0) {
                    non_zero_cnt++;
                }
            }
            if(sum == K) {
                if(non_zero_cnt >= 3) {
                    cout << "No" << endl;
                    return;
                }
                v_num[v] = 0;
            } else if(sum < K) {
                if(non_zero_cnt >= 2) {
                    cout << "No" << endl;
                    return;
                }
                v_num[v] = sum;
            } else {
                cout << "No" << endl;
                return;
            }
        }
    }

    cout << "Yes" << endl;
}

int main() {
    solve();
    return 0;
}