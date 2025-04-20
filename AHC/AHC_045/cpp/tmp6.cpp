#pragma GCC target("avx")
#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")

#include <bits/stdc++.h>

// =================================
// テンプレート
// =================================
using namespace std;
using ll = long long;
using ull = unsigned long long;
using pii = pair<int, int>;
using pll = pair<ll, ll>;
using Segment = pair<pii, pii>;

struct Init {
    Init() {
        ios::sync_with_stdio(0);
        cin.tie(0);
    }
} init;

struct pair_hash {
    size_t operator()(const pii &p) const {
        return hash<int>()(p.first) ^ (hash<int>()(p.second) << 1);
    }
};
// =================================
// 設定
// =================================
const bool IS_ONLINE_JUDGE = false;

const int SOLVE_L_THREAD = 6; // < SOLVE_L_THREADなら点推定しない

// 点推定
const double MAX_TIME = 0.7;
const int TRIAL_NUM = 1;
const double T0 = 10.0;
const double T1 = 0.5;
const double INIT_MUL_X = 0.5;
const int DIVIDE_TIME = 1000;
const int DISPLAY_CNT = 10000;
const float MEMO_START_MUL = 3 / 4;
const int MUL_X_CNT = 100;

// グループ推定共通
const int MAX_G_NUM = 5;
const int GROUP_SA_DISPLAY_CNT = 1000;

// グループ推定（post）
const bool IS_POST_SA = true;
const double POST_OPT_GRUOP_TIME = 1.0;
const double POST_OPT_T0 = 10.0;
const double POST_OPT_T1 = 0.5;

// グループ推定（オンリー）
const double GROUP_T0 = 1000.0;
const double GROUP_T1 = 1.0;
const double GROUP_MAX_TIME = 10.0;

// const double MAX_TIME = 1.63;
// const int TRIAL_NUM = 1;
// const double T0 = 10.0;
// const double T1 = 1.0;
// const double INIT_MUL_X = 0.5;
// const int DIVIDE_TIME = 1000;
// const int DISPLAY_CNT = 200000;
// const float MEMO_START_MUL = 3 / 4;
// const int MUL_X_CNT = 100;

// const bool IS_POST_SA = true;
// const double POST_OPT_GRUOP_TIME = 20.1;
// const double POST_OPT_T0 = 5.0;
// const double POST_OPT_T1 = 0.1;

// // グループ推定（オンリー）
// const double GROUP_T0 = 5.0;
// const double GROUP_T1 = 0.1;
// const double GROUP_MAX_TIME = 1.80;

bool DEBUG = false;
int DEBUG_QUERY = -1;
int DEBUG_L = -1;
const int START_FILE_NUM = 71;
const int END_FILE_NUM = 72;

// =================================
// 初期化
// =================================
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<> rand01(0.0, 1.0);

const int MAX_COST_UPDATE = 10000;
int cost_update_arr[MAX_COST_UPDATE];
int cost_update_id[MAX_COST_UPDATE];
int x_estimate_cost[800];

// =================================
// ユーティリティ
// =================================
template <typename... Args>
void debug_print(Args... args) {
    if(IS_ONLINE_JUDGE)
        return;
    ((cout << args << " "), ...) << endl;
}

pii sort_pair(pii ab) {
    return ab.first < ab.second ? ab : make_pair(ab.second, ab.first);
}

// =================================
// 幾何計算
// =================================
double euclidean_distance(pii a, pii b) {
    return sqrt(pow(a.first - b.first, 2) + pow(a.second - b.second, 2));
}

double calc_triangle_score(pii p1, pii p2, pii p3) {
    double d1 = euclidean_distance(p1, p2);
    double d2 = euclidean_distance(p2, p3);
    double d3 = euclidean_distance(p3, p1);
    double mean = (d1 + d2 + d3) / 3.0;
    double variance = ((d1 - mean) * (d1 - mean) +
                       (d2 - mean) * (d2 - mean) +
                       (d3 - mean) * (d3 - mean)) /
                      3.0;
    return sqrt(variance);
}

bool is_point_in_polygon(const vector<pii> &polygon, const pii &coord) {
    int px = coord.first;
    int py = coord.second;

    bool is_inside = false;
    int n = polygon.size();

    for(int i = 0; i < n; ++i) {
        int x1 = polygon[i].first;
        int y1 = polygon[i].second;
        int x2 = polygon[(i + 1) % n].first;
        int y2 = polygon[(i + 1) % n].second;

        if(min(y1, y2) < py && py <= max(y1, y2)) {
            double x_intersect = (py - y1) * (x2 - x1) / (double(y2 - y1) + 1e-10) + x1;
            if(px < x_intersect) {
                is_inside = !is_inside;
            }
        }
    }

    return is_inside;
}

int orientation(const pii &p, const pii &q, const pii &r) {
    double val = (q.second - p.second) * (r.first - q.first) -
                 (q.first - p.first) * (r.second - q.second);
    if(abs(val) < 1e-10)
        return 0;             // colinear
    return (val > 0) ? 1 : 2; // 1: clockwise, 2: counterclockwise
}

bool on_segment(const pii &p, const pii &q, const pii &r) {
    return min(p.first, r.first) <= q.first && q.first <= max(p.first, r.first) &&
           min(p.second, r.second) <= q.second && q.second <= max(p.second, r.second);
}

bool do_intersect(const Segment &s1, const Segment &s2) {
    pii p1 = s1.first, q1 = s1.second;
    pii p2 = s2.first, q2 = s2.second;

    // Check for shared point (not treated as intersection unless overlapping)
    set<pii> s1_points = {p1, q1};
    set<pii> s2_points = {p2, q2};
    vector<pii> shared;
    set_intersection(
        s1_points.begin(), s1_points.end(),
        s2_points.begin(), s2_points.end(),
        back_inserter(shared));
    if(shared.size() == 1)
        return false;

    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    // General case
    if(o1 != o2 && o3 != o4)
        return true;

    // Special colinear cases
    if(o1 == 0 && on_segment(p1, p2, q1))
        return true;
    if(o2 == 0 && on_segment(p1, q2, q1))
        return true;
    if(o3 == 0 && on_segment(p2, p1, q2))
        return true;
    if(o4 == 0 && on_segment(p2, q1, q2))
        return true;

    return false;
}

bool any_intersection(const vector<pii> &polygon, const Segment &target_edge) {
    int n = polygon.size();
    for(int i = 0; i < n; ++i) {
        Segment edge = {polygon[i], polygon[(i + 1) % n]};
        if(do_intersect(edge, target_edge)) {
            return true;
        }
    }
    return false;
}
vector<float> divide_interval(float l, float r, int n, bool is_exclude_edge = false) {
    vector<float> result;
    if(is_exclude_edge) {
        float step = (r - l) / (n + 1);
        for(int i = 0; i < n; ++i) {
            result.push_back(l + (i + 1) * step);
        }
    } else {
        float step = (r - l) / (n - 1);
        for(int i = 0; i < n; ++i) {
            result.push_back(l + i * step);
        }
    }
    return result;
}
int calc_dist(pii a, pii b) {
    int dx = a.first - b.first;
    int dy = a.second - b.second;
    return sqrt(dx * dx + dy * dy);

    // int s = dx * dx + dy * dy;
    // return s < SQRT_MEMO_SIZE ? sqrt_memo[s] : sqrt(s);
}

