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
    ll N, M, K;
    cin >> N >> M >> K;

    vl A(N);
    rep(i, 0, N) {
        cin >> A[i];
    }

    multiset<ll> L, R;

    vl tmp;
    rep(i, 0, M) {
        tmp.push_back(A[i]);
    }
    sort(tmp.begin(), tmp.end());

    rep(i, 0, K) {
        L.insert(tmp[i]);
    }
    rep(i, K, M) {
        R.insert(tmp[i]);
    }
    ll total = accumulate(L.begin(), L.end(), 0LL);
    vl ans;
    rep(i, 0, N - M + 1) {
        cpp_dump(i, total, L, R);
        ans.push_back(total);

        ll del_num = A[i];
        if(L.contains(del_num)) {
            L.erase(L.find(del_num));
            total -= del_num;
        } else {
            R.erase(R.find(del_num));
        }

        ll add_num = A[i + M];
        if((int)L.size() < K) {
            R.insert(add_num);
            ll v = R.extract(R.begin()).value();
            L.insert(v);
            total += v;
        } else {
            L.insert(add_num);
            ll v = L.extract(prev(L.end())).value();
            R.insert(v);
            total += add_num - v;
        }
    }

    for(const auto &a : ans) {
        cout << a << " ";
    }
    cout << endl;
}

int main() {
    solve();
    return 0;
}