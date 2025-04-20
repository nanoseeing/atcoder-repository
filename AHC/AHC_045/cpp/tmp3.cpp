#include <bits/stdc++.h>

// =================================
// テンプレート
// =================================
using namespace std;

struct Init {
    Init() {
        ios::sync_with_stdio(0);
        cin.tie(0);
    }
} init;
// =================================
// 設定
// =================================
const bool IS_ONLINE_JUDGE = false;
const double MAX_TIME = 1.75;
const int TRIAL_NUM = 1;
const double T0 = 10.0;
const double T1 = 1.0;
const double INIT_MUL_X = 0.5;

bool DEBUG = false;
int DEBUG_QUERY = -1;
int DEBUG_L = -1;
const int START_FILE_NUM = 0;
const int END_FILE_NUM = 100;

using ll = long long;
using ull = unsigned long long;
using pii = pair<int, int>;
using pll = pair<ll, ll>;

// =================================
// 初期化
// =================================
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<> rand01(0.0, 1.0);

const int MAX_COST_UPDATE = 10000;
int cost_update_arr[MAX_COST_UPDATE];
int cost_update_id[MAX_COST_UPDATE];

// const int SQRT_MEMO_SIZE = 100000000;
// int sqrt_memo[SQRT_MEMO_SIZE];

// =================================
// ユーティリティ
// =================================
template <typename... Args>
void debug_print(Args... args) {
    if(IS_ONLINE_JUDGE)
        return;
    ((cout << args << " "), ...) << endl;
}

pair<int, int> sort_pair(pair<int, int> ab) {
    return ab.first < ab.second ? ab : make_pair(ab.second, ab.first);
}

// =================================
// 幾何計算
// =================================

int calc_dist(pair<int, int> a, pair<int, int> b) {
    int dx = a.first - b.first;
    int dy = a.second - b.second;
    return sqrt(dx * dx + dy * dy);

    // int s = dx * dx + dy * dy;
    // return s < SQRT_MEMO_SIZE ? sqrt_memo[s] : sqrt(s);
}