int calc_dist_not_sqrt(pii a, pii b) {
    int dx = a.first - b.first;
    int dy = a.second - b.second;
    return (dx * dx + dy * dy);
}

pii calc_centroid(const vector<pii> &points) {
    long long x_sum = 0, y_sum = 0;
    int n = points.size();

    for(const auto &[x, y] : points) {
        x_sum += x;
        y_sum += y;
    }

    return {static_cast<int>(x_sum / n), static_cast<int>(y_sum / n)};
}
// =================================
// グラフアルゴリズム
// =================================
pair<unordered_set<int>, unordered_set<int>>
cut_graph(const unordered_map<int, vector<int>> &graph,
          pii cut_edge) {
    auto bfs = [](const unordered_map<int, vector<int>> &graph, int init_v,
                  int not_visit_v) {
        unordered_set<int> visited = {init_v, not_visit_v};
        deque<int> q = {init_v};

        while(!q.empty()) {
            int node = q.back();
            q.pop_back();
            for(int nv : graph.at(node)) {
                if(!visited.count(nv)) {
                    visited.insert(nv);
                    q.emplace_back(nv);
                }
            }
        }
        visited.erase(not_visit_v);
        return visited;
    };

    int v1 = cut_edge.first, v2 = cut_edge.second;
    return {bfs(graph, v1, v2), bfs(graph, v2, v1)};
}

vector<vector<int>>
construct_dist_matrix(const vector<pii> &points) {
    int N = points.size();
    vector<vector<int>> dist_matrix(N, vector<int>(N, 0));

    for(int i = 0; i < N; ++i) {
        for(int j = i + 1; j < N; ++j) {
            int dist = calc_dist(points[i], points[j]);
            dist_matrix[i][j] = dist;
            dist_matrix[j][i] = dist;
        }
    }
    return dist_matrix;
}

unordered_map<int, vector<pii>>
construct_graph(const unordered_map<int, pii> &points) {
    unordered_map<int, vector<pii>> graph;
    for(const auto &[v, _] : points) {
        graph[v] = {};
    }

    for(const auto &[v1, p1] : points) {
        for(const auto &[v2, p2] : points) {
            if(v1 == v2)
                continue;
            int dist = calc_dist(p1, p2);
            graph[v1].emplace_back(dist, v2);
        }
    }

    for(auto &[v, neighbors] : graph) {
        sort(neighbors.begin(), neighbors.end()); // by distance
    }

    return graph;
}

unordered_map<int, vector<int>>
construct_graph_from_edges(const vector<pii> &edges,
                           const vector<int> &vs) {
    unordered_map<int, vector<int>> graph;
    for(int v : vs) {
        graph[v] = {};
    }
    for(const auto &[v1, v2] : edges) {
        graph[v1].emplace_back(v2);
        graph[v2].emplace_back(v1);
    }
    return graph;
}

vector<tuple<int, int, int>>
construct_sorted_edges(const unordered_map<int, pii> &coords) {
    vector<tuple<int, int, int>> edges;
    for(const auto &[v1, p1] : coords) {
        for(const auto &[v2, p2] : coords) {
            if(v1 == v2)
                continue;
            int dist = calc_dist(p1, p2);
            edges.emplace_back(dist, v1, v2);
        }
    }
    sort(edges.begin(), edges.end());
    return edges;
}

// Union-Find（Disjoint Set Union）
class UnionFind {
  private:
    unordered_map<int, int> parent;

  public:
    UnionFind(const vector<int> &vs) {
        for(int v : vs) {
            parent[v] = -1;
        }
    }

    int find(int x) {
        if(parent[x] < 0)
            return x;
        return parent[x] = find(parent[x]);
    }

    void unite(int x, int y) {
        x = find(x);
        y = find(y);
        if(x == y)
            return;

        if(parent[x] > parent[y])
            swap(x, y);
        parent[x] += parent[y];
        parent[y] = x;
    }

    bool same(int x, int y) {
        return find(x) == find(y);
    }

    int size(int x) {
        return -parent[find(x)];
    }

    vector<int> members(int x) {
        int root = find(x);
        vector<int> res;
        for(const auto &[k, _] : parent) {
            if(find(k) == root)
                res.emplace_back(k);
        }
        return res;
    }

    vector<int> roots() {
        vector<int> res;
        for(const auto &[k, v] : parent) {
            if(v < 0)
                res.emplace_back(k);
        }
        return res;
    }

    int group_count() {
        return roots().size();
    }

    unordered_map<int, vector<int>> all_group_members() {
        unordered_map<int, vector<int>> res;
        for(const auto &[k, _] : parent) {
            int r = find(k);
            res[r].emplace_back(k);
        }
        return res;
    }
};

// クラスカル法による最小全域木
pair<vector<pii>, int>
kruskals_algorithm(vector<tuple<int, int, int>> edges, const vector<int> &vs) {
    UnionFind uf(vs);
    int cost = 0;
    vector<pii> ans_edges;

    for(const auto &[c, a, b] : edges) {
        if(!uf.same(a, b)) {
            cost += c;
            uf.unite(a, b);
            ans_edges.emplace_back(sort_pair({a, b}));
        }
    }

    return {ans_edges, cost};
}

tuple<vector<pii>, vector<int>, int>
prim_k(const vector<vector<int>> &graph, int init_v, int k,
       const unordered_set<int> &used_v, int N) {
    unordered_set<int> used = used_v;
    used.insert(init_v);

    priority_queue<tuple<int, int, int>, vector<tuple<int, int, int>>, greater<>>
        pq;
    for(int v = 0; v < N; ++v) {
        pq.emplace(graph[init_v][v], init_v, v);
    }

    vector<int> ans_v = {init_v};
    vector<pii> ans_edges;
    int ans_cost = 0;

    while(!pq.empty() && ans_v.size() < k) {
        auto [cost, from_v, to_v] = pq.top();
        pq.pop();

        if(used.count(to_v))
            continue;
        used.insert(to_v);
        ans_edges.emplace_back(sort_pair({from_v, to_v}));
        ans_v.emplace_back(to_v);
        ans_cost += cost;

        for(int nxt = 0; nxt < N; ++nxt) {
            if(!used.count(nxt)) {
                pq.emplace(graph[to_v][nxt], to_v, nxt);
            }
        }
    }

    return {ans_edges, ans_v, ans_cost};
}

pair<vector<pii>, int> prim_vs(
    const vector<vector<int>> &graph,
    const vector<int> &vs) {
    if(vs.empty())
        return {{}, 0};

    int init_v = vs[0];
    unordered_set<int> used = {init_v};

    priority_queue<tuple<int, int, int>, vector<tuple<int, int, int>>, greater<>> que;
    for(int v : vs) {
        que.emplace(graph[init_v][v], init_v, v);
    }

    vector<pii> ans_edges;
    int ans_cost = 0;

    while(!que.empty() && ans_edges.size() < vs.size() - 1) {
        auto [cost_v, from_v, to_v] = que.top();
        que.pop();

        if(used.count(to_v))
            continue;

        used.insert(to_v);
        ans_edges.emplace_back(sort_pair({from_v, to_v}));
        ans_cost += cost_v;

        for(int nxt : vs) {
            if(!used.count(nxt)) {
                que.emplace(graph[to_v][nxt], to_v, nxt);
            }
        }
    }

    return {ans_edges, ans_cost};
}

