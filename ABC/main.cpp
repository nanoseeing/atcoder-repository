#include <bits/stdc++.h>
using namespace std;

struct Init {
    Init() {
        ios::sync_with_stdio(0);
        cin.tie(0);
        cout << setprecision(13);
    }
} init;

using ll = long long;
using ull = unsigned long long;
using pii = pair<int, int>;
using pll = pair<ll, ll>;

// ==========================================
int main() {
    ll N, M;
    cin >> N >> M;
    vector<set<ll>> A;
    vector<vector<ll>> is_in_foods(N);
    for(int i : views::iota(0, M)) {
        int K;
        cin >> K;
        vector<ll> a_list;
        for(int k : views::iota(0, K)) {
            int a;
            cin >> a;
            a--;
            is_in_foods[a].push_back(i);
            a_list.push_back(a);
        }
        A.push_back(set<ll>(a_list.begin(), a_list.end()));
    }

    int ans = 0;
    for(int n : views::iota(0, N)) {
        int b;
        cin >> b;
        b--;
        for(const auto &target_food_ind : is_in_foods[b]) {
            A[target_food_ind].erase(b);
            if(A[target_food_ind].empty()) {
                ans++;
            }
        }
        cout << ans << "\n";
    }
    return 0;
}
