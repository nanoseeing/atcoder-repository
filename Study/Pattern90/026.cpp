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
    vvl edges = vector(N, vl());
    rep(i, 0, N - 1) {
        ll a, b;
        cin >> a >> b;
        a--;
        b--;
        edges[a].push_back(b);
        edges[b].push_back(a);
    }

    vector<int> colors(N, -1);
    auto dfs = [&](int v) -> void {
        colors[v] = 0;
        auto q = deque<pii>();
        q.push_back({v, 0});
        while(!q.empty()) {
            auto [v, color] = q.front();
            q.pop_front();
            for(int u : edges[v]) {
                if(colors[u] == -1) {
                    colors[u] = 1 - color;
                    q.push_back({u, 1 - color});
                }
            }
        }
    };
    dfs(0);

    int cnt_color_0 = 0;
    int cnt_color_1 = 0;
    rep(i, 0, N) {
        if(colors[i] == 0) {
            cnt_color_0++;
        } else if(colors[i] == 1) {
            cnt_color_1++;
        }
    }

    int target_color = cnt_color_0 > cnt_color_1 ? 0 : 1;
    vector<int> ans;
    rep(i, 0, N) {
        if(colors[i] == target_color) {
            ans.push_back(i + 1);
        }
    }

    cerr << "=====================" << endl;
    rep(i, 0, N / 2) {
        cout << ans[i] << " ";
    }
    cout << endl;
}

int main() {
    solve();
    return 0;
}