// =================================
// 焼きなまし
// =================================

// 指数スケジュール: 時間に基づいて指数的に上昇または減衰
double exponential_schedule(double init, double obj, double elapsed_time,
                            double max_time) {
    double lambda_param = log(obj / init) / max_time;
    return init * exp(lambda_param * elapsed_time);
}

// 線形スケジュール: 時間に基づいて線形に変化
double linear_schedule(double init, double obj, double elapsed_time,
                       double max_time) {
    return init + (obj - init) * (elapsed_time / max_time);
}

// =================================
// 環境クラス (抽象クラス)
// =================================
class Env {
  public:
    int N, M, Q, L, W;
    vector<int> G;
    vector<vector<int>> rectangles;
    vector<pii> center_points;
    vector<vector<pii>> query_history;

    virtual vector<pii> query(const vector<int> &c_list) = 0;
    virtual int answer(const vector<vector<int>> &groups,
                       const vector<vector<pii>> &edges) = 0;

    void read_first_line(const string &line) {
        stringstream ss(line);
        ss >> N >> M >> Q >> L >> W;
    }

    void read_second_line(const vector<string> &lines) {
        stringstream ss(lines[0]);
        G.assign(istream_iterator<int>(ss), istream_iterator<int>());

        for(int i = 1; i < lines.size(); ++i) {
            vector<int> rect;
            stringstream ss_line(lines[i]);
            int val;
            while(ss_line >> val)
                rect.emplace_back(val);
            rectangles.emplace_back(rect);
        }

        for(const auto &rect : rectangles) {
            int cx = (rect[0] + rect[1]) / 2;
            int cy = (rect[2] + rect[3]) / 2;
            center_points.emplace_back(cx, cy);
        }
    }
};

// =================================
// オンライン判定用クラス
// =================================
class EnvOnline : public Env {
  public:
    EnvOnline() {
        string line;
        getline(cin, line);
        read_first_line(line);

        vector<string> lines(N + 1);
        for(int i = 0; i < N + 1; ++i) {
            getline(cin, lines[i]);
        }
        read_second_line(lines);
    }

    vector<pii> query(const vector<int> &c_list) override {
        cout << "? " << c_list.size();
        for(int v : c_list)
            cout << " " << v;
        cout << endl;

        vector<pii> res;
        for(int i = 0; i < c_list.size() - 1; ++i) {
            int a, b;
            cin >> a >> b;
            res.emplace_back(a, b);
        }

        query_history.emplace_back(res);
        return res;
    }

    int answer(const vector<vector<int>> &groups,
               const vector<vector<pii>> &edges) override {
        cout << "!" << endl;
        for(int i = 0; i < groups.size(); ++i) {
            for(int v : groups[i])
                cout << v << " ";
            cout << endl;
            for(auto [u, v] : edges[i]) {
                cout << u << " " << v << endl;
            }
        }
        return 0;
    }
};

// =================================
// オフライン判定用クラス
// =================================
class EnvOffline : public Env {
  private:
    string input_file_path, output_file_path;

  public:
    vector<pii> coordinates;
    EnvOffline(const string &input_path, const string &output_path,
               bool dbg = false, int dbg_L = -1, int dbg_Q = -1)
        : input_file_path(input_path), output_file_path(output_path) {

        ifstream fin(input_file_path);
        string line;
        getline(fin, line);
        read_first_line(line);

        vector<string> lines(N + 1);
        for(int i = 0; i < N + 1; ++i) {
            getline(fin, lines[i]);
        }
        read_second_line(lines);

        for(int i = 0; i < N; ++i) {
            int x, y;
            fin >> x >> y;
            coordinates.emplace_back(x, y);
        }

        if(dbg) {
            if(dbg_L > 0)
                L = dbg_L;
            if(dbg_Q > 0)
                Q = dbg_Q;
        }
    }

    vector<pii> query(const vector<int> &c_list) override {
        stringstream ss;
        ss << "? " << c_list.size();
        for(int v : c_list)
            ss << " " << v;

        // 必要な座標だけ抽出
        unordered_map<int, pii> subset_coords;
        for(int v : c_list) {
            subset_coords[v] = coordinates[v];
        }

        auto sorted_edges = construct_sorted_edges(subset_coords);
        auto [ans_edges, _] = kruskals_algorithm(sorted_edges, c_list);
        query_history.emplace_back(ans_edges);

        return ans_edges;
    }

    int answer(const vector<vector<int>> &groups,
               const vector<vector<pii>> &edges) override {
        int cost = 0;
        ofstream fout(output_file_path);
        fout << "!" << endl;

        for(int i = 0; i < groups.size(); ++i) {
            for(int v : groups[i])
                fout << v << " ";
            fout << endl;
            for(auto [u, v] : edges[i]) {
                fout << u << " " << v << endl;
                cost += calc_dist(coordinates[u], coordinates[v]);
            }
        }

        // for(const string &q : query_history) {
        //     fout << q << endl;
        // }

        // cout << "query: " << query_history.size() << endl;
        return cost;
    }
};

// =================================
// 共通
// =================================
pair<vector<int>, vector<pii>>
construct_triangle(
    Env &env,
    int random_v,
    const unordered_map<int, vector<pii>> &all_graph) {
    const auto &neighbors = all_graph.at(random_v);

    vector<int> neighbor_v_nums;
    for(const auto &[_, v] : neighbors) {
        neighbor_v_nums.push_back(v);
        if(neighbor_v_nums.size() >= 10)
            break;
    }

    uniform_int_distribution<> dist(0, neighbor_v_nums.size() - 1);
    int v1 = neighbor_v_nums[dist(gen)];

    double best_score = numeric_limits<double>::infinity();
    pii now_p = env.center_points[random_v];

    int best_v2 = -1;
    for(int v2 : neighbor_v_nums) {
        if(v2 == v1)
            continue;

        auto p1 = env.center_points[v1];
        auto p2 = env.center_points[v2];

        double t_score = calc_triangle_score(now_p, p1, p2);
        if(t_score < best_score) {
            best_score = t_score;
            best_v2 = v2;
        }
    }

    vector<int> polygon_vs = {random_v, v1, best_v2};
    vector<pii> polygon_coords = {
        env.center_points[random_v],
        env.center_points[v1],
        env.center_points[best_v2]};

    return {polygon_vs, polygon_coords};
}

