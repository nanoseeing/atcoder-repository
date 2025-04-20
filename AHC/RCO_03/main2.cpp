#include <bits/stdc++.h>
using namespace std;

const int N = 200;
const double MAX_TIME = 10.95;
// ==========================================

struct Init {
    Init() {
        ios::sync_with_stdio(0);
        cin.tie(0);
        cout << setprecision(13);
    }
} init;

using ll = long long;
using longd = long double;
using ull = unsigned long long;
using pii = pair<int, int>;
using pll = pair<ll, ll>;

// ==========================================

class Xorshift32 {
  public:
    explicit Xorshift32(uint32_t seed = 2463534242)
        : state(seed) {
    }

    uint32_t next() {
        uint32_t x = state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state = x;
        return x;
    }

    uint32_t next(uint32_t max) {
        return next() % max;
    }

    double next_double() {
        return static_cast<double>(next()) / static_cast<double>(UINT32_MAX);
    }

  private:
    uint32_t state;
};

Xorshift32 xor_rng(5555);

// ==========================================

struct Score {
    longd score;
    longd xu;
    longd sum_x;
    longd sum_x2;
};

;
longd score_fomula(longd sum_x, longd sum_x2, longd nu) {
    // Score
    // Σ(xi - xu)^2 / N
    // = Σ(xi^2 + xu^2 - 2 * xi * xu) / N
    // = Σx2 + N * xu * xu - 2 * Σx * xu
    return (sum_x2 + N * nu * nu - 2 * sum_x * nu) / N;
}

longd calc_dist(const vector<pii> &coords, int i, int j) {
    int x1 = coords[i].first;
    int y1 = coords[i].second;
    int x2 = coords[j].first;
    int y2 = coords[j].second;
    return hypot(x1 - x2, y1 - y2);
}

Score calc_score(Score score, vector<longd> del_edges, vector<longd> add_edges) {
    longd next_sum_x = score.sum_x;
    longd next_sum_x2 = score.sum_x2;
    for(longd edge : del_edges) {
        next_sum_x -= edge;
        next_sum_x2 -= edge * edge;
    }
    for(longd edge : add_edges) {
        next_sum_x += edge;
        next_sum_x2 += edge * edge;
    }
    longd next_nu = next_sum_x / N;
    longd next_score = score_fomula(next_sum_x, next_sum_x2, next_nu);

    Score next_score_obj;
    next_score_obj.score = next_score;
    next_score_obj.xu = next_nu;
    next_score_obj.sum_x = next_sum_x;
    next_score_obj.sum_x2 = next_sum_x2;
    return next_score_obj;
}

Score init_score(const vector<int> &ans, const vector<vector<double>> &dist_matrix) {
    longd sum_dists = 0;
    longd sum_dists2 = 0;
    for(int i = 0; i < N; i++) {
        auto dist = dist_matrix[ans[i]][ans[(i + 1) % N]];
        sum_dists += dist;
        sum_dists2 += dist * dist;
    }

    longd nu = sum_dists / N;
    longd score = score_fomula(sum_dists, sum_dists2, nu);
    Score score_obj;
    score_obj.score = score;
    score_obj.xu = nu;
    score_obj.sum_x = sum_dists;
    score_obj.sum_x2 = sum_dists2;
    return score_obj;
}

vector<int> double_bridge(const vector<int> &path) {
    int n = path.size();
    assert(n >= 8); // パス長が短すぎると4分割ができない

    // ランダムに4点を選んでソート（昇順）
    set<int> split_set;
    while(split_set.size() < 4) {
        int idx = xor_rng.next(n - 1) + 1; // 0とnを避ける（セグメント長が0になるのを防ぐ）
        split_set.insert(idx);
    }

    vector<int> split_points(split_set.begin(), split_set.end());
    int a = split_points[0];
    int b = split_points[1];
    int c = split_points[2];
    int d = split_points[3];

    // 元の順序: [0,a) [a,b) [b,c) [c,d) [d,n)
    vector<int> seg1(path.begin(), path.begin() + a);
    vector<int> seg2(path.begin() + a, path.begin() + b);
    vector<int> seg3(path.begin() + b, path.begin() + c);
    vector<int> seg4(path.begin() + c, path.begin() + d);
    vector<int> seg5(path.begin() + d, path.end());

    // 新しい順序（ランダム変更の一例）
    // ここでは 1-4-3-2-5 に変換（double bridge moveの典型的な再結合パターン）
    vector<int> new_path;
    new_path.insert(new_path.end(), seg1.begin(), seg1.end());
    new_path.insert(new_path.end(), seg4.begin(), seg4.end());
    new_path.insert(new_path.end(), seg3.begin(), seg3.end());
    new_path.insert(new_path.end(), seg2.begin(), seg2.end());
    new_path.insert(new_path.end(), seg5.begin(), seg5.end());

    return new_path;
}

