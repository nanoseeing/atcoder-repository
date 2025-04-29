#include <atcoder/modint>
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
using pii = pair<int, int>;
using pll = pair<ll, ll>;
using vl = vector<ll>;
using vvl = vector<vector<ll>>;

#define rep(i, a, b)  for(int i = (a); i < (b); ++i)
#define rrep(i, a, b) for(int i = (a); i >= (b); --i)

 combination(n : usize, r : usize, fact : &[Mint]) -> Mint {
    fact[n] / (fact[r] * fact[n - r])
}

void solve() {
    int N;
    cin >> N;

    vl two_mod(N + 5);
    two_mod[0] = 1;
    for(int i = 1; i <= N + 5; i++) {
        two_mod[i] = (two_mod[i - 1] * 2) % MOD;
    }

    vl sum_two_mod(N + 4);
    sum_two_mod[0] = 0;
    for(int i = 1; i <= N + 4; i++) {
        sum_two_mod[i] = (sum_two_mod[i - 1] + two_mod[i]) % MOD;
    }

    ll all_num = two_mod[N] - 1;
    for(int i = 1; i <= N; i++) {
        if(i == 1) {
            cout << all_num << endl;
            continue;
        }
        int k = i - 1;
        ll x = sum_two_mod[k - 1] - (k - 1) + (N - k + 1) * (two_mod[k] - 1);
        ll ans = (all_num - x) % MOD;
        cout << ans << endl;
    }
}

int main() {
    solve();
    return 0;
}