pair<vector<int>, vector<pii>> construct_droneyard(
    Env &env,
    vector<int> polygon_vs,
    vector<pii> polygon_coords) {
    vector<int> ret_polygon_vs = polygon_vs;
    vector<pii> ret_polygon_coords = polygon_coords;

    unordered_set<int> ok_vs;
    for(int i = 0; i < env.N; ++i) {
        if(find(polygon_vs.begin(), polygon_vs.end(), i) == polygon_vs.end()) {
            ok_vs.insert(i);
        }
    }

    for(int step = 0; step < env.L - 3; ++step) {
        vector<int> next_vs;
        unordered_set<int> no_vs;

        for(int v : ok_vs) {
            if(is_point_in_polygon(ret_polygon_coords, env.center_points[v])) {
                no_vs.insert(v);
            } else {
                next_vs.push_back(v);
            }
        }

        for(int nv : no_vs) {
            ok_vs.erase(nv);
        }

        int now_polygon_num = ret_polygon_coords.size();
        vector<int> rand_polygon_inds(now_polygon_num);
        iota(rand_polygon_inds.begin(), rand_polygon_inds.end(), 0);
        shuffle(rand_polygon_inds.begin(), rand_polygon_inds.end(), gen);

        int add_v = -1;
        int add_ind = -1;

        for(int polygon_i : rand_polygon_inds) {
            vector<tuple<double, int, int>> memos;

            for(int next_v : next_vs) {
                pii p1 = ret_polygon_coords[polygon_i];
                pii p2 = ret_polygon_coords[(polygon_i + 1) % now_polygon_num];
                pii target_p = env.center_points[next_v];

                if(any_intersection(ret_polygon_coords, {p1, target_p}) ||
                   any_intersection(ret_polygon_coords, {p2, target_p})) {
                    continue;
                }

                double t_cost = calc_triangle_score(p1, p2, target_p);
                memos.emplace_back(t_cost, next_v, polygon_i);
            }

            if(!memos.empty()) {
                sort(memos.begin(), memos.end());
                tie(ignore, add_v, add_ind) = memos[0];
                break;
            }
        }

        if(add_v != -1) {
            ret_polygon_vs.insert(ret_polygon_vs.begin() + add_ind + 1, add_v);
            ret_polygon_coords.insert(ret_polygon_coords.begin() + add_ind + 1, env.center_points[add_v]);
            ok_vs.erase(add_v);
        }
    }

    return {ret_polygon_vs, ret_polygon_coords};
}

tuple<vector<vector<int>>, vector<vector<pii>>, vector<int>>
calc_greedy_answer(Env &env, const vector<pii> &target_points) {
    auto graph = construct_dist_matrix(target_points);

    // G = [(i, g)] の形でソート（大きい順）
    vector<pii> groups;
    for(int i = 0; i < env.G.size(); ++i) {
        groups.emplace_back(i, env.G[i]);
    }
    sort(groups.begin(), groups.end(), [](const auto &a, const auto &b) {
        return a.second > b.second; // 降順
    });

    vector<vector<pii>> ans_edges(groups.size());
    vector<vector<int>> ans_v(groups.size());
    vector<int> ans_costs(groups.size());

    unordered_set<int> now_used;
    set<int> not_used;
    for(int i = 0; i < env.N; ++i)
        not_used.insert(i);

    for(const auto &[group_n, group_size] : groups) {
        // まだ使っていないノードから1つ取得
        int v = *not_used.begin();
        not_used.erase(v);

        vector<pii> prim_edges;
        vector<int> prim_v;
        double prim_cost;
        tie(prim_edges, prim_v, prim_cost) =
            prim_k(graph, v, group_size, now_used, env.N);

        ans_edges[group_n] = prim_edges;
        ans_v[group_n] = prim_v;
        ans_costs[group_n] = prim_cost;

        for(int u : prim_v) {
            now_used.insert(u);
            not_used.erase(u);
        }
    }

    return {ans_v, ans_edges, ans_costs};
}

// =================================
// グループ交換SA
// =================================
class MyGraph {
  public:
    vector<int> vs;
    vector<pii> edges;
    int group_ind;

    unordered_map<int, int> visited_cnt;
    unordered_map<int, vector<int>> graph;

    MyGraph(const vector<int> &vs_, const vector<pii> &edges_, int group_ind_)
        : vs(vs_), edges(edges_), group_ind(group_ind_) {
        for(int v : vs)
            visited_cnt[v] = 0;
        construct_graph();
    }

    void update_edges(const vector<pii> &remove_edges,
                      const vector<pii> &add_edges) {
        set<pii> current(edges.begin(), edges.end());
        for(const auto &e : remove_edges)
            current.erase(e);
        for(const auto &e : add_edges)
            current.insert(e);
        edges.assign(current.begin(), current.end());
        construct_graph();
    }

    int choice_min_visited() {
        int min_val = INT_MAX;
        for(const auto &[v, cnt] : visited_cnt) {
            min_val = min(min_val, cnt);
        }

        vector<int> candidates;
        for(const auto &[v, cnt] : visited_cnt) {
            if(cnt == min_val)
                candidates.push_back(v);
        }

        uniform_int_distribution<> dist(0, candidates.size() - 1);
        return candidates[dist(gen)];
    }

    pair<vector<int>, vector<pii>> walk_bfs(int init_v, int q_num) {
        unordered_set<int> visited_v;
        unordered_set<pii, pair_hash> visited_edge;
        vector<pii> q = {{-1, init_v}};

        random_device rd;
        mt19937 gen(rd());

        while(!q.empty() && visited_v.size() < q_num) {
            shuffle(q.begin(), q.end(), gen);
            auto [pv, v] = q.back();
            q.pop_back();

            if(visited_v.count(v))
                continue;

            if(pv != -1) {
                visited_edge.insert(sort_pair({pv, v}));
            }

            visited_v.insert(v);
            for(int nv : graph[v]) {
                if(!visited_v.count(nv)) {
                    q.emplace_back(v, nv);
                }
            }
        }

        for(int v : visited_v)
            visited_cnt[v] += 1;

        // set → vector に変換して返す
        vector<int> visited_v_list(visited_v.begin(), visited_v.end());
        vector<pii> visited_edge_list(visited_edge.begin(), visited_edge.end());

        return {visited_v_list, visited_edge_list};
    }

  private:
    void construct_graph() {
        graph.clear();
        for(int v : vs)
            graph[v] = {};
        for(const auto &[u, v] : edges) {
            graph[u].push_back(v);
            graph[v].push_back(u);
        }
    }
};

unordered_map<int, int> define_query_cnt(const Env &env, optional<int> limit_query_cnt) {
    unordered_map<int, int> group_query_cnt;
    for(int g = 0; g < env.M; ++g) {
        group_query_cnt[g] = 0;
    }
    vector<int> group_query_v_cnt(env.M, 0);
    vector<int> largest_groups;

    // 初期割当て
    for(int g = 0; g < env.M; ++g) {
        if(env.G[g] >= 3) {
            group_query_cnt[g] += 1;
            group_query_v_cnt[g] += env.L;
            if(env.L < env.G[g]) {
                largest_groups.push_back(g);
            }
        }
    }

    int sum_group_cnt = 0;
    for(int g = 0; g < env.M; ++g) {
        sum_group_cnt += group_query_cnt[g];
    }
    int remaining = env.Q - sum_group_cnt;

    for(int i = 0; i < remaining; ++i) {
        double min_cost = numeric_limits<double>::max();
        int target_group = -1;

        for(int g : largest_groups) {
            double cost = static_cast<double>(group_query_v_cnt[g]) / env.G[g];
            if(cost < min_cost) {
                min_cost = cost;
                target_group = g;
            }
        }

        if(target_group != -1) {
            group_query_v_cnt[target_group] += env.L;
            group_query_cnt[target_group] += 1;
            if(limit_query_cnt.has_value() && min_cost >= limit_query_cnt.value()) {
                break;
            }
        }
    }

    return group_query_cnt;
}

