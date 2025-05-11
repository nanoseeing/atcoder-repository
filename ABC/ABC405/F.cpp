#include <atcoder/segtree>
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

int op(int a, int b) {
    return a + b;
}

int e() {
    return 0;
}

void solve() {
    int N, M, Q;
    cin >> N >> M;
    vector<tuple<int, int, int, int>> all_datas;
    rep(i, 0, M) {
        int a, b;
        cin >> a >> b;
        a--;
        b--;
        all_datas.emplace_back(a, b, 0, -1);
    }

    cin >> Q;
    rep(i, 0, Q) {
        int c, d;
        cin >> c >> d;
        c--;
        d--;
        all_datas.emplace_back(c, d, 1, i);
    }

    vector<int> ans(Q, 0);

    sort(all_datas.begin(), all_datas.end(), [](const auto &a, const auto &b) {
        return get<0>(a) < get<0>(b);
    });
    segtree<int, op, e> seg(N * 2);
    rep(i, 0, (int)all_datas.size()) {
        auto [a, b, t, qi] = all_datas[i];
        if(t == 0) {
            seg.set(b, seg.get(b) + 1);
        } else {
            ans[qi] += seg.prod(a, b);
        }
    }

    sort(all_datas.begin(), all_datas.end(), [](const auto &a, const auto &b) {
        return get<1>(a) > get<1>(b);
    });

    segtree<int, op, e> seg2(N * 2);
    rep(i, 0, (int)all_datas.size()) {
        auto [a, b, t, qi] = all_datas[i];
        if(t == 0) {
            seg2.set(a, seg2.get(a) + 1);
        } else {
            ans[qi] += seg2.prod(a, b);
        }
    }

    rep(i, 0, Q) {
        cout << ans[i] << '\n';
    }
}

int main() {
    solve();
    return 0;
}