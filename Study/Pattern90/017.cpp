#include <atcoder/scc>
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
    ll A, B, C;
    cin >> A >> B >> C;
    ll gcd_num = gcd(gcd(A, B), C);
    A /= gcd_num;
    B /= gcd_num;
    C /= gcd_num;
    ll ans = 0;
    ans = A + B + C - 3;
    cout << ans << endl;
}

int main() {
    solve();
    return 0;
}