pair<vector<vector<int>>, vector<vector<pii>>> update_mst(
    Env &env,
    vector<vector<int>> &ans_v,
    vector<vector<pii>> &ans_edges,
    const unordered_map<int, int> &group_query_cnt) {
    for(int g = 0; g < env.M; ++g) {
        vector<int> &now_v = ans_v[g];
        vector<pii> &now_edges = ans_edges[g];

        if(now_v.size() < 3) {
            auto it = group_query_cnt.find(g);
            if(it != group_query_cnt.end() && it->second > 0) {
                cout << g << " ERROR" << endl;
            }
        } else if(now_v.size() <= env.L) {
            ans_edges[g] = env.query(now_v); // query should return vector<pii>
        } else {
            MyGraph my_graph(now_v, now_edges, g);

            auto it = group_query_cnt.find(g);
            int query_times = (it != group_query_cnt.end()) ? it->second : 0;

            for(int i = 0; i < query_times; ++i) {
                int rand_v = my_graph.choice_min_visited();
                auto [visited_v, visited_edge] = my_graph.walk_bfs(rand_v, env.L);

                auto new_edges = env.query(visited_v);
                my_graph.update_edges(visited_edge, new_edges);
            }

            ans_edges[g] = my_graph.edges;
        }
    }

    return {ans_v, ans_edges};
}

// --- 近傍操作 ---
tuple<
    vector<vector<int>>, // next_vs
    vector<int>,         // group indices [g1, g2]
    vector<int>,         // costs [g1_cost, g2_cost]
    vector<vector<pii>>, // edges [g1_edges, g2_edges]
    vector<pii>          // centroids [g1_centroid, g2_centroid]
    >
neighbor_function_define_group(
    const vector<vector<int>> &vs,
    const vector<int> &costs,
    const vector<pii> &centroids,
    const vector<pii> &coords,
    const vector<vector<int>> &graph) {
    int G = vs.size();

    vector<vector<int>> next_vs = vs;

    // 重み付きコスト
    vector<double> weighted_costs;
    for(int i = 0; i < G; ++i) {
        weighted_costs.push_back(static_cast<double>(costs[i]) / max(1, (int)vs[i].size()));
    }

    // g1 を選ぶ
    discrete_distribution<> dist1(weighted_costs.begin(), weighted_costs.end());
    int g1 = dist1(gen);

    // g2 を距離ベースで選ぶ
    vector<double> dist_weights;
    vector<int> other_inds;
    for(int g = 0; g < G; ++g) {
        if(g == g1)
            continue;
        double dx = centroids[g].first - centroids[g1].first;
        double dy = centroids[g].second - centroids[g1].second;
        double d = sqrt(dx * dx + dy * dy);
        if(d > 1e-6) {
            dist_weights.push_back(1.0 / d);
            other_inds.push_back(g);
        }
    }

    discrete_distribution<> dist2(dist_weights.begin(), dist_weights.end());
    int g2 = other_inds[dist2(gen)];

    // 交換個数を決定
    int g1_num = next_vs[g1].size();
    int g2_num = next_vs[g2].size();
    int g_num = uniform_int_distribution<>(1, min({MAX_G_NUM, g1_num, g2_num}))(gen);

    // ランダムにノードを選ぶ
    vector<int> temp1, temp2;
    sample(next_vs[g1].begin(), next_vs[g1].end(), back_inserter(temp1), g_num, gen);
    sample(next_vs[g2].begin(), next_vs[g2].end(), back_inserter(temp2), g_num, gen);

    // セット化して削除
    unordered_set<int> set1(temp1.begin(), temp1.end());
    unordered_set<int> set2(temp2.begin(), temp2.end());

    vector<int> new_g1, new_g2;
    for(int v : next_vs[g1])
        if(!set1.count(v))
            new_g1.push_back(v);
    for(int v : next_vs[g2])
        if(!set2.count(v))
            new_g2.push_back(v);

    new_g1.insert(new_g1.end(), temp2.begin(), temp2.end());
    new_g2.insert(new_g2.end(), temp1.begin(), temp1.end());

    next_vs[g1] = new_g1;
    next_vs[g2] = new_g2;

    // MST とコスト計算
    auto [g1_edges, g1_cost] = prim_vs(graph, next_vs[g1]);
    auto [g2_edges, g2_cost] = prim_vs(graph, next_vs[g2]);

    // 重心再計算
    vector<pii> g1_coords, g2_coords;
    for(int v : next_vs[g1])
        g1_coords.push_back(coords[v]);
    for(int v : next_vs[g2])
        g2_coords.push_back(coords[v]);

    pii g1_centroid = calc_centroid(g1_coords);
    pii g2_centroid = calc_centroid(g2_coords);

    return {
        next_vs,
        {g1, g2},
        {g1_cost, g2_cost},
        {g1_edges, g2_edges},
        {g1_centroid, g2_centroid}};
}

pair<vector<vector<int>>, vector<vector<pii>>>
simulated_annealing_define_group(
    const vector<vector<int>> &x0,
    double t0,
    double t1,
    const vector<pii> &coords,
    vector<vector<pii>> ans_edges,
    vector<int> ans_costs,
    double max_time,
    bool display) {
    vector<vector<int>> x = x0;
    vector<vector<int>> graph = construct_dist_matrix(coords);
    vector<int> costs = ans_costs;
    vector<vector<pii>> edges = ans_edges;

    // 初期重心計算
    vector<pii> centroids;
    for(const auto &vs_list : x) {
        vector<pii> target_coords;
        for(int v : vs_list) {
            target_coords.push_back(coords[v]);
        }
        centroids.push_back(calc_centroid(target_coords));
    }

    int current_cost = accumulate(costs.begin(), costs.end(), 0);
    auto best_x = x;
    auto best_edges = edges;
    int best_cost = current_cost;

    auto start_time = chrono::steady_clock::now();
    int iteration = 0;

    uniform_real_distribution<> rand01(0.0, 1.0);

    while(true) {
        auto now = chrono::steady_clock::now();
        double elapsed = chrono::duration<double>(now - start_time).count();
        if(elapsed >= max_time)
            break;

        double temp = exponential_schedule(t0, t1, elapsed, max_time);

        // 近傍解生成
        auto [new_x, target_groups, target_costs, target_edges, target_centroids] =
            neighbor_function_define_group(x, costs, centroids, coords, graph);

        int new_cost = 0;
        for(int g = 0; g < new_x.size(); ++g) {
            if(find(target_groups.begin(), target_groups.end(), g) == target_groups.end()) {
                new_cost += costs[g];
            }
        }
        new_cost += target_costs[0] + target_costs[1];

        int delta_cost = new_cost - current_cost;

        if(delta_cost < 0 || rand01(gen) < exp(-delta_cost / temp)) {
            x = new_x;
            current_cost = new_cost;
            int g1 = target_groups[0], g2 = target_groups[1];
            costs[g1] = target_costs[0];
            costs[g2] = target_costs[1];
            edges[g1] = target_edges[0];
            edges[g2] = target_edges[1];
            centroids[g1] = target_centroids[0];
            centroids[g2] = target_centroids[1];
        }

        if(current_cost < best_cost) {
            best_x = x;
            best_edges = edges; // my_deepcopy 相当（値渡しコピー）
            best_cost = current_cost;
        }

        iteration++;
        if(display && iteration % GROUP_SA_DISPLAY_CNT == 0) {
            debug_print("Iteration: " + to_string(iteration) +
                        ", Current cost: " + to_string(current_cost) +
                        ", Best cost: " + to_string(best_cost) +
                        ", Temp: " + to_string(temp));
        }
    }

    return {best_x, best_edges};
}

