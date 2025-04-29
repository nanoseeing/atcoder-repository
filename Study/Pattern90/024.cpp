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

    vl A(N), B(N);
    rep(i, 0, N) {
        cin >> A[i];
    }
    rep(i, 0, N) {
        cin >> B[i];
    }

    ll diff = 0;
    rep(i, 0, N) {
        diff += abs(A[i] - B[i]);
    }
    if(diff > K) {
        cout << "No" << endl;
        return;
    }
    if((K - diff) % 2 == 0) {
        cout << "Yes" << endl;
    } else {
        cout << "No" << endl;
    }
}

int main() {
    solve();
    return 0;
}