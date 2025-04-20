#include <bits/stdc++.h>
using namespace std;

const int N = 200;
const int M = N * (N - 1) / 2;
const double MAX_TIME = 100.95;
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

struct Path {
    int node;
    int prev;
    int next;
};

vector<int> construct_tour(const vector<Path> &path) {
    vector<int> tour;
    tour.push_back(0);
    int sv = 0;
    for(const int i : views::iota(0, N + 100)) {
        int next = path[sv].next;
        if(next == 0)
            break;
        tour.push_back(next);
        sv = next;
    }

    assert(tour.size() == N);
    return tour;
}

vector<Path> construct_path(const vector<int> &tour) {
    vector<Path> path(N);
    for(int i = 0; i < N; i++) {
        path[tour[i]].node = tour[i];
        path[tour[i]].prev = tour[(i - 1 + N) % N];
        path[tour[i]].next = tour[(i + 1) % N];
    }
    sort(path.begin(), path.end(), [](const Path &a, const Path &b) {
        return a.node < b.node;
    });
    return path;
}

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

Score init_score(const vector<Path> &paths, const vector<vector<double>> &dist_matrix) {

    longd sum_dists = 0;
    longd sum_dists2 = 0;
    auto vs = construct_tour(paths);
    for(int i = 0; i < N; i++) {
        auto dist = dist_matrix[vs[i]][vs[(i + 1) % N]];
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

vector<int> double_bridge(const vector<int> &tour) {
    set<int> split_set;
    while(split_set.size() < 4) {
        int idx = xor_rng.next(N - 1) + 1; // 0とnを避ける（セグメント長が0になるのを防ぐ）
        split_set.insert(idx);
    }

    vector<int> split_points(split_set.begin(), split_set.end());
    int a = split_points[0];
    int b = split_points[1];
    int c = split_points[2];
    int d = split_points[3];

    // 元の順序: [0,a) [a,b) [b,c) [c,d) [d,n)
    vector<int> seg1(tour.begin(), tour.begin() + a);
    vector<int> seg2(tour.begin() + a, tour.begin() + b);
    vector<int> seg3(tour.begin() + b, tour.begin() + c);
    vector<int> seg4(tour.begin() + c, tour.begin() + d);
    vector<int> seg5(tour.begin() + d, tour.end());

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

vector<Path> double_bridge(const vector<Path> &path) {
    auto tour = construct_tour(path);
    auto new_tour = double_bridge(tour);
    auto new_path = construct_path(new_tour);
    return new_path;
}

vector<int> two_opt(const vector<int> &path, int l, int r) {
    vector<int> new_path = path;
    reverse(new_path.begin() + l, new_path.begin() + r);
    return new_path;
}

vector<Path> two_opt(const vector<Path> &path, int u1, int u2, int v1, int v2) {
    assert(path[u1].next == u2);
    assert(path[v1].next == v2);

    auto new_path = path;
    int sv = new_path[v1].prev;

    while(sv != u2) {
        swap(new_path[sv].prev, new_path[sv].next);
        sv = new_path[sv].next;
    }

    new_path[u1].next = v1;
    new_path[v2].prev = u2;
    new_path[v1].next = new_path[v1].prev;
    new_path[v1].prev = u1;
    new_path[u2].prev = new_path[u2].next;
    new_path[u2].next = v2;

    return new_path;
}

tuple<vector<Path>, Score> two_opt(const vector<Path> &path, Score score, const vector<vector<double>> &dist_matrix, const vector<set<pair<double, int>>> &dist_set) {
    Score score1 = calc_score(score, {}, {});
    for(int u1 = 0; u1 < N; u1++) {
        int u2 = path[u1].next;

        double abs_dist = abs(dist_matrix[u1][u2] - score.xu);
        double min_search = score.xu - abs_dist;
        double max_search = score.xu + abs_dist;
        auto it1 = dist_set[u1].lower_bound({min_search, 0});
        auto it2 = dist_set[u1].lower_bound({max_search, 0});
        if(it1 == it2) {
            continue;
        }

        for(auto it = it1; it != it2; ++it) {
            int v1 = it->second;
            int v2 = path[v1].next;
            if(v1 == u1 || v1 == u2 || v2 == u1 || v2 == u2) {
                continue;
            }

            vector<longd> del_edges;
            vector<longd> add_edges;
            add_edges.push_back(dist_matrix[u1][v1]);
            add_edges.push_back(dist_matrix[u2][v2]);
            del_edges.push_back(dist_matrix[u1][u2]);
            del_edges.push_back(dist_matrix[v1][v2]);
            Score score2 = calc_score(score, del_edges, add_edges);

            if(score2.score < score1.score) {
                auto new_path = two_opt(path, u1, u2, v1, v2);
                return {new_path, score2};
            }
        }
    }
    return {path, score1};
}

vector<Path> SA(
    vector<Path> &x0,
    longd max_time,
    const vector<vector<double>> &dist_matrix,
    vector<set<pair<double, int>>> &dist_set,
    bool display) {

    auto st = chrono::steady_clock::now();
    auto x = x0;
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

vector<Path> calc_init_ans(const vector<vector<double>> &dist_matrix) {

    double ave_dist = 0;
    for(int i = 0; i < N; i++) {
        for(int j = i + 1; j < N; j++) {
            ave_dist += dist_matrix[i][j];
        }
    }
    ave_dist /= M;

    int st = 0;
    unordered_map<int, bool> used;
    double sum_dists = 0;
    used[st] = true;

    vector<int> init_x;
    init_x.push_back(st);

    for(int cnt = 0; cnt < N - 1; cnt++) {
        double min_dist = 1e9;
        double target_dist = 0.0;
        int min_idx = -1;
        if(cnt > 0) {
            ave_dist = sum_dists / cnt;
        }
        ave_dist = 250;
        for(int j = 0; j < N; j++) {
            if(used[j]) {
                continue;
            }
            auto pre_v = init_x.back();
            double dist = dist_matrix[pre_v][j];
            auto abs_dist = abs(ave_dist - dist);
            if(abs_dist < min_dist) {
                min_dist = abs_dist;
                min_idx = j;
                target_dist = dist;
            }
        }
        init_x.push_back(min_idx);
        used[min_idx] = true;
        sum_dists += target_dist;
    }
    return construct_path(init_x);
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

void problem_output(const vector<Path> &ans) {
    auto ans_v = construct_tour(ans);
    for(int i = 0; i < N; i++) {
        cout << ans_v[i] << endl;
    }
}
int main() {
    auto coordinates = problem_input();

    vector<vector<double>> dist_matrix(N, vector<double>(N));
    vector<set<pair<double, int>>> dist_set(N);
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            double dist = calc_dist(coordinates, i, j);
            dist_matrix[i][j] = dist;
            dist_set[i].insert({dist, j});
        }
    }

    auto init_x = calc_init_ans(dist_matrix);
    auto ans = SA(init_x, MAX_TIME, dist_matrix, dist_set, true);
    problem_output(ans);

    return 0;
}
