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

const int MAX_HW = 1000;
void solve() {
    ll N;
    cin >> N;
    vvl grid = vector(MAX_HW + 5, vl(MAX_HW + 5, 0));

    rep(i, 0, N) {
        ll x1, y1, x2, y2;
        cin >> x1 >> y1 >> x2 >> y2;
        x2--;
        y2--;
        grid[x1][y1]++;
        grid[x2 + 1][y2 + 1]++;
        grid[x1][y2 + 1]--;
        grid[x2 + 1][y1]--;
    }

    cerr << "=====" << endl;
    rep(i, 0, MAX_HW + 3) {
        rep(j, 0, MAX_HW + 3) {
            grid[i][j + 1] += grid[i][j];
        }
    }
    cerr << "=====" << endl;
    rep(j, 0, MAX_HW + 3) {
        rep(i, 0, MAX_HW + 3) {
            grid[i + 1][j] += grid[i][j];
        }
    }

    cerr << "=====" << endl;
    vl ans(N + 3, 0);
    rep(i, 0, MAX_HW + 3) {
        rep(j, 0, MAX_HW + 3) {
            ans[grid[i][j]]++;
        }
    }

    cerr << "=====" << endl;
    rep(i, 1, N + 1) {
        cout << ans[i] << "\n";
    }
}

int main() {
    solve();
    return 0;
}