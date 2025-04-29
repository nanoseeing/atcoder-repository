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

const int MAX_HW = 8 * 1e5 + 5;
ll grid_search(vvl grid, ll H, ll W) {
    unordered_map<ll, ll> num_map;
    rep(j, 0, W) {
        unordered_set<ll> num_set;
        rep(i, 0, H) {
            num_set.emplace(grid[i][j]);
        }
        if(num_set.size() == 1) {
            num_map[*num_set.begin()]++;
        }
    }

    ll ans = 0;
    for(auto it : num_map) {
        // cout << it.first << " " << it.second << endl;
        ans = max(ans, it.second);
    }

    return ans * H;
}

void solve() {
    ll H, W;
    cin >> H >> W;

    vvl grid = vector(H, vl(W));
    rep(i, 0, H) {
        rep(j, 0, W) {
            cin >> grid[i][j];
        }
    }

    ll max_ans = 0;
    rep(i, 0, 1 << H) {
        vl target_j;
        rep(j, 0, 9) {
            if(i >> j & 1) {
                target_j.push_back(j);
            }
        }
        vvl target_grid = vector(target_j.size(), vl(W));
        rep(j, 0, (int)target_j.size()) {
            rep(k, 0, W) {
                target_grid[j][k] = grid[target_j[j]][k];
            }
        }
        max_ans = max(max_ans, grid_search(target_grid, target_j.size(), W));
    }

    cout << max_ans << endl;
}

int main() {
    solve();
    return 0;
}