int calc_dist_not_sqrt(pair<int, int> a, pair<int, int> b) {
    int dx = a.first - b.first;
    int dy = a.second - b.second;
    return (dx * dx + dy * dy);
}
// =================================
// グラフアルゴリズム
// =================================
pair<unordered_set<int>, unordered_set<int>>
cut_graph(const unordered_map<int, vector<int>> &graph,
          pair<int, int> cut_edge) {
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
construct_dist_matrix(const vector<pair<int, int>> &points) {
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

unordered_map<int, vector<pair<int, int>>>
construct_graph(const unordered_map<int, pair<int, int>> &points) {
    unordered_map<int, vector<pair<int, int>>> graph;
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
construct_graph_from_edges(const vector<pair<int, int>> &edges,
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
construct_sorted_edges(const unordered_map<int, pair<int, int>> &coords) {
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
pair<vector<pair<int, int>>, int>
kruskals_algorithm(vector<tuple<int, int, int>> edges, const vector<int> &vs) {
    UnionFind uf(vs);
    int cost = 0;
    vector<pair<int, int>> ans_edges;

    for(const auto &[c, a, b] : edges) {
        if(!uf.same(a, b)) {
            cost += c;
            uf.unite(a, b);
            ans_edges.emplace_back(sort_pair({a, b}));
        }
    }

    return {ans_edges, cost};
}

tuple<vector<pair<int, int>>, vector<int>, int>
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
    vector<pair<int, int>> ans_edges;
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
    vector<pair<int, int>> center_points;
    vector<string> query_history;

    virtual vector<pair<int, int>> query(const vector<int> &c_list) = 0;
    virtual int answer(const vector<vector<int>> &groups,
                       const vector<vector<pair<int, int>>> &edges) = 0;

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

    vector<pair<int, int>> query(const vector<int> &c_list) override {
        cout << "? " << c_list.size();
        for(int v : c_list)
            cout << " " << v;
        cout << endl;

        vector<pair<int, int>> res;
        for(int i = 0; i < c_list.size() - 1; ++i) {
            int a, b;
            cin >> a >> b;
            res.emplace_back(a, b);
        }
        return res;
    }

    int answer(const vector<vector<int>> &groups,
               const vector<vector<pair<int, int>>> &edges) override {
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
    vector<pair<int, int>> coordinates;

  public:
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

    vector<pair<int, int>> query(const vector<int> &c_list) override {
        stringstream ss;
        ss << "? " << c_list.size();
        for(int v : c_list)
            ss << " " << v;
        query_history.emplace_back(ss.str());

        // 必要な座標だけ抽出
        unordered_map<int, pair<int, int>> subset_coords;
        for(int v : c_list) {
            subset_coords[v] = coordinates[v];
        }

        auto sorted_edges = construct_sorted_edges(subset_coords);
        auto [ans_edges, _] = kruskals_algorithm(sorted_edges, c_list);

        return ans_edges;
    }

    int answer(const vector<vector<int>> &groups,
               const vector<vector<pair<int, int>>> &edges) override {
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

        for(const string &q : query_history) {
            fout << q << endl;
        }

        // cout << "query: " << query_history.size() << endl;
        return cost;
    }
};

// =================================
// 本体
// =================================
// int cost_formula(int d1, int d2) {
//     return d1 > d2 ? d1 - d2 : 0;
// }

int cost_formula(pair<int, int> p1, pair<int, int> p2,
                 pair<int, int> q1, pair<int, int> q2) {
    int d1 = calc_dist(p1, p2);
    int d2 = calc_dist(q1, q2);
    return d1 > d2 ? d1 - d2 : 0;
}

tuple<vector<int>, int>
calc_init_cost(const vector<pair<pair<int, int>, pair<int, int>>> &constrains,
               const vector<pair<int, int>> &x) {
    vector<int> cost_list(constrains.size(), 0);
    int total_cost = 0;

    for(int ci = 0; ci < constrains.size(); ++ci) {
        auto [p, q] = constrains[ci];
        auto [p1, p2] = p;
        auto [q1, q2] = q;
        cost_list[ci] = cost_formula(x[p1], x[p2], x[q1], x[q2]);
        total_cost += cost_list[ci];
    }

    return {cost_list, total_cost};
}

tuple<int, int>
calc_cost(const vector<pair<pair<int, int>, pair<int, int>>> &constrains,
          const vector<pair<int, int>> &x_now,
          const unordered_map<int, pair<int, int>> &x_update,
          const vector<int> &cost_list, int cost_now,
          const vector<vector<int>> &vid_to_constid) {
    int new_cost = cost_now;

    // !!! 高速化のため、xの複数更新に非対応としているため注意
    int update_cost_cnt = 0;
    for(const auto &[v, now_coord] : x_update) {
        for(const auto &consid : vid_to_constid[v]) {
            auto [p, q] = constrains[consid];

            auto get_coord = [&](int vid) -> pair<int, int> {
                return (vid == v) ? now_coord : x_now[vid];
            };

            const auto &coord1 = get_coord(p.first);
            const auto &coord2 = get_coord(p.second);
            const auto &coord3 = get_coord(q.first);
            const auto &coord4 = get_coord(q.second);

            int pre_cost = cost_list[consid];
            int now_cost = cost_formula(coord1, coord2, coord3, coord4);

            new_cost += now_cost - pre_cost;
            cost_update_arr[update_cost_cnt] = now_cost;
            cost_update_id[update_cost_cnt] = consid;
            update_cost_cnt++;
        }
    }

    if(update_cost_cnt >= MAX_COST_UPDATE) {
        cerr << "cost_update_arr overflow" << endl;
        exit(1);
    }
    return {new_cost, update_cost_cnt};
}

pair<unordered_map<int, pair<int, int>>, double>
neighbor(const vector<pair<int, int>> &x,  // 現在の座標
         const vector<int> &target_v,      // 対象インデックス
         const vector<vector<int>> &rects, // 各矩形の [x1, x2, y1, y2]
         double now_mul_x, int k = 1) {
    // ランダムに k 個サンプリング（重複あり）
    vector<int> vs;
    std::uniform_int_distribution<> dist(0, target_v.size() - 1);
    for(int i = 0; i < k; ++i) {
        vs.push_back(target_v[dist(gen)]);
    }

    unordered_map<int, pair<int, int>> x_update;
    double mul_x = 0.01;

    for(int v : vs) {
        uniform_real_distribution<> prob(0.0, 1.0);
        double rnd = prob(gen);

        int x1 = rects[v][0], x2 = rects[v][1];
        int y1 = rects[v][2], y2 = rects[v][3];
        int nx = x[v].first, ny = x[v].second;

        pair<int, int> rand_coord;

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
pair<vector<pair<int, int>>, int> simulated_annealing(
    const vector<pair<int, int>> &x0, double t0, double t1, double max_time,
    double init_mul_x,
    const vector<pair<pair<int, int>, pair<int, int>>> &constrains,
    const vector<vector<int>> &rectangles, const vector<int> &target_vs,
    const vector<vector<int>> &vid_to_constid, bool display = false) {
    vector<pair<int, int>> x = x0;
    vector<pair<int, int>> x_best = x;
    unordered_map<int, pair<int, int>> x_update;

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

    while(true) {
        auto now = chrono::steady_clock::now();
        double elapsed = chrono::duration<double>(now - st).count();
        if(elapsed >= max_time)
            break;
        if(cost_best <= 1e-9) {
            debug_print("OPTIMAL SOLUTION FOUND");
            break;
        }
        double temp = exponential_schedule(t0, t1, elapsed, max_time);

        tie(x_update, now_mul_x) = neighbor(x, target_vs, rectangles, now_mul_x);

        int cost_new;
        tie(cost_new, cost_update_cnt) = calc_cost(
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
            }
        }

        if(cost_current < cost_best) {
            x_best = x;
            cost_best = cost_current;
        }

        simulate_cnt++;
        if(display && simulate_cnt % 200000 == 0) {
            debug_print("i: ", simulate_cnt, ", cost_now: ", cost_current,
                        ", best_cost: ", cost_best, ", temp: ", temp,
                        ", good_cnt: ", good_cnt, "/", simulate_cnt);
        }

        if(good_cnt > 0 && good_cnt % 100 == 0) {
            now_mul_x = max(min(0.01, sum_mul_x / good_cnt), 0.5);
            sum_mul_x = 0.0;
        }
    }

    return {x_best, cost_best};
}

tuple<vector<vector<int>>, vector<vector<pair<int, int>>>, vector<double>>
calc_greedy_answer(Env &env, const vector<pair<int, int>> &target_points) {
    auto graph = construct_dist_matrix(target_points);

    // G = [(i, g)] の形でソート（大きい順）
    vector<pair<int, int>> groups;
    for(int i = 0; i < env.G.size(); ++i) {
        groups.emplace_back(i, env.G[i]);
    }
    sort(groups.begin(), groups.end(), [](const auto &a, const auto &b) {
        return a.second > b.second; // 降順
    });

    vector<vector<pair<int, int>>> ans_edges(groups.size());
    vector<vector<int>> ans_v(groups.size());
    vector<double> ans_costs(groups.size());

    unordered_set<int> now_used;
    set<int> not_used;
    for(int i = 0; i < env.N; ++i)
        not_used.insert(i);

    for(const auto &[group_n, group_size] : groups) {
        // まだ使っていないノードから1つ取得
        int v = *not_used.begin();
        not_used.erase(v);

        vector<pair<int, int>> prim_edges;
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

int solve(Env &env) {

    unordered_map<int, pair<int, int>> coords;
    for(int i = 0; i < env.N; ++i) {
        coords[i] = env.center_points[i];
    }
    auto all_graph = construct_graph(coords);

    vector<pair<int, int>> max_d_list;
    for(int i = 0; i < env.rectangles.size(); ++i) {
        int l = env.rectangles[i][0];
        int r = env.rectangles[i][1];
        int t = env.rectangles[i][2];
        int b = env.rectangles[i][3];
        int dx = r - l;
        int dy = b - t;
        max_d_list.emplace_back(i, dx + dy); // perimeter-like metric
    }

    sort(max_d_list.begin(), max_d_list.end(),
         [](auto &a, auto &b) { return a.second > b.second; });

    vector<int> max_d_points(env.N);
    for(int i = 0; i < env.N; ++i)
        max_d_points[i] = max_d_list[i].first;

    set<pair<pair<int, int>, pair<int, int>>> constrains;
    unordered_set<int> random_vs;

    for(int i = 0; i < env.Q && i < max_d_list.size(); ++i) {
        int big_v = max_d_list[i].first;
        vector<pair<int, int>> other_points = all_graph[big_v];
        other_points.resize(min(env.L - 1, (int)other_points.size()));

        vector<int> other_points_ind;
        for(auto &[d, p] : other_points)
            other_points_ind.emplace_back(p);

        sort(other_points_ind.begin(), other_points_ind.end(),
             [&](int a, int b) { return max_d_points[a] > max_d_points[b]; });

        if(other_points_ind.size() > env.L - 1) {
            other_points_ind.resize(env.L - 1);
        }

        vector<int> tmp_random_vs = {big_v};
        tmp_random_vs.insert(tmp_random_vs.end(), other_points_ind.begin(),
                             other_points_ind.end());

        auto edges = env.query(tmp_random_vs);
        auto graph = construct_graph_from_edges(edges, tmp_random_vs);

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
        for(int v : tmp_random_vs)
            random_vs.insert(v);
    }

    vector<pair<pair<int, int>, pair<int, int>>> constrains_list(
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

    vector<vector<pair<int, int>>> x_bests;

    for(int trial = 0; trial < TRIAL_NUM; ++trial) {
        auto [x_best, cost_best] = simulated_annealing(
            env.center_points, T0, T1, MAX_TIME, INIT_MUL_X, constrains_list,
            env.rectangles, random_vs_list, vid_to_constid, true);
        x_bests.emplace_back(x_best);
    }

    vector<pair<int, int>> x_best_mean;
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

    auto [ans_v, ans_edges, _] = calc_greedy_answer(env, x_best_mean);
    return env.answer(ans_v, ans_edges);
}

int main() {
    // for(int i = 0; i < SQRT_MEMO_SIZE; ++i) {
    //     sqrt_memo[i] = sqrt(i);
    // }

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
            int cost = solve(env); // solve() の戻り値に応じて調整

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

        ofstream result_file("../result/mcmc_cpp.txt");
        result_file << "avg: " << avg << "\n";
        for(int pi = 0; pi < costs.size(); ++pi) {
            result_file << setw(4) << setfill('0') << pi << " " << costs[pi] << "\n";
        }
    }

    return 0;
}