// solve 本体
int solve_for_group(Env &env, vector<pii> &target_points) {
    auto [ans_v, ans_edges, ans_costs] = calc_greedy_answer(env, target_points);

    if(env.M > 1) {
        tie(ans_v, ans_edges) = simulated_annealing_define_group(
            ans_v, GROUP_T0, GROUP_T1,
            target_points,
            ans_edges,
            ans_costs,
            GROUP_MAX_TIME,
            true);
    }
    auto group_query_cnt = define_query_cnt(env, nullopt);
    tie(ans_v, ans_edges) = update_mst(env, ans_v, ans_edges, group_query_cnt);

    return env.answer(ans_v, ans_edges); // 結果を出力
}

// =================================
// 回答を更新できる余地があればする
// =================================
bool is_reachable(const unordered_map<int, vector<int>> &graph, const unordered_set<int> &vs) {
    if(vs.empty())
        return true;

    unordered_set<int> visited;
    deque<int> q;
    q.push_back(*vs.begin());

    while(!q.empty()) {
        int v = q.back();
        q.pop_back();
        visited.insert(v);
        for(int nv : graph.at(v)) {
            if(visited.count(nv) == 0 && vs.count(nv)) {
                q.push_back(nv);
            }
        }
    }

    return visited.size() == vs.size();
}

vector<pii> update_edges(
    const unordered_set<int> &vs,
    const vector<pii> &now_edges,
    const vector<pii> &update_edges) {
    vector<pii> new_edges;

    for(const auto &[v1, v2] : now_edges) {
        if(vs.count(v1) == 0 || vs.count(v2) == 0) {
            new_edges.emplace_back(v1, v2);
        }
    }

    new_edges.insert(new_edges.end(), update_edges.begin(), update_edges.end());
    return new_edges;
}

pair<vector<vector<int>>, vector<vector<pii>>> update_ans(
    Env &env,
    vector<vector<int>> &ans_v,
    vector<vector<pii>> &ans_edges) {
    vector<vector<pii>> query_edges = env.query_history;
    vector<unordered_set<int>> query_vs;

    for(const auto &edges : query_edges) {
        unordered_set<int> group_query_vs;
        for(const auto &[v1, v2] : edges) {
            group_query_vs.insert(v1);
            group_query_vs.insert(v2);
        }
        query_vs.push_back(group_query_vs);
    }

    int update_cnt = 0;
    for(int g = 0; g < env.M; ++g) {
        const vector<int> &target_vs_list = ans_v[g];
        unordered_set<int> target_vs(target_vs_list.begin(), target_vs_list.end());
        vector<pii> &target_edges = ans_edges[g];

        for(size_t qi = 0; qi < query_vs.size(); ++qi) {
            const auto &group_query_vs = query_vs[qi];
            const auto &group_query_edges = query_edges[qi];

            // クエリ頂点が全てこのグループに含まれていなければスキップ
            bool all_in_group = all_of(group_query_vs.begin(), group_query_vs.end(),
                                       [&](int v) { return target_vs.count(v); });
            if(!all_in_group)
                continue;

            auto graph = construct_graph_from_edges(target_edges, target_vs_list);
            if(!is_reachable(graph, group_query_vs))
                continue;

            auto new_edges = update_edges(group_query_vs, target_edges, group_query_edges);
            ans_edges[g] = new_edges;
            update_cnt++;
        }
    }

    if(update_cnt > 0) {
        debug_print("Update edges: " + to_string(update_cnt));
    }
    return {ans_v, ans_edges};
}
// =================================
// MST制約を満たす点推定
// =================================
int cost_formula(pii p1, pii p2,
                 pii q1, pii q2) {
    int d1 = calc_dist(p1, p2);
    int d2 = calc_dist(q1, q2);
    return d1 > d2 ? d1 - d2 : 0;
}

tuple<vector<int>, int>
calc_init_cost(const vector<pair<pii, pii>> &constrains,
               const vector<pii> &x) {
    vector<int> cost_list(constrains.size(), 0);
    int total_cost = 0;
    for(int i = 0; i < 800; ++i) {
        x_estimate_cost[i] = 0;
    }

    for(int ci = 0; ci < constrains.size(); ++ci) {
        auto [p, q] = constrains[ci];
        auto [p1, p2] = p;
        auto [q1, q2] = q;
        int now_cost = cost_formula(x[p1], x[p2], x[q1], x[q2]);
        cost_list[ci] = now_cost;
        total_cost += now_cost;
        x_estimate_cost[p1] += now_cost;
        x_estimate_cost[p2] += now_cost;
        x_estimate_cost[q1] += now_cost;
        x_estimate_cost[q2] += now_cost;
    }

    return {cost_list, total_cost};
}

tuple<int, int, unordered_map<int, int>>
calc_cost(const vector<pair<pii, pii>> &constrains,
          const vector<pii> &x_now,
          const unordered_map<int, pii> &x_update,
          const vector<int> &cost_list, int cost_now,
          const vector<vector<int>> &vid_to_constid) {
    int new_cost = cost_now;

    // !!! 高速化のため、xの複数更新に非対応としているため注意
    int update_cost_cnt = 0;
    unordered_map<int, int> x_cost_update;
    x_cost_update.reserve(100);
    for(const auto &[v, now_coord] : x_update) {
        for(const auto &consid : vid_to_constid[v]) {
            auto [p, q] = constrains[consid];

            auto get_coord = [&](int vid) -> pii {
                return (vid == v) ? now_coord : x_now[vid];
            };

            const auto &coord1 = get_coord(p.first);
            const auto &coord2 = get_coord(p.second);
            const auto &coord3 = get_coord(q.first);
            const auto &coord4 = get_coord(q.second);

            int pre_cost = cost_list[consid];
            int now_cost = cost_formula(coord1, coord2, coord3, coord4);
            int cost_difff = now_cost - pre_cost;

            x_cost_update[p.first] += cost_difff;
            x_cost_update[p.second] += cost_difff;
            x_cost_update[q.first] += cost_difff;
            x_cost_update[q.second] += cost_difff;

            new_cost += cost_difff;
            cost_update_arr[update_cost_cnt] = now_cost;
            cost_update_id[update_cost_cnt] = consid;
            update_cost_cnt++;
        }
    }

    if(update_cost_cnt >= MAX_COST_UPDATE) {
        cerr << "cost_update_arr overflow" << endl;
        exit(1);
    }
    return {new_cost, update_cost_cnt, x_cost_update};
}

