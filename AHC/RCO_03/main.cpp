#include <bits/stdc++.h>
using namespace std;

const double T0 = 5.0;
const double T1 = 0.005;
const double MAX_TIME = 500.95;
// ==========================================

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

double exponential_schedule(double init, double obj, double elapsed_time,
                            double max_time) {
    double lambda_param = log(obj / init) / max_time;
    return init * exp(lambda_param * elapsed_time);
}

double linear_schedule(double init, double obj, double elapsed_time, double max_time) {
    return init + (obj - init) * (elapsed_time / max_time);
}

double calc_var(vector<double> &x) {
    double mean = 0;
    for(int i = 0; i < x.size(); i++) {
        mean += x[i];
    }
    mean /= x.size();

    double var = 0;
    for(int i = 0; i < x.size(); i++) {
        var += (x[i] - mean) * (x[i] - mean);
    }
    var /= x.size();
    return var;
}

double calc_score(vector<int> &ans, const vector<pii> &coords) {
    int score = 0;
    vector<double> dists;
    for(int i = 0; i < ans.size(); i++) {
        int x1 = coords[ans[i]].first;
        int y1 = coords[ans[i]].second;
        int x2 = coords[ans[(i + 1) % ans.size()]].first;
        int y2 = coords[ans[(i + 1) % ans.size()]].second;
        double dist = hypot(x1 - x2, y1 - y2);
        dists.push_back(dist);
    }
    return calc_var(dists);
}

vector<int> swap_neighbor(const vector<int> &path) {
    int N = path.size();
    if(N < 2)
        return path;

    int i = xor_rng.next(N);
    int j = xor_rng.next(N);
    while(i == j)
        j = xor_rng.next(N);

    vector<int> new_path = path;
    swap(new_path[i], new_path[j]);

    return new_path;
}

vector<int> insertion_neighbor(const vector<int> &path) {
    int N = path.size();
    if(N < 2)
        return path;

    int i = xor_rng.next(N);
    int j = xor_rng.next(N);
    while(i == j)
        j = xor_rng.next(N);

    vector<int> new_path = path;
    int city = new_path[i];
    new_path.erase(new_path.begin() + i);

    if(i < j)
        j--; // iが先に削除されたため調整が必要
    new_path.insert(new_path.begin() + j, city);

    return new_path;
}

vector<int> two_opt(const vector<int> &path) {
    int N = path.size();
    if(N < 4)
        return path;

    int i = xor_rng.next(N);
    int j = xor_rng.next(N);
    while(abs(i - j) < 2)
        j = xor_rng.next(N);

    if(i > j)
        swap(i, j);

    vector<int> new_path = path;
    reverse(new_path.begin() + i, new_path.begin() + j);

    return new_path;
}

vector<int> three_opt(const vector<int> &path) {
    int N = path.size();
    if(N < 6)
        return path;

    int a = xor_rng.next(N);
    int b = xor_rng.next(N);
    int c = xor_rng.next(N);
    vector<int> idx = {a, b, c};
    sort(idx.begin(), idx.end());
    a = idx[0], b = idx[1], c = idx[2];

    // a < b < c の3点を選び、3つの区間に分割
    vector<int> new_path;
    new_path.insert(new_path.end(), path.begin(), path.begin() + a);
    new_path.insert(new_path.end(), path.begin() + b, path.begin() + c); // b〜cを先に
    new_path.insert(new_path.end(), path.begin() + a, path.begin() + b); // a〜bを後に
    new_path.insert(new_path.end(), path.begin() + c, path.end());

    return new_path;
}

