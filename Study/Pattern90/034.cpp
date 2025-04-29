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
    ll N, K;
    cin >> N >> K;
    vl A;
    rep(i, 0, N) {
        ll a;
        cin >> a;
        A.push_back(a);
    }

    ll r = 0;
    ll ans = 0;
    ll cnt = 0;
    map<ll, ll> num_map;
    rep(l, 0, N) {
        while(true) {
            if(r == N) {
                // 追加できない
                break;
            }
            if(cnt == K && num_map[A[r]] == 0) {
                // 追加できない
                break;
            }
            if(num_map[A[r]] == 0) {
                // 新しい数を追加
                cnt++;
            }
            num_map[A[r]]++;
            r++;
            ans = max(ans, r - l);
        }
        num_map[A[l]]--;
        if(num_map[A[l]] == 0) {
            // 数がなくなった
            cnt--;
        }
    }

    cout << ans << endl;
}

int main() {
    solve();
    return 0;
}