pair<unordered_map<int, pii>, double>
neighbor(const vector<pii> &x,             // 現在の座標
         const vector<int> &target_v,      // 対象インデックス
         const vector<vector<int>> &rects, // 各矩形の [x1, x2, y1, y2]
         double now_mul_x, int k = 1) {
    // ランダムに k 個サンプリング（重複あり）
    vector<int> vs;
    uniform_int_distribution<> dist(0, target_v.size() - 1);
    for(int i = 0; i < k; ++i) {
        vs.push_back(target_v[dist(gen)]);
    }

    unordered_map<int, pii> x_update;
    double mul_x = 0.01;

    for(int v : vs) {
        uniform_real_distribution<> prob(0.0, 1.0);
        double rnd = prob(gen);

        int x1 = rects[v][0], x2 = rects[v][1];
        int y1 = rects[v][2], y2 = rects[v][3];
        int nx = x[v].first, ny = x[v].second;

        pii rand_coord;

        if(rnd < 0.99) {
            int dx = x2 - x1;
            int dy = y2 - y1;
            int max_dx = max(static_cast<int>(dx * now_mul_x), 1);
            int max_dy = max(static_cast<int>(dy * now_mul_x), 1);

            int lim_l = max(x1, nx - max_dx);
            int lim_r = min(x2, nx + max_dx);
            int lim_u = max(y1, ny - max_dy);
            int lim_d = min(y2, ny + max_dy);

            uniform_int_distribution<> dist_x(lim_l, lim_r);
            uniform_int_distribution<> dist_y(lim_u, lim_d);
            rand_coord = {dist_x(gen), dist_y(gen)};
        } else {
            uniform_int_distribution<> dist_x(x1, x2);
            uniform_int_distribution<> dist_y(y1, y2);
            rand_coord = {dist_x(gen), dist_y(gen)};
        }

        x_update[v] = rand_coord;

        // mul_x計算
        if(x1 != x2) {
            mul_x = static_cast<double>(abs(nx - rand_coord.first)) / (x2 - x1);
        } else {
            mul_x = 0.01;
        }
    }

    return {x_update, mul_x};
}

// simulated_annealing 本体
pair<vector<pii>, int> simulated_annealing_estimate_coords(
    const vector<pii> &x0, double t0, double t1, double max_time,
    double init_mul_x,
    const vector<pair<pii, pii>> &constrains,
    const vector<vector<int>> &rectangles, const vector<int> &target_vs,
    const vector<vector<int>> &vid_to_constid, bool display = false) {
    vector<pii> x = x0;
    vector<pii> x_best = x;
    unordered_map<int, pii> x_update;

    vector<vector<pii>> x_best_memo;
    vector<float> time_div = divide_interval(max_time * MEMO_START_MUL, max_time, DIVIDE_TIME);
    deque<float> time_div_q(time_div.begin(), time_div.end());

    vector<int> cost_list;
    int cost_current;
    tie(cost_list, cost_current) = calc_init_cost(constrains, x);
    int cost_best = cost_current;
    int cost_update_cnt = 0;

    int simulate_cnt = 0;
    int good_cnt = 0;
    auto st = chrono::steady_clock::now();

    double sum_mul_x = 0.0;
    double now_mul_x = init_mul_x;

    unordered_map<int, int> x_update_cost;

    while(true) {
        auto now = chrono::steady_clock::now();
        double elapsed = chrono::duration<double>(now - st).count();
        if(elapsed >= max_time)
            break;
        // if(cost_best <= 1e-9) {
        //     debug_print("OPTIMAL SOLUTION FOUND");
        //     break;
        // }
        double temp = exponential_schedule(t0, t1, elapsed, max_time);

        tie(x_update, now_mul_x) = neighbor(x, target_vs, rectangles, now_mul_x);

        int cost_new;
        tie(cost_new, cost_update_cnt, x_update_cost) = calc_cost(
            constrains, x, x_update, cost_list, cost_current, vid_to_constid);

        int delta_cost = cost_new - cost_current;

        if(delta_cost < 0) {
            good_cnt++;
            sum_mul_x += now_mul_x;
        }

        if(delta_cost <= 0 || rand01(gen) < exp(-delta_cost / temp)) {
            cost_current = cost_new;
            for(const auto &[v, coord] : x_update) {
                x[v] = coord;
            }
            for(int i = 0; i < cost_update_cnt; ++i) {
                int ci = cost_update_id[i];
                int val = cost_update_arr[i];
                cost_list[ci] = val;
                for(const auto &[key, value] : x_update_cost) {
                    x_estimate_cost[key] += value;
                }
            }
        }

        if(cost_current < cost_best) {
            x_best = x;
            cost_best = cost_current;
        }

        if(time_div_q.size() > 0 && elapsed >= time_div_q.front()) {
            x_best_memo.push_back(x_best);
            time_div_q.pop_front();
        }

        simulate_cnt++;
        if(display && simulate_cnt % DISPLAY_CNT == 0) {
            debug_print("i: ", simulate_cnt, ", cost_now: ", cost_current,
                        ", best_cost: ", cost_best, ", temp: ", temp,
                        ", good_cnt: ", good_cnt, "/", simulate_cnt);
        }

        if(good_cnt > 0 && good_cnt % MUL_X_CNT == 0) {
            now_mul_x = max(min(0.01, sum_mul_x / good_cnt), 0.5);
            sum_mul_x = 0.0;
        }
    }

    if(x_best_memo.size() > 0) {
        const int memo_size = x_best_memo.size();
        vector<pii> x_best_mean;

        // 加重平均
        int std_total = 0;
        for(int i = 0; i < memo_size; ++i) {
            std_total += i + 1;
        }
        for(int i = 0; i < x_best_memo[0].size(); ++i) {
            double total_x = 0.0, total_y = 0.0;
            for(int j = 0; j < memo_size; ++j) {
                total_x += x_best_memo[j][i].first * (j + 1) / std_total;
                total_y += x_best_memo[j][i].second * (j + 1) / std_total;
            }
            x_best_mean.emplace_back(total_x, total_y);
        }

        // 単純平均
        // for(int i = 0; i < x_best_memo[0].size(); ++i) {
        //     double total_x = 0.0, total_y = 0.0;
        //     for(const auto &memo : x_best_memo) {
        //         total_x += memo[i].first;
        //         total_y += memo[i].second;
        //     }
        //     total_x /= memo_size;
        //     total_y /= memo_size;
        //     debug_print("x_best_mean: ", total_x, ", ", total_y);
        //     x_best_mean.emplace_back(total_x, total_y);
        // }
        return {x_best_mean, cost_best};
    }
    return {x_best, cost_best};
}

// =================================
// MST制約を満たす点推定
// =================================
vector<vector<int>> generate_target_queries(
    Env &env) {
    unordered_map<int, pii> coords;
    for(int i = 0; i < env.N; ++i) {
        coords[i] = env.center_points[i];
    }
    auto all_graph = construct_graph(coords);

    vector<pii> area_list;
    for(int i = 0; i < env.rectangles.size(); ++i) {
        int l = env.rectangles[i][0];
        int r = env.rectangles[i][1];
        int t = env.rectangles[i][2];
        int b = env.rectangles[i][3];
        int dx = r - l;
        int dy = b - t;
        area_list.emplace_back(i, dx * dy);
    }

    sort(area_list.begin(), area_list.end(),
         [](const auto &a, const auto &b) {
             return a.second > b.second;
         });

    vector<int> area_points;
    for(const auto &[i, _] : area_list) {
        area_points.push_back(i);
    }

    vector<vector<int>> target_query;
    for(int q = 0; q < env.Q && q < area_points.size(); ++q) {
        int target_v = area_points[q];
        auto [polygon_vs, polygon_coords] = construct_triangle(env, target_v, all_graph);
        tie(polygon_vs, ignore) = construct_droneyard(env, polygon_vs, polygon_coords);
        target_query.push_back(polygon_vs);
    }

    return target_query;
}