vector<int> or_opt(const vector<int> &path, int k = 1) {
    int N = path.size();
    if(N < k + 2)
        return path;

    int from = xor_rng.next(N - k + 1);

    int to = xor_rng.next(N - k);
    while(to >= from && to <= from + k - 1) {
        to = xor_rng.next(N - k);
    }

    vector<int> new_path = path;
    vector<int> segment(new_path.begin() + from, new_path.begin() + from + k);
    new_path.erase(new_path.begin() + from, new_path.begin() + from + k);

    if(to > from)
        to -= k; // erase済みの分だけ調整
    new_path.insert(new_path.begin() + to, segment.begin(), segment.end());

    return new_path;
}

vector<int> neightbor(vector<int> &ans) {

    vector<int> tmp = ans;

    int N = ans.size();

    double p = xor_rng.next_double();
    if(p < 0.05) {
        // 2-opt
        return two_opt(tmp);
    } else if(p < 0.3) {
        // 3-opt
        return three_opt(tmp);
    } else if(p < 0.5) {
        // swap
        return swap_neighbor(tmp);
    } else if(p < 0.7) {
        // insertion
        return insertion_neighbor(tmp);
    } else {
        // or-opt
        return or_opt(tmp, xor_rng.next(4) + 1);
    }
}

vector<int> SA(
    vector<int> &x0,
    double t0,
    double t1,
    const vector<pii> &coords,
    double max_time,
    bool display) {

    auto st = chrono::steady_clock::now();

    vector<int> x = x0;

    double current_cost = calc_score(x0, coords);
    auto best_x = x;
    double best_cost = current_cost;

    int iteration = 0;
    while(true) {
        auto now = chrono::steady_clock::now();
        double elapsed = chrono::duration<double>(now - st).count();
        if(elapsed >= max_time)
            break;

        double temp = exponential_schedule(t0, t1, elapsed, max_time);
        vector<int> new_x = neightbor(x);

        double new_cost = calc_score(new_x, coords);

        double delta_cost = new_cost - current_cost;
        if(delta_cost < 0 || xor_rng.next_double() < exp(-delta_cost / temp)) {
            current_cost = new_cost;
            x = new_x;
        }

        if(current_cost < best_cost) {
            best_cost = current_cost;
            best_x = x;
        }

        iteration++;
        if(display && iteration % 10000 == 0) {
            cerr << "Iteration: " << iteration
                 << ", Current cost: " << current_cost
                 << ", Best cost: " << best_cost
                 << ", Temp: " << temp << endl;
        }
    }

    if(display) {
        cerr << "Final iteration: " << iteration
             << ", Current cost: " << current_cost
             << ", Best cost: " << best_cost << endl;
    }

    return best_x;
}

vector<int> calc_init_ans(const vector<pii> &coords) {

    vector<double> all_dists;
    for(int i = 0; i < coords.size(); i++) {
        for(int j = i + 1; j < coords.size(); j++) {
            double dist = hypot(coords[i].first - coords[j].first,
                                coords[i].second - coords[j].second);
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

    for(int cnt = 0; cnt < coords.size() - 1; cnt++) {
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
        for(int j = 0; j < coords.size(); j++) {
            if(used.find(j) != used.end()) {
                continue;
            }
            auto pre_v = ans.back();
            auto c1 = coords[pre_v];
            auto c2 = coords[j];
            double dist = hypot(c1.first - c2.first, c1.second - c2.second);
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

int main() {

    int N;
    vector<pii> coordinates;
    cin >> N;

    for(int i = 0; i < N; i++) {
        int x, y;
        cin >> x >> y;
        coordinates.push_back({x, y});
    }

    // vector<int> init_ans;
    // for(int i = 0; i < N; i++) {
    //     init_ans.push_back(i);
    // }

    vector<int> init_ans = calc_init_ans(coordinates);

    double now_cost = calc_score(init_ans, coordinates);
    double t0 = now_cost / N;
    cerr << "now_cost: " << now_cost << endl;
    cerr << "t0: " << t0 << endl;

    auto ans = SA(init_ans, T0, T1, coordinates, MAX_TIME, true);

    for(int i = 0; i < N; i++) {
        cout << ans[i] << endl;
    }

    return 0;
}