tuple<vector<int>, Score> two_opt(const vector<int> &path, Score score, const vector<vector<double>> &dist_matrix, const vector<set<double>> &dist_set) {
    Score score1 = calc_score(score, {}, {});
    for(int i = -1; i < N - 1; i++) {
        for(int j = i + 2; j < N - 1; j++) {
            int u1 = path[(i + N) % N];
            int u2 = path[(i + 1) % N];
            int v1 = path[j];
            int v2 = path[(j + 1) % N];

            // double abs_dist = abs(dist_matrix[u1][u2] - score.xu);
            // double min_search = score.xu - abs_dist;
            // double max_search = score.xu + abs_dist;
            // dist_set[u1].upper_bound(min_search);
            // dist_set[u1].lower_bound(max_search);

            vector<longd> del_edges;
            vector<longd> add_edges;
            add_edges.push_back(dist_matrix[u1][v1]);
            add_edges.push_back(dist_matrix[u2][v2]);
            del_edges.push_back(dist_matrix[u1][u2]);
            del_edges.push_back(dist_matrix[v1][v2]);
            Score score2 = calc_score(score, del_edges, add_edges);

            if(score2.score < score1.score) {
                vector<int> new_path = path;
                reverse(new_path.begin() + i + 1, new_path.begin() + j + 1);
                return {new_path, score2};
            }
        }
    }

    return {path, score1};
}

vector<int> SA(
    vector<int> &x0,
    const vector<pii> &coords,
    longd max_time,
    const vector<vector<double>> &dist_matrix,
    const vector<set<double>> &dist_set,
    bool display) {

    auto st = chrono::steady_clock::now();
    vector<int> x = x0;
    auto best_x = x;

    Score current_score = init_score(x0, dist_matrix);
    Score best_score = current_score;

    cerr << "Initial cost: " << current_score.score << endl;

    int iteration = 0;
    while(true) {
        auto now = chrono::steady_clock::now();
        double elapsed = chrono::duration<double>(now - st).count();
        if(elapsed >= max_time)
            break;

        auto [new_x, new_score] = two_opt(x, current_score, dist_matrix, dist_set);
        if(current_score.score <= new_score.score) {
            // cerr << "not improve" << endl;
            x = double_bridge(x);
            current_score = init_score(x, dist_matrix);
        } else {
            x = new_x;
            current_score = new_score;
        }

        if(current_score.score < best_score.score) {
            best_score = current_score;
            best_x = x;
        }

        iteration++;
        if(display && iteration % 1000 == 0) {
            cerr << "Iteration: " << iteration
                 << ", Current cost: " << current_score.score
                 << ", Best cost: " << best_score.score << endl;
        }
    }

    if(display) {
        cerr << "Final iteration: " << iteration
             << ", Current cost: " << current_score.score
             << ", Best cost: " << best_score.score << endl;
    }

    return best_x;
}

vector<int> calc_init_ans(const vector<vector<double>> &dist_matrix) {

    vector<double> all_dists;
    for(int i = 0; i < N; i++) {
        for(int j = i + 1; j < N; j++) {
            double dist = dist_matrix[i][j];
            all_dists.push_back(dist);
        }
    }

    double ave_dist = 0;
    for(int i = 0; i < all_dists.size(); i++) {
        ave_dist += all_dists[i];
    }
    ave_dist /= all_dists.size();

    int st = 0;
    unordered_map<int, bool> used;
    vector<double> add_dists;
    used[st] = true;
    vector<int> ans;
    ans.push_back(st);

    for(int cnt = 0; cnt < N - 1; cnt++) {
        double min_dist = 1e9;
        double target_dist = 0.0;
        int min_idx = -1;
        if(cnt > 0) {
            ave_dist = 0;
            for(int i = 0; i < add_dists.size(); i++) {
                ave_dist += add_dists[i];
            }
            ave_dist /= add_dists.size();
        }
        for(int j = 0; j < N; j++) {
            if(used.find(j) != used.end()) {
                continue;
            }
            auto pre_v = ans.back();
            double dist = dist_matrix[pre_v][j];
            auto abs_dist = abs(ave_dist - dist);
            if(abs_dist < min_dist) {
                min_dist = abs_dist;
                min_idx = j;
                target_dist = dist;
            }
        }
        ans.push_back(min_idx);
        used[min_idx] = true;
        add_dists.push_back(target_dist);
    }
    return ans;
}

vector<pii> problem_input() {
    int _tmp_n;
    cin >> _tmp_n;

    vector<pii> coordinates;
    for(int i = 0; i < N; i++) {
        int x, y;
        cin >> x >> y;
        coordinates.push_back({x, y});
    }

    return coordinates;
}
int main() {
    auto coordinates = problem_input();

    vector<vector<double>> dist_matrix(N, vector<double>(N));
    vector<set<double>> dist_set(N);
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            double dist = calc_dist(coordinates, i, j);
            dist_matrix[i][j] = dist;
            dist_set[i].insert(dist);
        }
    }

    auto init_ans = calc_init_ans(dist_matrix);
    auto ans = SA(init_ans, coordinates, MAX_TIME, dist_matrix, dist_set, true);

    for(int i = 0; i < N; i++) {
        cout << ans[i] << endl;
    }

    return 0;
}