vector<vector<int>> generate_query_target_simple(Env &env) {

    // 面積の大きい点を優先的に選ぶ
    unordered_map<int, pii> coords;
    for(int i = 0; i < env.N; ++i) {
        coords[i] = env.center_points[i];
    }
    auto all_graph = construct_graph(coords);

    vector<pii> max_d_list;
    for(int i = 0; i < env.rectangles.size(); ++i) {
        int l = env.rectangles[i][0];
        int r = env.rectangles[i][1];
        int t = env.rectangles[i][2];
        int b = env.rectangles[i][3];
        int dx = r - l;
        int dy = b - t;
        max_d_list.emplace_back(i, dx * dy);
    }

    sort(max_d_list.begin(), max_d_list.end(),
         [](auto &a, auto &b) { return a.second > b.second; });
    vector<int> max_d_points(env.N);
    for(int i = 0; i < env.N; ++i)
        max_d_points[i] = max_d_list[i].first;

    // 近傍の点を選ぶ
    vector<vector<int>> query_target;
    for(int i = 0; i < env.Q && i < max_d_list.size(); ++i) {
        int big_v = max_d_list[i].first;
        vector<pii> other_points = all_graph[big_v];
        other_points.resize(min(env.L - 1, (int)other_points.size()));

        vector<int> other_points_ind;
        for(auto &[d, p] : other_points)
            other_points_ind.emplace_back(p);

        sort(other_points_ind.begin(), other_points_ind.end(),
             [&](int a, int b) { return max_d_points[a] > max_d_points[b]; });

        if(other_points_ind.size() > env.L - 1) {
            other_points_ind.resize(env.L - 1);
        }

        vector<int> now_query_target = {big_v};
        now_query_target.insert(now_query_target.end(), other_points_ind.begin(),
                                other_points_ind.end());
        query_target.push_back(now_query_target);
    }

    return query_target;
}

int solve_coord_estimate(Env &env) {

    const auto query_target = generate_query_target_simple(env);
    // const auto query_target = generate_target_queries(env);

    // クエリを実行して制約を得る
    unordered_set<int> random_vs;
    set<pair<pii, pii>> constrains;
    for(const auto &target : query_target) {
        auto edges = env.query(target);
        auto graph = construct_graph_from_edges(edges, target);

        for(auto &e : edges) {
            auto [group_vs1, group_vs2] = cut_graph(graph, e);
            for(int v1 : group_vs1) {
                for(int v2 : group_vs2) {
                    auto [sv1, sv2] = sort_pair({v1, v2});
                    if(e == make_pair(sv1, sv2))
                        continue;
                    constrains.insert({e, {sv1, sv2}});
                }
            }
        }
        for(int v : target)
            random_vs.insert(v);
    }

    vector<pair<pii, pii>> constrains_list(
        constrains.begin(), constrains.end());
    vector<int> random_vs_list(random_vs.begin(), random_vs.end());

    vector<unordered_set<int>> _vid_to_constid(env.N);
    for(int i = 0; i < env.N; ++i) {
        _vid_to_constid[i] = unordered_set<int>();
    }
    for(int i = 0; i < constrains_list.size(); ++i) {
        auto [e1, e2] = constrains_list[i];
        auto [p1, p2] = e1;
        auto [q1, q2] = e2;
        _vid_to_constid[p1].insert(i);
        _vid_to_constid[p2].insert(i);
        _vid_to_constid[q1].insert(i);
        _vid_to_constid[q2].insert(i);
    }
    vector<vector<int>> vid_to_constid(env.N);
    for(int v = 0; v < env.N; ++v) {
        auto const_ids = _vid_to_constid[v];
        vid_to_constid[v] = vector<int>(const_ids.begin(), const_ids.end());
    }

    // ------------------ SA ---------------------
    vector<vector<pii>> x_bests;

    for(int trial = 0; trial < TRIAL_NUM; ++trial) {
        auto [x_best, cost_best] = simulated_annealing_estimate_coords(
            env.center_points, T0, T1, MAX_TIME, INIT_MUL_X, constrains_list,
            env.rectangles, random_vs_list, vid_to_constid, true);
        x_bests.emplace_back(x_best);
    }

    vector<pii> x_best_mean;
    for(int i = 0; i < env.N; ++i) {
        int total_x = 0, total_y = 0;
        for(const auto &sol : x_bests) {
            total_x += sol[i].first;
            total_y += sol[i].second;
        }
        total_x /= x_bests.size();
        total_y /= x_bests.size();
        x_best_mean.emplace_back(total_x, total_y);
    }

    auto [ans_v, ans_edges, ans_costs] = calc_greedy_answer(env, x_best_mean);

    if(env.M > 1 && IS_POST_SA) {
        tie(ans_v, ans_edges) = simulated_annealing_define_group(
            ans_v, POST_OPT_T0, POST_OPT_T1,
            x_best_mean,
            ans_edges,
            ans_costs,
            POST_OPT_GRUOP_TIME,
            true);
    }

    auto [new_ans_v, new_ans_edges] = update_ans(env, ans_v, ans_edges);
    return env.answer(new_ans_v, new_ans_edges);
}

int solve(Env &env) {
    if(env.L < SOLVE_L_THREAD) {
        return solve_for_group(env, env.center_points);
    } else {
        return solve_coord_estimate(env);
    }
}

int main() {
    if(IS_ONLINE_JUDGE) {
        EnvOnline env;
        solve(env);
    } else {
        vector<int> costs;

        for(int file_num = START_FILE_NUM; file_num < END_FILE_NUM; ++file_num) {
            debug_print("=====");

            auto start = chrono::steady_clock::now();

            stringstream in_path_ss, out_path_ss;
            in_path_ss << "../in/" << setfill('0') << setw(4) << file_num << ".txt";
            out_path_ss << "../out/" << setfill('0') << setw(4) << file_num << ".txt";

            string input_file_path = in_path_ss.str();
            string output_file_path = out_path_ss.str();

            EnvOffline env(input_file_path, output_file_path);
            int cost = solve(env);

            // !!! デバッグ用
            // int cost = solve_for_group(env, env.coordinates);

            auto end = chrono::steady_clock::now();
            double elapsed = chrono::duration<double>(end - start).count();

            debug_print("[", file_num, "/", END_FILE_NUM, "]: ", cost, " ", fixed,
                        setprecision(2), elapsed, "s", "(Q=", env.Q, "L=", env.L,
                        "W=", env.W, ")");

            costs.emplace_back(cost);
        }

        double avg = accumulate(costs.begin(), costs.end(), 0.0) / costs.size();
        double max_cost = *max_element(costs.begin(), costs.end());

        debug_print("avg: ", avg);
        debug_print("max: ", max_cost);

        ofstream result_file("../result/cpp_xxx.txt");
        result_file << "avg: " << avg << "\n";
        for(int pi = 0; pi < costs.size(); ++pi) {
            result_file << setw(4) << setfill('0') << pi << " " << costs[pi] << "\n";
        }
    }

    return 0;
}