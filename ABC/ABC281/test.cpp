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

template <typename T>
int my_upper_bound(const std::vector<T> &vec, const T &value) {
    int low = -1;
    int high = vec.size();

    while(low + 1 < high) {
        int mid = (low + high) / 2;
        if(value < vec[mid]) {
            high = mid;
        } else {
            low = mid;
        }
    }

    return high; // この位置が value より大きい最初の要素
}

void solve() {
    // vector<int> x = {1, 2, 3, 4, 5, 7};
    // rep(i, -1, 10) {
    //     auto it = lower_bound(x.begin(), x.end(), i);
    //     cpp_dump(i, it - x.begin());
    // }
    // cpp_dump(x.rbegin(), *x.rbegin(), x.end(), *x.end());

    // multiset<ll> y;

    // y = {7, 1, 2, 3, 4, 5, 1};
    // y.erase(y.find(1));
    // cpp_dump(y);

    // y = {7, 1, 2, 3, 4, 5, 1};
    // y.erase(1);
    // cpp_dump(y);

    // y = {7, 1, 2, 3, 4, 5, 1};
    // cpp_dump(*y.begin(), *y.rbegin(), y.size());

    vector<int> v = {2, 4, 6, 6, 7, 8};
    rep(i, 0, 10) {
        cpp_dump(i, my_upper_bound(v, i));
    }
}

int main() {
    solve();
    return 0;
}