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

void solve() {
    ll N, Q;
    cin >> N >> Q;
    vector<ll> A(N);
    rep(i, 0, N) {
        cin >> A[i];
    }

    ll shift = 0;
    rep(i, 0, Q) {
        ll t, x, y;
        cin >> t >> x >> y;
        x--;
        y--;
        cpp_dump(t, x, y);
        if(t == 1) {
            ll new_x, new_y;
            new_x = (x - shift + N) % N;
            new_y = (y - shift + N) % N;
            cpp_dump(new_x, new_y);
            swap(A[new_x], A[new_y]);
        } else if(t == 2) {
            shift = (shift + 1) % N;
        } else if(t == 3) {
            ll new_x = (x - shift + N) % N;
            cout << A[new_x] << "\n";
        }
    }
}

int main() {
    solve();
    return 0;
}