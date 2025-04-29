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
    ll N, M;
    cin >> N;
    vvl grid = vector(N, vl(N));
    rep(i, 0, N) {
        rep(j, 0, N) {
            cin >> grid[i][j];
        }
    }

    cin >> M;
    vl X, Y;
    rep(i, 0, M) {
        ll x, y;
        cin >> x >> y;
        x--;
        y--;
        X.push_back(x);
        Y.push_back(y);
    }

    vl n_vec;
    rep(i, 0, N) {
        n_vec.push_back(i);
    }

    ll min_ans = 1e9;
    do {
        ll ans = 0;
        bool flag = true;
        rep(i, 0, N - 1) {
            ll a = n_vec[i];
            ll b = n_vec[i + 1];
            rep(j, 0, M) {
                if((X[j] == a && Y[j] == b) || (X[j] == b && Y[j] == a)) {
                    flag = false;
                    break;
                }
            }
            ans += grid[a][i];
        }
        ans += grid[n_vec[N - 1]][N - 1];
        if(flag) {
            min_ans = min(min_ans, ans);
        }
    } while(next_permutation(n_vec.begin(), n_vec.end()));

    if(min_ans == 1e9) {
        cout << -1 << endl;
        return;
    }
    cout << min_ans << endl;
}

int main() {
    solve();
    return 0;
}