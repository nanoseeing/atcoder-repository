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

ll dp[51][1326];

ll dfs(ll white, ll black) {
    if(dp[white][black] != -1) {
        return dp[white][black];
    }

    // white - 1
    unordered_set<ll> grundys;
    if(white > 0) {
        ll next_black = black + white;
        ll next_white = white - 1;
        grundys.emplace(dfs(next_white, next_black));
    }

    // black = black - 1 ~ [black / 2]
    rep(i, black - black / 2, black) {
        ll next_black = i;
        grundys.emplace(dfs(white, next_black));
    }

    // if(white == 0 && black == 3) {
    //     cerr << "grundys: ";
    //     for(auto g : grundys) {
    //         cerr << g << " ";
    //     }
    // }
    rep(i, 0, 51 * 1326) {
        if(!grundys.contains(i)) {
            return dp[white][black] = i;
        }
    }
    assert(false);
}
void solve() {
    ll N;
    cin >> N;
    vl W, B;
    rep(i, 0, N) {
        ll w;
        cin >> w;
        W.push_back(w);
    }
    rep(i, 0, N) {
        ll b;
        cin >> b;
        B.push_back(b);
    }

    rep(white, 0, 51) {
        rep(black, 0, 1326) {
            dp[white][black] = -1;
        }
    }
    dp[0][0] = 0;
    dp[0][1] = 0;

    dfs(50, 50);
    rep(i, 0, 51) {
        dfs(i, 0);
    }

    // cerr << "=====" << endl;
    // rep(i, 0, 10) {
    //     rep(j, 0, 10) {
    //         string display = format("({:1}{:1}){:3}", i, j, dp[i][j]);
    //         cerr << display << " ";
    //     }
    //     cerr << endl;
    // }
    // cerr << "=====" << endl;

    ll xor_grundys = 0;
    rep(i, 0, N) {
        ll grundy = dp[W[i]][B[i]];
        xor_grundys ^= grundy;
    }

    cerr << "=====" << endl;

    if(xor_grundys == 0) {
        cout << "Second" << endl;
    } else {
        cout << "First" << endl;
    }
}

int main() {
    solve();
    return 0;
}