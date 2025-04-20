

#ifndef AHC_BASE_CPP_
#define AHC_BASE_CPP_
// #pragma GCC target("avx")
// #pragma GCC optimize("O3")
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
const bool IS_ONLINE_JUDGE = true;

const int SOLVE_L_THREAD = 5; // <= SOLVE_L_THREADなら点推定しない

// 点推定
const double MAX_TIME = 1.8;
const double M1_MAX_TIME = 1.88; // グループ数1の時
const int TRIAL_NUM = 1;
const double T0 = 100.0;
const double T1 = 1.0;
const double INIT_MUL_X = 0.5;
const int DISPLAY_CNT = 100000;
const int MUL_X_CNT = 100;
const bool IS_WEIGHTED_AVG = true;
const double RAND_MOVE = 0.5;
const int MEMO_SAVE_CNT = 10000;
const int MEMO_SAVE_CAP = 100;
const int FIN_THRESH = 300;

// グループ推定（post）
const bool IS_POST_SA = true;
const double POST_OPT_GRUOP_TIME = 1.88;
const double POST_OPT_T0 = 130.0;
const double POST_OPT_T1 = 1.0;

// グループ推定（オンリー）
const double GROUP_MAX_TIME = 1.88;
const double GROUP_T0 = 130.0;
const double GROUP_T1 = 1.0;

// グループ推定共通
const double MAX_G_RATE = 1.2;
const double MIN_G_RATE = 0.2;
const int MAX_G_NUM = 10;
const int MIN_G_NUM = 2;
const int GROUP_SA_DISPLAY_CNT = 10000;

// メモ用
const int MAX_COST_UPDATE = 10000;
const int GLOBAL_N = 800;
int cost_update_arr[MAX_COST_UPDATE];
int cost_update_id[MAX_COST_UPDATE];
pii x_estimate[GLOBAL_N];
int global_rectangles[GLOBAL_N][4];

// デバッグ用
bool DEBUG = false;
int DEBUG_QUERY = -1;
int DEBUG_L = -1;
const int START_FILE_NUM = 0;
const int END_FILE_NUM = 100;

// =================================
// 初期化
// =================================
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

int sample_discrete(const std::vector<double> &weights, Xorshift32 &rng) {
    double total = std::accumulate(weights.begin(), weights.end(), 0.0);
    double r = rng.next_double() * total;
    double cumulative = 0.0;

    for(size_t i = 0; i < weights.size(); ++i) {
        cumulative += weights[i];
        if(r < cumulative) {
            return static_cast<int>(i);
        }
    }
    // 落ちたら最後を返す（丸め誤差対策）
    return static_cast<int>(weights.size() - 1);
}

int xor_uniform(Xorshift32 &rng, int low, int high) {
    return low + rng.next(high - low + 1);
}

template <typename Iterator>
std::vector<typename std::iterator_traits<Iterator>::value_type>
random_sample(Iterator begin, Iterator end, int k, Xorshift32 &rng) {
    using T = typename std::iterator_traits<Iterator>::value_type;
    std::vector<T> pool(begin, end);
    int n = pool.size();
    for(int i = 0; i < k; ++i) {
        int j = i + rng.next(n - i); // i〜n-1 の間
        std::swap(pool[i], pool[j]);
    }
    return std::vector<T>(pool.begin(), pool.begin() + k);
}

template <typename T>
void xorshift_shuffle(std::vector<T> &vec, Xorshift32 &rng) {
    for(int i = static_cast<int>(vec.size()) - 1; i > 0; --i) {
        int j = rng.next(i + 1); // 0〜i のランダムな整数
        std::swap(vec[i], vec[j]);
    }
}

class WalkersAlias {
  public:
    void setWeight(const std::vector<double> &weights) {
        n = weights.size();
        p.resize(n);
        a.resize(n);

        // Normalize weights
        double sum = std::accumulate(weights.begin(), weights.end(), 0.0);
        for(size_t i = 0; i < n; ++i) {
            p[i] = weights[i] * n / sum;
        }

        std::vector<int> small, large;
        for(size_t i = 0; i < n; ++i) {
            if(p[i] < 1.0)
                small.push_back(i);
            else
                large.push_back(i);
        }

        while(!small.empty() && !large.empty()) {
            int s = small.back();
            small.pop_back();
            int l = large.back();

            a[s] = l;
            p[l] = p[l] - (1.0 - p[s]);

            if(p[l] < 1.0) {
                small.push_back(l);
                large.pop_back();
            }
        }

        // 残りの値を初期化（念のため）
        for(int i : large)
            p[i] = 1.0;
        for(int i : small)
            p[i] = 1.0;
    }

    int choice(std::mt19937 &gen) {
        std::uniform_real_distribution<> real_dist(0.0, n);
        double r = real_dist(gen);
        int i = static_cast<int>(r);
        if(r - i < p[i])
            return i;
        else
            return a[i];
    }

  private:
    size_t n;
    std::vector<double> p; // probability threshold
    std::vector<int> a;    // alias index
};

Xorshift32 xor_rng(2525);
auto GLOBAL_START_TIME = chrono::steady_clock::now();

random_device rd;
mt19937 gen(rd());

// =================================
// ユーティリティ
// =================================
template <typename... Args>
void debug_print(Args... args) {
    ((cerr << args << " "), ...) << endl;
}

pii sort_pair(pii ab) {
    return ab.first < ab.second ? ab : make_pair(ab.second, ab.first);
}

// =================================
// 幾何計算
// =================================

vector<float> divide_interval(float l, float r, int n, bool is_exclude_edge = false) {
    vector<float> result;
    if(is_exclude_edge) {
        float step = (r - l) / (n + 1);
        for(int i = 0; i < n; ++i) {
            result.emplace_back(l + (i + 1) * step);
        }
    } else {
        float step = (r - l) / (n - 1);
        for(int i = 0; i < n; ++i) {
            result.emplace_back(l + i * step);
        }
    }
    return result;
}
inline int calc_dist(pii a, pii b) {
    int dx = a.first - b.first;
    int dy = a.second - b.second;
    return sqrt(dx * dx + dy * dy);
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
    unordered_set<int> not_used(vs.begin(), vs.end());

    priority_queue<tuple<int, int, int>, vector<tuple<int, int, int>>, greater<>> que;
    for(int v : vs) {
        que.emplace(graph[init_v][v], init_v, v);
    }

    vector<pii> ans_edges;
    int ans_cost = 0;

    while(!que.empty() && ans_edges.size() < vs.size() - 1) {
        auto [cost_v, from_v, to_v] = que.top();
        que.pop();

        if(used.contains(to_v))
            continue;

        used.insert(to_v);
        not_used.erase(to_v);
        ans_edges.emplace_back(sort_pair({from_v, to_v}));
        ans_cost += cost_v;

        for(int nxt : not_used) {
            que.emplace(graph[to_v][nxt], to_v, nxt);
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

        return cost;
    }
};

// =================================
// 共通
// =================================
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
                candidates.emplace_back(v);
        }
        int rand_choice = xor_uniform(xor_rng, 0, candidates.size() - 1);
        return candidates[rand_choice];
    }

    pair<vector<int>, vector<pii>> walk_bfs(int init_v, int q_num) {
        unordered_set<int> visited_v;
        unordered_set<pii, pair_hash> visited_edge;
        vector<pii> q = {{-1, init_v}};

        while(!q.empty() && visited_v.size() < q_num) {
            xorshift_shuffle(q, xor_rng);
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
            graph[u].emplace_back(v);
            graph[v].emplace_back(u);
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
                largest_groups.emplace_back(g);
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
    vector<vector<int>>, // group vs [g1, g2]
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
    const vector<vector<int>> &graph,
    // const double max_exchange_rate,
    const int max_exchange_group_num) {
    int G = vs.size();

    // 重み付きコスト
    vector<double> weighted_costs;
    for(int i = 0; i < G; ++i) {
        double mean_cost = static_cast<double>(costs[i]) / (int)vs[i].size();
        weighted_costs.emplace_back(pow(mean_cost, 2));
    }

    // g1 を選ぶ
    int g1 = sample_discrete(weighted_costs, xor_rng);

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
            dist_weights.emplace_back(1.0 / pow(d, 2));
            other_inds.emplace_back(g);
        }
    }

    int g2 = other_inds[sample_discrete(dist_weights, xor_rng)];

    int g1_num = vs[g1].size();
    int g2_num = vs[g2].size();
    int common_num = min(g1_num, g2_num);

    // 交換個数を決定
    int g_num = xor_uniform(xor_rng, 1, min({1, common_num, max_exchange_group_num}));

    // ランダムにノードを選ぶ
    vector<int> temp1 = random_sample(vs[g1].begin(), vs[g1].end(), g_num, xor_rng);
    vector<int> temp2 = random_sample(vs[g2].begin(), vs[g2].end(), g_num, xor_rng);

    // セット化して削除
    unordered_set<int> set1(temp1.begin(), temp1.end());
    unordered_set<int> set2(temp2.begin(), temp2.end());

    vector<int> new_g1, new_g2;
    for(int v : vs[g1])
        if(!set1.count(v))
            new_g1.emplace_back(v);
    for(int v : vs[g2])
        if(!set2.count(v))
            new_g2.emplace_back(v);

    new_g1.insert(new_g1.end(), temp2.begin(), temp2.end());
    new_g2.insert(new_g2.end(), temp1.begin(), temp1.end());

    // MST とコスト計算
    auto [g1_edges, g1_cost] = prim_vs(graph, new_g1);
    auto [g2_edges, g2_cost] = prim_vs(graph, new_g2);

    // 重心再計算
    vector<pii> g1_coords, g2_coords;
    for(int v : new_g1)
        g1_coords.emplace_back(coords[v]);
    for(int v : new_g2)
        g2_coords.emplace_back(coords[v]);

    pii g1_centroid = calc_centroid(g1_coords);
    pii g2_centroid = calc_centroid(g2_coords);

    return {
        {new_g1, new_g2},
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

    // グローバル時間からの経過時間を引いておく
    auto st = chrono::steady_clock::now();
    const auto inline_max_time = max_time - chrono::duration<double>(st - GLOBAL_START_TIME).count();

    vector<vector<int>> x = x0;
    vector<vector<int>> graph = construct_dist_matrix(coords);
    vector<int> costs = ans_costs;
    vector<vector<pii>> edges = ans_edges;

    // 初期重心計算
    vector<pii> centroids;
    for(const auto &vs_list : x) {
        vector<pii> target_coords;
        for(int v : vs_list) {
            target_coords.emplace_back(coords[v]);
        }
        centroids.emplace_back(calc_centroid(target_coords));
    }

    int current_cost = accumulate(costs.begin(), costs.end(), 0);
    auto best_x = x;
    auto best_edges = edges;
    int best_cost = current_cost;

    int iteration = 0;
    while(true) {
        auto now = chrono::steady_clock::now();
        double elapsed = chrono::duration<double>(now - st).count();
        if(elapsed >= inline_max_time)
            break;

        double temp = linear_schedule(t0, t1, elapsed, inline_max_time);
        // double max_exchange_rate = linear_schedule(MAX_G_RATE, MIN_G_RATE, elapsed, inline_max_time);
        int max_exchange_group_num = linear_schedule(MAX_G_NUM, MIN_G_NUM, elapsed, inline_max_time);

        // 近傍解生成
        auto [new_x, target_groups, target_costs, target_edges, target_centroids] =
            neighbor_function_define_group(x, costs, centroids, coords, graph, max_exchange_group_num);

        int new_cost = 0;
        for(int g = 0; g < x.size(); ++g) {
            if(find(target_groups.begin(), target_groups.end(), g) == target_groups.end()) {
                new_cost += costs[g];
            }
        }
        new_cost += target_costs[0] + target_costs[1];

        int delta_cost = new_cost - current_cost;

        if(delta_cost < 0 || xor_rng.next_double() < exp(-delta_cost / temp)) {
            current_cost = new_cost;
            int g1 = target_groups[0], g2 = target_groups[1];
            x[g1] = new_x[0];
            x[g2] = new_x[1];
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
                        // ", max_g_rate: " + to_string(max_exchange_rate) +
                        ", max_g_num: " + to_string(max_exchange_group_num) +
                        ", Temp: " + to_string(temp));
        }
    }

    if(display) {
        debug_print("Iteration: " + to_string(iteration) +
                    ", Best cost: " + to_string(best_cost));
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
    q.emplace_back(*vs.begin());

    while(!q.empty()) {
        int v = q.back();
        q.pop_back();
        visited.insert(v);
        for(int nv : graph.at(v)) {
            if(visited.count(nv) == 0 && vs.count(nv)) {
                q.emplace_back(nv);
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
        query_vs.emplace_back(group_query_vs);
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
inline int cost_formula(pii p1, pii p2,
                        pii q1, pii q2) {
    int d1 = calc_dist(p1, p2);
    int d2 = calc_dist(q1, q2);
    return d1 > d2 ? d1 - d2 : 0;
}

tuple<vector<int>, int>
calc_init_cost(const vector<pair<pii, pii>> &constrains) {
    vector<int> cost_list(constrains.size(), 0);
    int total_cost = 0;

    for(int ci = 0; ci < constrains.size(); ++ci) {
        auto [p, q] = constrains[ci];
        auto [p1, p2] = p;
        auto [q1, q2] = q;
        int now_cost = cost_formula(x_estimate[p1], x_estimate[p2], x_estimate[q1], x_estimate[q2]);
        cost_list[ci] = now_cost;
        total_cost += now_cost;
    }

    return {cost_list, total_cost};
}

tuple<int, int>
calc_cost(const vector<pair<pii, pii>> &constrains,
          const unordered_map<int, pii> &x_update,
          const vector<int> &cost_list, int cost_now,
          const vector<vector<int>> &vid_to_constid) {

    int cost_diff = 0;
    int update_cost_cnt = 0;

    // !!! xの複数更新に非対応としているため注意
    for(const auto &[v, now_coord] : x_update) {
        for(const auto &consid : vid_to_constid[v]) {
            auto [p, q] = constrains[consid];
            auto get_coord = [&](int vid) -> pii {
                return (vid == v) ? now_coord : x_estimate[vid];
            };
            int now_cost = cost_formula(get_coord(p.first), get_coord(p.second), get_coord(q.first), get_coord(q.second));
            cost_diff += now_cost - cost_list[consid];
            cost_update_arr[update_cost_cnt] = now_cost;
            cost_update_id[update_cost_cnt] = consid;
            update_cost_cnt++;
        }
    }

    if(update_cost_cnt >= MAX_COST_UPDATE) {
        debug_print("cost_update_id overflow");
    }

    int new_cost = cost_now + cost_diff;
    return {new_cost, update_cost_cnt};
}

tuple<unordered_map<int, pii>, double>
neighbor(const vector<int> &target_v, // 対象インデックス
         double now_mul_x, auto &area_rand, int k = 1) {

    // ランダムに k 個
    // !!! 実質k=1で実装
    vector<int> vs;
    for(int i = 0; i < k; ++i) {
        int rand_g = area_rand.choice(gen);
        // int rand_g = xor_uniform(xor_rng, 0, target_v.size() - 1);
        vs.emplace_back(target_v[rand_g]);
    }

    unordered_map<int, pii> x_update;
    double mul_x = 0.01;
    for(int v : vs) {
        int x1 = global_rectangles[v][0], x2 = global_rectangles[v][1];
        int y1 = global_rectangles[v][2], y2 = global_rectangles[v][3];
        int nx = x_estimate[v].first, ny = x_estimate[v].second;

        pii rand_coord;

        if(xor_rng.next_double() < RAND_MOVE) {
            int dx = x2 - x1;
            int dy = y2 - y1;
            int max_dx = max(static_cast<int>(dx * now_mul_x), 1);
            int max_dy = max(static_cast<int>(dy * now_mul_x), 1);

            int lim_l = max(x1, nx - max_dx);
            int lim_r = min(x2, nx + max_dx);
            int lim_u = max(y1, ny - max_dy);
            int lim_d = min(y2, ny + max_dy);
            rand_coord = {xor_uniform(xor_rng, lim_l, lim_r), xor_uniform(xor_rng, lim_u, lim_d)};
        } else {
            rand_coord = {xor_uniform(xor_rng, x1, x2), xor_uniform(xor_rng, y1, y2)};
        }

        x_update[v] = rand_coord;

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
    double t0, double t1, double max_time,
    double init_mul_x,
    const vector<pair<pii, pii>> &constrains,
    const vector<int> &target_vs,
    const vector<vector<int>> &vid_to_constid, bool display = false) {

    // グローバル時間からの経過時間を引いておく
    auto st = chrono::steady_clock::now();
    const auto inline_max_time = max_time - chrono::duration<double>(st - GLOBAL_START_TIME).count();

    vector<double> weighted_costs(target_vs.size(), 0.0);
    for(int i = 0; i < target_vs.size(); ++i) {
        int v = target_vs[i];
        int dx = global_rectangles[v][1] - global_rectangles[v][0];
        int dy = global_rectangles[v][3] - global_rectangles[v][2];
        weighted_costs[i] = dx * dy + 50;
    }
    auto area_rand_waker_alias = WalkersAlias();
    area_rand_waker_alias.setWeight(weighted_costs);

    vector<pii> x_best(x_estimate, x_estimate + GLOBAL_N);
    unordered_map<int, pii> x_update;

    queue<vector<pii>> x_best_memo;

    vector<int> cost_list;
    int cost_current;
    tie(cost_list, cost_current) = calc_init_cost(constrains);
    int cost_best = cost_current;
    int cost_update_cnt = 0;

    int simulate_cnt = 0;
    int good_cnt = 0;

    double sum_mul_x = 0.0;
    double now_mul_x = init_mul_x;

    int remain_simulate_cnt = MEMO_SAVE_CNT * MEMO_SAVE_CAP;
    bool is_best = false;

    while(remain_simulate_cnt > 0) {
        auto now = chrono::steady_clock::now();
        double elapsed = chrono::duration<double>(now - st).count();
        if(elapsed >= inline_max_time)
            break;

        double temp = exponential_schedule(t0, t1, elapsed, max_time);

        tie(x_update, now_mul_x) = neighbor(target_vs, now_mul_x, area_rand_waker_alias);

        int cost_new;
        tie(cost_new, cost_update_cnt) = calc_cost(
            constrains, x_update, cost_list, cost_current, vid_to_constid);

        int delta_cost = cost_new - cost_current;

        if(delta_cost < 0) {
            good_cnt++;
            sum_mul_x += now_mul_x;
        }

        if(delta_cost <= 0 || xor_rng.next_double() < exp(-delta_cost) / temp) {
            cost_current = cost_new;
            for(const auto &[v, coord] : x_update) {
                x_estimate[v] = coord;
            }
            for(int i = 0; i < cost_update_cnt; ++i) {
                int ci = cost_update_id[i];
                int val = cost_update_arr[i];
                cost_list[ci] = val;
            }
        }

        if(cost_current < cost_best) {
            x_best = vector(x_estimate, x_estimate + GLOBAL_N);
            cost_best = cost_current;
        }

        simulate_cnt++;

        if(simulate_cnt % MEMO_SAVE_CNT == 0) {
            if(int(x_best_memo.size()) >= MEMO_SAVE_CAP) {
                x_best_memo.pop();
            }
            x_best_memo.push(x_best);
        }

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

    if(display) {
        debug_print("i: ", simulate_cnt, ", best_cost: ", cost_best, ", good count: ", good_cnt);
    }

    vector<vector<pii>> x_best_memo_vec;
    while(!x_best_memo.empty()) {
        x_best_memo_vec.push_back(x_best_memo.front());
        x_best_memo.pop();
    }

    if(x_best_memo_vec.size() > 0) {
        const int memo_size = x_best_memo_vec.size();
        vector<pii> x_best_mean;
        if(IS_WEIGHTED_AVG) {
            int std_total = 0;
            for(int i = 0; i < memo_size; ++i) {
                std_total += i + 1;
            }
            for(int i = 0; i < x_best_memo_vec[0].size(); ++i) {
                double total_x = 0.0, total_y = 0.0;
                for(int j = 0; j < memo_size; ++j) {
                    total_x += x_best_memo_vec[j][i].first * (j + 1) / std_total;
                    total_y += x_best_memo_vec[j][i].second * (j + 1) / std_total;
                }
                x_best_mean.emplace_back(total_x, total_y);
            }
        } else {
            for(int i = 0; i < x_best_memo_vec[0].size(); ++i) {
                double total_x = 0.0, total_y = 0.0;
                for(const auto &memo : x_best_memo_vec) {
                    total_x += memo[i].first;
                    total_y += memo[i].second;
                }
                total_x /= memo_size;
                total_y /= memo_size;
                x_best_mean.emplace_back(total_x, total_y);
            }
        }
        return {x_best_mean, cost_best};
    }
    return {x_best, cost_best};
}

// =================================
// MST制約を満たす点推定
// =================================

vector<vector<int>> generate_query_target_simple(Env &env, int querry_size) {

    // 面積の大きい点を優先的に選ぶ
    unordered_map<int, pii> coords;
    for(int i = 0; i < env.N; ++i) {
        coords[i] = env.center_points[i];
    }
    auto all_graph = construct_graph(coords);

    vector<pii> max_d_list;
    for(int i = 0; i < GLOBAL_N; ++i) {
        int l = global_rectangles[i][0];
        int r = global_rectangles[i][1];
        int t = global_rectangles[i][2];
        int b = global_rectangles[i][3];
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
    for(int i = 0; i < querry_size && i < max_d_list.size(); ++i) {
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
        query_target.emplace_back(now_query_target);
    }

    return query_target;
}

vector<pii> solve_coord_estimate(Env &env) {

    // unordered_map<int, int> query_cnt = define_query_cnt(env, 1.5);
    // int total_query_cnt = 0;
    // for(auto &[g, cnt] : query_cnt) {
    //     total_query_cnt += cnt;
    // }

    const auto query_target = generate_query_target_simple(env, env.Q);

    // ------------------ クエリを実行して制約を得る ---------------------

    std::cerr << "Query target size: " << query_target.size() << std::endl;
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
    std::cerr << "Constrains size: " << constrains.size() << std::endl;

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

    for(int i = 0; i < env.N; ++i) {
        x_estimate[i] = env.center_points[i];
    }
    for(int trial = 0; trial < TRIAL_NUM; ++trial) {
        double max_time = env.M > 1 ? MAX_TIME : M1_MAX_TIME;
        auto [x_best, cost_best] = simulated_annealing_estimate_coords(
            T0, T1, max_time, INIT_MUL_X, constrains_list,
            random_vs_list, vid_to_constid, true);
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

    return x_best_mean;
    // ------------------ SA 2 ---------------------
    // if(env.M > 1 && IS_POST_SA) {
    //     tie(ans_v, ans_edges) = simulated_annealing_define_group(
    //         ans_v, POST_OPT_T0, POST_OPT_T1,
    //         x_best_mean,
    //         ans_edges,
    //         ans_costs,
    //         POST_OPT_GRUOP_TIME,
    //         true);
    // }

    // // tie(ans_v, ans_edges) = update_mst(env, ans_v, ans_edges, query_cnt);

    // auto [new_ans_v, new_ans_edges] = update_ans(env, ans_v, ans_edges);
    // return env.answer(new_ans_v, new_ans_edges);
}

// int solve(Env &env) {
//     for(int i = 0; i < GLOBAL_N; ++i) {
//         x_estimate[i] = env.center_points[i];
//     }
//     for(int i = 0; i < GLOBAL_N; ++i) {
//         for(int j = 0; j < 4; ++j) {
//             global_rectangles[i][j] = env.rectangles[i][j];
//         }
//     }

//     if(env.L <= SOLVE_L_THREAD) {
//         return solve_for_group(env, env.center_points);
//     } else {
//         return solve_coord_estimate(env);
//     }
// }

// int main() {
//     if(IS_ONLINE_JUDGE) {
//         EnvOnline env;
//         solve(env);
//     } else {
//         vector<int> costs;

//         for(int file_num = START_FILE_NUM; file_num < END_FILE_NUM; ++file_num) {
//             GLOBAL_START_TIME = chrono::steady_clock::now();
//             debug_print("=====");

//             stringstream in_path_ss, out_path_ss;
//             in_path_ss << "../in/" << setfill('0') << setw(4) << file_num << ".txt";
//             out_path_ss << "../out/" << setfill('0') << setw(4) << file_num << ".txt";

//             string input_file_path = in_path_ss.str();
//             string output_file_path = out_path_ss.str();

//             EnvOffline env(input_file_path, output_file_path);
//             int cost = solve(env);

//             auto end = chrono::steady_clock::now();
//             double elapsed = chrono::duration<double>(end - GLOBAL_START_TIME).count();

//             debug_print("[", file_num, "/", END_FILE_NUM, "]: ", cost, " ", fixed,
//                         setprecision(2), elapsed, "s", "(Q=", env.Q, "L=", env.L,
//                         "W=", env.W, ")");

//             costs.emplace_back(cost);
//         }

//         double avg = accumulate(costs.begin(), costs.end(), 0.0) / costs.size();
//         double max_cost = *max_element(costs.begin(), costs.end());

//         debug_print("avg: ", avg);
//         debug_print("max: ", max_cost);

//         ofstream result_file("../result/cpp_fast4.txt");
//         result_file << "avg: " << avg << "\n";
//         for(int pi = 0; pi < costs.size(); ++pi) {
//             result_file << setw(4) << setfill('0') << pi << " " << costs[pi] << "\n";
//         }
//     }

//     return 0;
// }

using uint8 = uint8_t;
using int64 = int64_t;
using uint64 = uint64_t;

#define UNEXPECTED()                                                     \
    {                                                                    \
        std::cerr << "Unexpected@" << __FILE__ << ":" << __LINE__ << "!" \
                  << std::endl;                                          \
        exit(1);                                                         \
    }

// Override operator<< for ostream to handle vector.
template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &v) {
    os << "[";
    for(const int i : std::views::iota(0, int(v.size()))) {
        if(i > 0)
            os << ", ";
        os << v[i];
    }
    os << "]";
    return os;
}

double g_time_factor = 1.0;

class TimeMeasurement {
  public:
    TimeMeasurement()
        : start_(std::chrono::steady_clock::now()) {
    }

    int64 CurrentMs() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_)
                   .count() *
               g_time_factor;
    }

    int64 CurrentUs() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(now - start_)
                   .count() *
               g_time_factor;
    }

  private:
    std::chrono::time_point<std::chrono::steady_clock> start_;
};
TimeMeasurement g_time;

class Random {
  public:
    Random()
        : state_(3141592653589793238ULL) {
    }

    double RandomDouble(double a, double b) {
        Update();
        const double r = state_ * kScale_;
        return a + (b - a) * r;
    }

    int RandomInt(int a, int b) {
        return static_cast<int>(RandomDouble(a, b));
    }

    int RandomIntInclusive(int a, int b) {
        return RandomInt(a, b + 1);
    }

  private:
    uint64 state_;

    // 2^(-64)
    static constexpr double kScale_ = 1.0 / 18446744073709551616.0;

    void Update() {
        // xorshift
        state_ ^= state_ >> 13;
        state_ ^= state_ << 7;
        state_ ^= state_ >> 17;
    }
};
Random g_random;

#endif // AHC_BASE_CPP_

#ifndef OPTIMIZATION_BASE_CPP_
#define OPTIMIZATION_BASE_CPP_

#ifndef ALGORITHMS_CPP_
#define ALGORITHMS_CPP_

class MergeFindSet {
  public:
    MergeFindSet(const int n)
        : rs_(n), ps_(n), ss_(n, 1) {
        for(const int i : std::views::iota(0, n)) {
            ps_[i] = i;
        }
    }

    int Find(const int x) {
        if(x != ps_[x]) {
            ps_[x] = Find(ps_[x]);
        }
        return ps_[x];
    }

    void Merge(const int x, const int y) {
        const int xr = Find(x);
        const int yr = Find(y);
        if(rs_[xr] > rs_[yr]) {
            ps_[yr] = ps_[xr];
            ss_[xr] += ss_[yr];
        } else if(rs_[xr] < rs_[yr]) {
            ps_[xr] = ps_[yr];
            ss_[yr] += ss_[xr];
        } else if(ps_[xr] != ps_[yr]) {
            ps_[yr] = ps_[xr];
            rs_[xr]++;
            ss_[xr] += ss_[yr];
        }
    }

    int Size(int x) {
        return ss_[Find(x)];
    }

  private:
    std::vector<int> rs_, ps_, ss_;
};

class SubsetSumSolver {
  public:
    SubsetSumSolver() {
    }

    void Solve(const std::vector<int> &xs, const int max_sum) {
        xs_ = xs;
        dp_table_.clear();
        dp_table_.assign(max_sum + 1, 0);
        prev_index_table_.clear();
        prev_index_table_.assign(max_sum + 1, -1);

        dp_table_[0] = 1;
        for(const int i : std::views::iota(0, int(xs_.size()))) {
            const int x = xs_[i];
            // Iterate in descending order so each element is used at most once.
            for(int sum = max_sum; sum >= x; sum--) {
                if(!dp_table_[sum] && dp_table_[sum - x]) {
                    dp_table_[sum] = 1;
                    prev_index_table_[sum] = i;
                }
            }
        }

        is_solved_ = true;
    }

    // Returns whether there is a subset of xs_ that sums to target.
    bool SubsetExists(const int target) const {
        if(!is_solved_) {
            UNEXPECTED();
        }
        if(target >= dp_table_.size())
            return false;
        return dp_table_[target];
    }

    // Returns a subset of xs_ that sums to target as indices.
    // Assumption: SubsetExists(target) is true.
    std::vector<int> GetSubset(const int target) const {
        if(!is_solved_) {
            UNEXPECTED();
        }
        std::vector<int> subset_indices;
        int sum = target;
        while(sum > 0) {
            const int index = prev_index_table_[sum];
            subset_indices.push_back(index);
            sum -= xs_[index];
        }
        if(sum != 0) {
            UNEXPECTED();
        }
        return subset_indices;
    }

  private:
    std::vector<int> xs_;
    bool is_solved_ = false;
    std::vector<int> dp_table_;
    std::vector<int> prev_index_table_;
};

struct Edge {
    int s, t;
    Edge(const int s, const int t)
        : s(s), t(t) {
    }
    Edge()
        : s(-1), t(-1) {
    }
};

struct WEdge {
    int s, t;
    double w;
    WEdge(const int s, const int t, const double w)
        : s(s), t(t), w(w) {
    }
    WEdge()
        : s(-1), t(-1), w(0.0) {
    }
};

struct Graph {
    int vertex_count;
    std::vector<Edge> edges;

    // edges[v_begins[v], v_ends[v]) is the list of edges from the vertex v.
    std::vector<int> v_begins;
    std::vector<int> v_ends;
    Graph(const int in_vertex_count, const std::vector<Edge> &in_edges)
        : vertex_count(in_vertex_count), edges(in_edges.size()),
          v_begins(vertex_count), v_ends(vertex_count) {
        std::vector<int> counts(vertex_count);
        for(const auto &edge : in_edges) {
            counts[edge.s]++;
        }
        for(const int v : std::views::iota(1, vertex_count)) {
            v_begins[v] = v_begins[v - 1] + counts[v - 1];
            v_ends[v] = v_begins[v];
        }
        for(const auto &edge : in_edges) {
            edges[v_ends[edge.s]] = edge;
            v_ends[edge.s]++;
        }
    }

    Graph()
        : vertex_count(0) {
    }

    auto OutEdges(int v) -> std::ranges::subrange<std::vector<Edge>::iterator> {
        auto begin_it = edges.begin() + v_begins[v];
        auto end_it = edges.begin() + v_ends[v];
        return std::ranges::subrange(begin_it, end_it);
    }

    auto OutEdges(int v) const
        -> std::ranges::subrange<std::vector<Edge>::const_iterator> {
        auto begin_it = edges.begin() + v_begins[v];
        auto end_it = edges.begin() + v_ends[v];
        return std::ranges::subrange(begin_it, end_it);
    }

    void Reset(int new_vertex_count, const std::vector<Edge> &new_edges) {
        vertex_count = new_vertex_count;
        edges = new_edges;
        v_begins.clear();
        v_begins.resize(vertex_count);
        v_ends.clear();
        v_ends.resize(vertex_count);

        std::vector<int> counts(vertex_count);
        for(const auto &edge : new_edges) {
            counts[edge.s]++;
        }
        for(const int v : std::views::iota(1, vertex_count)) {
            v_begins[v] = v_begins[v - 1] + counts[v - 1];
            v_ends[v] = v_begins[v];
        }
        for(const auto &edge : new_edges) {
            edges[v_ends[edge.s]] = edge;
            v_ends[edge.s]++;
        }
    }
};

class IndexSet {
  public:
    IndexSet(int n)
        : positions_(n, -1) {
    }

    void Add(int v) {
        if(Contains(v)) {
            return;
        }
        positions_[v] = values_.size();
        values_.push_back(v);
    }

    void Remove(int v) {
        int p = positions_[v];
        int b = values_.back();
        values_[p] = b;
        values_.pop_back();
        positions_[b] = p;
        positions_[v] = -1;
    }

    bool Contains(int v) const {
        return positions_[v] != -1;
    }

    int Size() const {
        return values_.size();
    }

    const std::vector<int> &Values() const {
        return values_;
    }

    void Clear() {
        for(const int v : values_) {
            positions_[v] = -1;
        }
        values_.clear();
    }

  private:
    std::vector<int> values_;
    std::vector<int> positions_;
};

class DoubleMatrix {
  public:
    DoubleMatrix(int r, int c)
        : r_(r), c_(c), values_(r * c) {
    }

    double Get(int i, int j) const {
        return values_[i * c_ + j];
    }

    void Set(int i, int j, double value) {
        values_[i * c_ + j] = value;
    }

    void Increment(int i, int j, double value) {
        values_[i * c_ + j] += value;
    }

  private:
    int r_;
    int c_;
    std::vector<double> values_;
};

#endif // ALGORITHMS_CPP_

#ifndef MCM_CPP_
#define MCM_CPP_

#ifndef DEBUG_CPP_
#define DEBUG_CPP_

#ifndef PROBLEM_BASE_CPP_
#define PROBLEM_BASE_CPP_

constexpr int kVertexCount = 800;
constexpr int kMaxQueryCount = 400;
constexpr int kMaxQuerySize = 15;

constexpr int kQueryPhaseQuickModeTime = 1200;

bool g_is_local_mode = false;

struct Double2D {
    double x = 0.0;
    double y = 0.0;
    Double2D(double x, double y)
        : x(x), y(y) {
    }
    Double2D()
        : x(0.0), y(0.0) {
    }
};

// Returns the squared distance between two points.
double Distance2(const Double2D &p0, const Double2D &p1) {
    const double dx = p0.x - p1.x;
    const double dy = p0.y - p1.y;
    return dx * dx + dy * dy;
}

double Distance(const Double2D &p0, const Double2D &p1) {
    const double dx = p0.x - p1.x;
    const double dy = p0.y - p1.y;
    return std::sqrt(dx * dx + dy * dy);
}

struct Double2DRange {
    double x_min;
    double x_max;
    double y_min;
    double y_max;
    Double2DRange(const double x_min, const double x_max, const double y_min,
                  const double y_max)
        : x_min(x_min), x_max(x_max), y_min(y_min), y_max(y_max) {
    }
    Double2DRange()
        : x_min(0.0), x_max(0.0), y_min(0.0), y_max(0.0) {
    }
};

struct ProblemInput {
    int cluster_count = 0;
    int max_query_size = 0;
    int max_error_width = 0;
    std::vector<int> cluster_sizes;
    std::array<double, kVertexCount> x_mins;
    std::array<double, kVertexCount> x_maxs;
    std::array<double, kVertexCount> y_mins;
    std::array<double, kVertexCount> y_maxs;
};

std::array<Double2D, kVertexCount> g_actual_points{};

struct RangeConstraint {
    int i;
    double x_min;
    double x_max;
    double y_min;
    double y_max;
    RangeConstraint(int i, double x_min, double x_max, double y_min, double y_max)
        : i(i), x_min(x_min), x_max(x_max), y_min(y_min), y_max(y_max) {
    }
};

// A constraint that distance(P_i0, p_i1) >= distance(P_j0, p_j1).
struct DistanceConstraint {
    int i0, i1, j0, j1;
    DistanceConstraint(const int i0, const int i1, const int j0, const int j1)
        : i0(i0), i1(i1), j0(j0), j1(j1) {
    }
    DistanceConstraint()
        : i0(-1), i1(-1), j0(-1), j1(-1) {
    }

    // For std::set.
    bool operator==(const DistanceConstraint &other) const {
        return i0 == other.i0 && i1 == other.i1 && j0 == other.j0 && j1 == other.j1;
    }

    // For std::set.
    bool operator<(const DistanceConstraint &other) const {
        if(i0 != other.i0)
            return i0 < other.i0;
        if(i1 != other.i1)
            return i1 < other.i1;
        if(j0 != other.j0)
            return j0 < other.j0;
        return j1 < other.j1;
    }
};

enum class IndexedDistanceConstraintType { kUndefined,
                                           kI0,
                                           kJ0,
                                           kI0J0 };

struct IndexedDistanceConstraint {
    int index;
    IndexedDistanceConstraintType type;
    DistanceConstraint distance_constraint;
};

class ConstraintSet {
  public:
    std::vector<RangeConstraint> range_constraints;

    // For gradient descent.
    std::vector<DistanceConstraint> distance_constraints;
    std::vector<std::vector<int>> v_to_distance_constraint_indexes;

    // For Gibbs sampling.
    std::vector<std::vector<IndexedDistanceConstraint>> v_to_i0_constraints;
    std::vector<std::vector<IndexedDistanceConstraint>> v_to_j0_constraints;
    std::vector<std::vector<IndexedDistanceConstraint>> v_to_i0j0_constraints;

    ConstraintSet()
        : v_to_distance_constraint_indexes(kVertexCount),
          v_to_i0_constraints(kVertexCount), v_to_j0_constraints(kVertexCount),
          v_to_i0j0_constraints(kVertexCount) {
    }

    void AddRangeConstraint(const RangeConstraint &range_constraint) {
        range_constraints.push_back(range_constraint);
    }

    void AddDistanceConstraint(const DistanceConstraint &distance_constraint) {
        if(distance_constraint_set_.contains(distance_constraint)) {
            return;
        }
        distance_constraint_set_.insert(distance_constraint);

        const int distance_constraint_index = distance_constraints.size();
        distance_constraints.push_back(distance_constraint);
        v_to_distance_constraint_indexes[distance_constraint.i0].push_back(
            distance_constraint_index);
        v_to_distance_constraint_indexes[distance_constraint.i1].push_back(
            distance_constraint_index);
        v_to_distance_constraint_indexes[distance_constraint.j0].push_back(
            distance_constraint_index);
        v_to_distance_constraint_indexes[distance_constraint.j1].push_back(
            distance_constraint_index);

        const auto [i0, i1, j0, j1] = distance_constraint;
        // Index i0
        if(i0 == j0) {
            IndexedDistanceConstraint constraint;
            constraint.index = i0;
            constraint.type = IndexedDistanceConstraintType::kI0J0;
            constraint.distance_constraint = DistanceConstraint(i0, i1, j0, j1);
            v_to_i0j0_constraints[i0].push_back(constraint);
        } else if(i0 == j1) {
            IndexedDistanceConstraint constraint;
            constraint.index = i0;
            constraint.type = IndexedDistanceConstraintType::kI0J0;
            constraint.distance_constraint = DistanceConstraint(i0, i1, j1, j0);
            v_to_i0j0_constraints[i0].push_back(constraint);
        } else {
            IndexedDistanceConstraint constraint;
            constraint.index = i0;
            constraint.type = IndexedDistanceConstraintType::kI0;
            constraint.distance_constraint = DistanceConstraint(i0, i1, j0, j1);
            v_to_i0_constraints[i0].push_back(constraint);
        }

        // Index i1
        if(i1 == j0) {
            IndexedDistanceConstraint constraint;
            constraint.index = i1;
            constraint.type = IndexedDistanceConstraintType::kI0J0;
            constraint.distance_constraint = DistanceConstraint(i1, i0, j0, j1);
            v_to_i0j0_constraints[i1].push_back(constraint);
        } else if(i1 == j1) {
            IndexedDistanceConstraint constraint;
            constraint.index = i1;
            constraint.type = IndexedDistanceConstraintType::kI0J0;
            constraint.distance_constraint = DistanceConstraint(i1, i0, j1, j0);
            v_to_i0j0_constraints[i1].push_back(constraint);
        } else {
            IndexedDistanceConstraint constraint;
            constraint.index = i1;
            constraint.type = IndexedDistanceConstraintType::kI0;
            constraint.distance_constraint = DistanceConstraint(i1, i0, j0, j1);
            v_to_i0_constraints[i1].push_back(constraint);
        }

        // Index j0
        if(j0 == i0 || j0 == i1) {
            // This case is already indexed to i0 or i1.
        } else {
            IndexedDistanceConstraint constraint;
            constraint.index = j0;
            constraint.type = IndexedDistanceConstraintType::kJ0;
            constraint.distance_constraint = DistanceConstraint(i0, i1, j0, j1);
            v_to_j0_constraints[j0].push_back(constraint);
        }

        // Index j1
        if(j1 == i0 || j1 == i1) {
            // This case is already indexed to i0 or i1.
        } else {
            IndexedDistanceConstraint constraint;
            constraint.index = j1;
            constraint.type = IndexedDistanceConstraintType::kJ0;
            constraint.distance_constraint = DistanceConstraint(i0, i1, j1, j0);
            v_to_j0_constraints[j1].push_back(constraint);
        }
    }

  private:
    // For deduplication.
    std::set<DistanceConstraint> distance_constraint_set_;
};

struct MCMCSample {
    std::array<Double2D, kVertexCount> points{};
};

struct MSTCluster {
    std::vector<int> vs;
    std::vector<Edge> mst_edges;
    MSTCluster(const std::vector<int> &vs, const std::vector<Edge> &mst_edges)
        : vs(vs), mst_edges(mst_edges) {
    }
};

#endif // PROBLEM_BASE_CPP_

void CheckConstraints(
    const ConstraintSet &constraint_set,
    const std::array<Double2D, kVertexCount> &points) {
    const double kMargin = 10.0;
    int range_constraint_error_count = 0;
    int distance_constraint_error_count = 0;
    for(const auto &constraint : constraint_set.range_constraints) {
        if(points[constraint.i].x < constraint.x_min - kMargin ||
           points[constraint.i].x > constraint.x_max + kMargin ||
           points[constraint.i].y < constraint.y_min - kMargin ||
           points[constraint.i].y > constraint.y_max + kMargin) {
            range_constraint_error_count++;
        }
    }
    for(const auto &constraint : constraint_set.distance_constraints) {
        const double d2i = Distance2(points[constraint.i0], points[constraint.i1]);
        const double di = std::sqrt(d2i);
        const double d2j = Distance2(points[constraint.j0], points[constraint.j1]);
        const double dj = std::sqrt(d2j);
        if(di - dj < -kMargin) {
            distance_constraint_error_count++;
        }
    }
    std::cerr << "# range constraint errors: " << range_constraint_error_count
              << std::endl;
    std::cerr << "# distance constraint errors: "
              << distance_constraint_error_count << std::endl;
}

#endif // DEBUG_CPP_

constexpr double kSamplingTimeLimit = 1700.0;

void RunGibbsSampling(
    const std::vector<int> &vs,
    const ConstraintSet &constraint_set,
    std::array<Double2D, kVertexCount> &points,
    std::vector<Double2DRange> &ranges,
    bool is_medium_mode = false) {
    for(const int v : vs) {
        // x
        double x_min =
            std::min(constraint_set.range_constraints[v].x_min, points[v].x);
        double x_max =
            std::max(constraint_set.range_constraints[v].x_max, points[v].x);
        double x = points[v].x;
        double y = points[v].y;

        for(const auto &indexed_istance_constraint :
            constraint_set.v_to_j0_constraints[v]) {
            const auto &distance_constraint =
                indexed_istance_constraint.distance_constraint;
            const auto [i0, i1, j0, j1] = distance_constraint;
            const double dxi = points[i1].x - points[i0].x;
            const double dyi = points[i1].y - points[i0].y;
            const double dxj = points[j1].x - x;
            const double dyj = points[j1].y - y;
            const double d2i = dxi * dxi + dyi * dyi;
            const double d2j = dxj * dxj + dyj * dyj;
            if(d2i >= d2j) {
                const double r2 = d2i - dyj * dyj; // r2 >= 0.0
                const double r = std::sqrt(std::max(r2, 0.0));
                x_min = std::max(x_min, points[j1].x - r);
                x_max = std::min(x_max, points[j1].x + r);
            } else {
                if(points[j0].x < points[j1].x) {
                    x_min = std::max(x_min, points[j0].x);
                    x_max = std::min(x_max, points[j1].x);
                } else {
                    x_min = std::max(x_min, points[j1].x);
                    x_max = std::min(x_max, points[j0].x);
                }
            }
        }

        for(const auto &indexed_istance_constraint :
            constraint_set.v_to_i0_constraints[v]) {
            const auto &distance_constraint =
                indexed_istance_constraint.distance_constraint;
            const auto [i0, i1, j0, j1] = distance_constraint;
            const double dxi = points[i1].x - x;
            const double dyi = points[i1].y - y;
            const double dxj = points[j1].x - points[j0].x;
            const double dyj = points[j1].y - points[j0].y;
            const double d2i = dxi * dxi + dyi * dyi;
            const double d2j = dxj * dxj + dyj * dyj;
            if(d2i >= d2j) {
                const double r2 = d2j - dyi * dyi;
                if(r2 >= 0.0) {
                    const double r = std::sqrt(r2);
                    if(points[i0].x < points[i1].x) {
                        x_max = std::min(x_max, points[i1].x - r);
                    } else {
                        x_min = std::max(x_min, points[i1].x + r);
                    }
                } else {
                    // Do nothing.
                }
            } else {
                if(points[i0].x < points[i1].x) {
                    x_max = std::min(x_max, points[i0].x);
                } else {
                    x_min = std::max(x_min, points[i0].x);
                }
            }
        }

        for(const auto &indexed_istance_constraint :
            constraint_set.v_to_i0j0_constraints[v]) {
            const auto &distance_constraint =
                indexed_istance_constraint.distance_constraint;
            const auto [i0, i1, j0, j1] = distance_constraint;
            const double dxi = points[i1].x - x;
            const double dyi = points[i1].y - y;
            const double dxj = points[j1].x - x;
            const double dyj = points[j1].y - y;
            const double d2i = dxi * dxi + dyi * dyi;
            const double d2j = dxj * dxj + dyj * dyj;
            if(d2i >= d2j) {
                const double y0 = y;
                const double x1 = points[i1].x;
                const double y1 = points[i1].y;
                const double x3 = points[j1].x;
                const double y3 = points[j1].y;
                if(x3 - x1 > 0.001) {
                    const double x =
                        (x3 * x3 - x1 * x1 + y3 * y3 - y1 * y1 + 2 * y0 * (y1 - y3)) /
                        (2.0 * (x3 - x1));
                    x_min = std::max(x_min, x);
                } else if(x3 - x1 < -0.001) {
                    const double x =
                        (x3 * x3 - x1 * x1 + y3 * y3 - y1 * y1 + 2 * y0 * (y1 - y3)) /
                        (2.0 * (x3 - x1));
                    x_max = std::min(x_max, x);
                } else {
                    // Do nothing.
                }
            } else {
                const double x1 = points[i1].x;
                const double x3 = points[j1].x;
                if(x3 - x1 > 0.001) {
                    x_min = std::max(x_min, points[i0].x);
                } else if(x3 - x1 < -0.001) {
                    x_max = std::min(x_max, points[i0].x);
                } else {
                    // Do nothing.
                }
            }
        }

        if(x_min > x_max) {
            // if (x_min - x_max > 10.0) {
            //   UNEXPECTED();
            // }
            points[v].x = (x_min + x_max) * 0.5;
            ranges[v].x_min = points[v].x;
            ranges[v].x_max = points[v].x;
        } else {
            if(is_medium_mode) {
                points[v].x = (x_min + x_max) * 0.5;
            } else {
                points[v].x = g_random.RandomDouble(x_min, x_max);
            }
            ranges[v].x_min = x_min;
            ranges[v].x_max = x_max;
        }

        x = points[v].x;

        // y
        double y_min =
            std::min(constraint_set.range_constraints[v].y_min, points[v].y);
        double y_max =
            std::max(constraint_set.range_constraints[v].y_max, points[v].y);

        for(const auto &indexed_istance_constraint :
            constraint_set.v_to_j0_constraints[v]) {
            const auto &distance_constraint =
                indexed_istance_constraint.distance_constraint;
            const auto [i0, i1, j0, j1] = distance_constraint;
            const double dxi = points[i1].x - points[i0].x;
            const double dyi = points[i1].y - points[i0].y;
            const double dxj = points[j1].x - x;
            const double dyj = points[j1].y - y;
            const double d2i = dxi * dxi + dyi * dyi;
            const double d2j = dxj * dxj + dyj * dyj;
            if(d2i >= d2j) {
                const double r2 = d2i - dxj * dxj; // r2 >= 0.0
                const double r = std::sqrt(std::fmax(r2, 0.0));
                y_min = std::max(y_min, points[j1].y - r);
                y_max = std::min(y_max, points[j1].y + r);
            } else {
                if(points[j0].y < points[j1].y) {
                    y_min = std::max(y_min, y);
                    y_max = std::min(y_max, points[j1].y);
                } else {
                    y_min = std::max(y_min, points[j1].y);
                    y_max = std::min(y_max, y);
                }
            }
        }

        for(const auto &indexed_istance_constraint :
            constraint_set.v_to_i0_constraints[v]) {
            const auto &distance_constraint =
                indexed_istance_constraint.distance_constraint;
            const auto [i0, i1, j0, j1] = distance_constraint;
            const double dxi = points[i1].x - x;
            const double dyi = points[i1].y - y;
            const double dxj = points[j1].x - points[j0].x;
            const double dyj = points[j1].y - points[j0].y;
            const double d2i = dxi * dxi + dyi * dyi;
            const double d2j = dxj * dxj + dyj * dyj;
            if(d2i >= d2j) {
                const double r2 = d2j - dxi * dxi;
                if(r2 > 0) {
                    const double r = std::sqrt(r2);
                    if(points[i0].y < points[i1].y) {
                        y_max = std::min(y_max, points[i1].y - r);
                    } else {
                        y_min = std::max(y_min, points[i1].y + r);
                    }
                }
            } else {
                if(points[i0].y < points[i1].y) {
                    y_max = std::min(y_max, y);
                } else {
                    y_min = std::max(y_min, y);
                }
            }
        }

        for(const auto &indexed_istance_constraint :
            constraint_set.v_to_i0j0_constraints[v]) {
            const auto &distance_constraint =
                indexed_istance_constraint.distance_constraint;
            const auto [i0, i1, j0, j1] = distance_constraint;
            const double dxi = points[i1].x - x;
            const double dyi = points[i1].y - y;
            const double dxj = points[j1].x - x;
            const double dyj = points[j1].y - y;
            const double d2i = dxi * dxi + dyi * dyi;
            const double d2j = dxj * dxj + dyj * dyj;
            if(d2i >= d2j) {
                const double x0 = x;
                const double x1 = points[i1].x;
                const double y1 = points[i1].y;
                const double x3 = points[j1].x;
                const double y3 = points[j1].y;
                if(y3 - y1 > 0.001) {
                    const double y =
                        (y3 * y3 - y1 * y1 + x3 * x3 - x1 * x1 + 2 * x0 * (x1 - x3)) /
                        (2.0 * (y3 - y1));
                    y_min = std::max(y_min, y);
                } else if(y3 - y1 < -0.001) {
                    const double y =
                        (y3 * y3 - y1 * y1 + x3 * x3 - x1 * x1 + 2 * x0 * (x1 - x3)) /
                        (2.0 * (y3 - y1));
                    y_max = std::min(y_max, y);
                } else {
                    // Do nothing.
                }
            } else {
                const double y1 = points[i1].y;
                const double y3 = points[j1].y;
                if(y3 - y1 > 0.001) {
                    y_min = std::max(y_min, y);
                } else if(y3 - y1 < -0.001) {
                    y_max = std::min(y_max, y);
                } else {
                    // Do nothing.
                }
            }
        }

        if(y_min > y_max) {
            // if (y_min - y_max > 10.0) {
            //   UNEXPECTED();
            // }
            points[v].y = (y_min + y_max) * 0.5;
            ranges[v].y_min = points[v].y;
            ranges[v].y_max = points[v].y;
        } else {
            if(is_medium_mode) {
                points[v].y = (y_min + y_max) * 0.5;
            } else {
                points[v].y = g_random.RandomDouble(y_min, y_max);
            }
            ranges[v].y_min = y_min;
            ranges[v].y_max = y_max;
        }
    }
}

void RunMCMCPhase(const std::array<Double2D, kVertexCount> &points,
                  const ConstraintSet &constraint_set,
                  std::vector<MCMCSample> &samples,
                  DoubleMatrix &distance_matrix) {
    samples.clear();
    for(const int i : std::views::iota(0, kVertexCount)) {
        for(const int j : std::views::iota(0, kVertexCount)) {
            distance_matrix.Set(i, j, 0.0);
        }
    }

    std::vector<int> vs;
    for(const int v : std::views::iota(0, kVertexCount)) {
        vs.push_back(v);
    }

    std::array<Double2D, kVertexCount> adjusted_points = points;
    std::vector<Double2DRange> ranges(kVertexCount);
    RunGibbsSampling(vs, constraint_set, adjusted_points, ranges, true);
    samples.push_back(MCMCSample(adjusted_points));
    for(const int i : std::views::iota(0, kVertexCount)) {
        for(const int j : std::views::iota(i + 1, kVertexCount)) {
            distance_matrix.Increment(
                i, j, Distance(samples[0].points[i], samples[0].points[j]));
        }
    }
    const int adjusted_sample_timie_limit =
        std::min(kSamplingTimeLimit, (g_time.CurrentMs() + 1900.0) * 0.5);
    while(samples.size() < 1000) {
        if(g_time.CurrentMs() > adjusted_sample_timie_limit) {
            std::cerr << "Emergency exit from sampling (" << samples.size()
                      << " samples)." << std::endl;
            break;
        }
        MCMCSample sample = samples.back();
        RunGibbsSampling(vs, constraint_set, sample.points, ranges);
        samples.push_back(sample);
        for(const int i : std::views::iota(0, kVertexCount)) {
            for(const int j : std::views::iota(i + 1, kVertexCount)) {
                distance_matrix.Increment(
                    i, j, Distance(sample.points[i], sample.points[j]));
            }
        }
    }

    const double factor = 1.0 / samples.size();
    for(const int i : std::views::iota(0, kVertexCount)) {
        for(const int j : std::views::iota(i + 1, kVertexCount)) {
            distance_matrix.Set(i, j, distance_matrix.Get(i, j) * factor);
            distance_matrix.Set(j, i, distance_matrix.Get(i, j));
        }
    }
}

#endif // MCM_CPP_

bool RunGradientDescent(
    const ConstraintSet &constraint_set,
    const double alpha,
    std::array<Double2D, kVertexCount> &points,
    IndexSet &updated_vertex_set,
    IndexSet &added_constraint_index_set) {
    static std::vector<int> current_updated_vs;
    current_updated_vs = updated_vertex_set.Values();

    added_constraint_index_set.Clear();

    auto expand_v = [&](const int v, int min_index) {
        for(const auto constraint_index :
            constraint_set.v_to_distance_constraint_indexes[v]) {
            if(constraint_index < min_index) {
                continue;
            }
            if(added_constraint_index_set.Contains(constraint_index)) {
                continue;
            }
            added_constraint_index_set.Add(constraint_index);
        }
    };

    for(const int v : updated_vertex_set.Values()) {
        expand_v(v, -1);
    }

    updated_vertex_set.Clear();

    for(const int constraint_index : added_constraint_index_set.Values()) {
        const auto &distance_constraint =
            constraint_set.distance_constraints[constraint_index];
        const auto &[i0, i1, j0, j1] = distance_constraint;
        const double dxi = points[i1].x - points[i0].x;
        const double dyi = points[i1].y - points[i0].y;
        const double dxj = points[j1].x - points[j0].x;
        const double dyj = points[j1].y - points[j0].y;
        const double d2i = dxi * dxi + dyi * dyi;
        const double d2j = dxj * dxj + dyj * dyj;
        if(d2i < d2j) {
            const double di = std::sqrt(d2i + 0.001);
            const double dj = std::sqrt(d2j + 0.001);
            const double ndxi = dxi / di;
            const double ndyi = dyi / di;
            const double ndxj = dxj / dj;
            const double ndyj = dyj / dj;
            points[i0].x += -alpha * ndxi;
            points[i0].y += -alpha * ndyi;
            points[i1].x += alpha * ndxi;
            points[i1].y += alpha * ndyi;
            points[j0].x += alpha * ndxj;
            points[j0].y += alpha * ndyj;
            points[j1].x += -alpha * ndxj;
            points[j1].y += -alpha * ndyj;
            updated_vertex_set.Add(i0);
            updated_vertex_set.Add(i1);
            updated_vertex_set.Add(j0);
            updated_vertex_set.Add(j1);
        }
    }

    for(const int i : current_updated_vs) {
        const auto &[unused_i, x_min, x_max, y_min, y_max] =
            constraint_set.range_constraints[i];
        const double x_center = (x_min + x_max) / 2.0;
        const double y_center = (y_min + y_max) / 2.0;
        if(points[i].x < x_min) {
            points[i].x += alpha;
            updated_vertex_set.Add(i);
        } else if(points[i].x > x_max) {
            points[i].x += -alpha;
            updated_vertex_set.Add(i);
        }
        if(points[i].y < y_min) {
            points[i].y += alpha;
            updated_vertex_set.Add(i);
        } else if(points[i].y > y_max) {
            points[i].y += -alpha;
            updated_vertex_set.Add(i);
        }
    }

    return updated_vertex_set.Size() > 0;
}

#endif // OPTIMIZATION_BASE_CPP_

#ifndef SOLVER_CPP_
#define SOLVER_CPP_

#ifndef CLUSTERING_CPP_
#define CLUSTERING_CPP_

#ifndef MST_CPP_
#define MST_CPP_

std::vector<Edge> FindMST(const int vertex_count,
                          const DoubleMatrix &distance_matrix) {
    std::vector<Edge> result;
    result.reserve(vertex_count - 1);

    std::vector<uint8> visited(vertex_count, false);
    std::vector<double> min_weights(vertex_count,
                                    std::numeric_limits<double>::max());
    std::vector<int> parents(vertex_count, -1);
    visited[0] = true;
    min_weights[0] = 0.0;

    int min_weight_v = 0;
    for(int count = 0; count < vertex_count - 1; ++count) {
        for(int v = 0; v < vertex_count; ++v) {
            if(!visited[v]) {
                double edge_weight = distance_matrix.Get(min_weight_v, v);
                if(edge_weight < min_weights[v]) {
                    min_weights[v] = edge_weight;
                    parents[v] = min_weight_v;
                }
            }
        }

        double min_weight = std::numeric_limits<double>::max();
        for(int v = 0; v < vertex_count; ++v) {
            if(!visited[v] && min_weights[v] < min_weight) {
                min_weight = min_weights[v];
                min_weight_v = v;
            }
        }

        visited[min_weight_v] = true;
        result.emplace_back(parents[min_weight_v], min_weight_v);
    }
    return result;
}

// Assumption: vertex_count <= N
template <std::size_t N>
void FindMSTOptimized(const int vertex_count,
                      const DoubleMatrix &distance_matrix,
                      std::vector<Edge> &result) {
    static std::array<uint8, N> visited = {};
    static std::array<double, N> min_weights = {};
    static std::array<int, N> parents = {};

    for(const int v : std::views::iota(0, vertex_count)) {
        visited[v] = false;
        min_weights[v] = std::numeric_limits<double>::max();
        parents[v] = -1;
    }

    visited[0] = true;
    min_weights[0] = 0.0;

    int min_weight_v = 0;
    for(int count = 0; count < vertex_count - 1; ++count) {
        for(int v = 0; v < vertex_count; ++v) {
            if(!visited[v]) {
                double edge_weight = distance_matrix.Get(min_weight_v, v);
                if(edge_weight < min_weights[v]) {
                    min_weights[v] = edge_weight;
                    parents[v] = min_weight_v;
                }
            }
        }

        double min_weight = std::numeric_limits<double>::max();
        for(int v = 0; v < vertex_count; ++v) {
            if(!visited[v] && min_weights[v] < min_weight) {
                min_weight = min_weights[v];
                min_weight_v = v;
            }
        }

        visited[min_weight_v] = true;
        result.emplace_back(parents[min_weight_v], min_weight_v);
    }
}

// Find MST for a specified set of vertices.
std::vector<Edge> FindMST(const std::vector<int> &vs,
                          const DoubleMatrix &distance_matrix) {
    std::vector<Edge> result;
    result.reserve(vs.size() - 1);

    std::vector<uint8> visited(vs.size(), false);
    std::vector<double> min_weights(vs.size(),
                                    std::numeric_limits<double>::max());
    std::vector<int> parents(vs.size(), -1);
    visited[0] = true;
    min_weights[0] = 0.0;

    int min_weight_i = 0;
    for(int count = 0; count < vs.size() - 1; ++count) {
        for(const int i : std::views::iota(0, int(vs.size()))) {
            if(!visited[i]) {
                double edge_weight = distance_matrix.Get(vs[min_weight_i], vs[i]);
                if(edge_weight < min_weights[i]) {
                    min_weights[i] = edge_weight;
                    parents[i] = min_weight_i;
                }
            }
        }

        double min_weight = std::numeric_limits<double>::max();
        for(const int i : std::views::iota(0, int(vs.size()))) {
            if(!visited[i] && min_weights[i] < min_weight) {
                min_weight = min_weights[i];
                min_weight_i = i;
            }
        }

        visited[min_weight_i] = true;
        result.emplace_back(vs[parents[min_weight_i]], vs[min_weight_i]);
    }
    return result;
}

#endif

// Returns whether two edges are equal (undirected).
bool UndirectedEdgeEqual(const Edge &e0, const Edge &e1) {
    return (e0.s == e1.s && e0.t == e1.t) || (e0.s == e1.t && e0.t == e1.s);
}

// ---------------------
// Extract MST edges that have both endpoints in the given component
// ---------------------
void ExtractMstEdges(const std::vector<int> &component,
                     const std::vector<Edge> &mst_edges,
                     std::vector<Edge> &component_mst_edges) {
    for(const auto &edge : mst_edges) {
        if(find(component.begin(), component.end(), edge.s) != component.end() &&
           find(component.begin(), component.end(), edge.t) != component.end())
            component_mst_edges.push_back(edge);
    }
}

// ---------------------
// Recursive clustering function.
// vs: The current set of vertices.
// mst_edges: MST edges for vs
// cluster_ids: Indices (into target_sizes) for clusters that must be assigned
//              to vs. target_sizes remains constant globally.
// points: Coordinates for all vertices.
// assignment: Output assignment for vertices; assignment[i] is the cluster id
//             for vertex i.
bool RunClustering(const ProblemInput &input, const std::vector<int> &vs,
                   const std::vector<Edge> &mst_edges,
                   const std::vector<int> &cluster_ids,
                   const std::array<Double2D, kVertexCount> &points,
                   const DoubleMatrix &distance_matrix,
                   std::vector<int> &assignment) {
    // Base case: only one cluster remains.
    if(cluster_ids.size() == 1) {
        if((int)vs.size() != input.cluster_sizes[cluster_ids[0]]) {
            UNEXPECTED();
        }
        for(int v : vs) {
            assignment[v] = cluster_ids[0];
        }
        return true;
    }

    // Build the current target sizes vector (for the clusters in cluster_ids)
    std::vector<int> current_target_sizes;
    for(int cluster_id : cluster_ids) {
        current_target_sizes.push_back(input.cluster_sizes[cluster_id]);
    }

    int total = std::accumulate(current_target_sizes.begin(),
                                current_target_sizes.end(), 0);

    SubsetSumSolver subset_sum_solver;
    subset_sum_solver.Solve(current_target_sizes, int(vs.size()));

    std::vector<Edge> undirected_mst_edges;
    for(const auto &edge : mst_edges) {
        undirected_mst_edges.push_back(Edge(edge.s, edge.t));
        undirected_mst_edges.push_back(Edge(edge.t, edge.s));
    }
    Graph graph(kVertexCount, undirected_mst_edges);

    std::vector<int> parents(kVertexCount, -1);
    std::vector<int> sizes(kVertexCount, -1);

    auto dfs = [&](auto &&dfs, const int v, const int p) -> void {
        sizes[v] = 1;
        for(const Edge &edge : graph.OutEdges(v)) {
            if(edge.t == p)
                continue;
            parents[edge.t] = v;
            dfs(dfs, edge.t, v);
            sizes[v] += sizes[edge.t];
        }
    };
    dfs(dfs, vs[0], -1);

    // Sort MST edges in descending order of weight (using squared Euclidean
    // distance).
    std::vector<Edge> sorted_edges = mst_edges;
    auto edge_weight = [&](const Edge &e) -> double {
        return distance_matrix.Get(e.s, e.t);
    };
    sort(sorted_edges.begin(), sorted_edges.end(),
         [&](const Edge &a, const Edge &b) {
             return edge_weight(a) > edge_weight(b);
         });

    // Try removing heavy edges and check if the resulting split fits a valid
    // partition.
    for(const auto &target_edge : sorted_edges) {
        int v0 = target_edge.s;
        int v1 = target_edge.t;
        if(parents[v1] == v0) {
            // Do nothing.
        } else if(parents[v0] == v1) {
            std::swap(v0, v1);
        } else {
            UNEXPECTED();
        }

        const int size_0 = int(vs.size()) - sizes[v1];
        const int size_1 = sizes[v1];
        if(!subset_sum_solver.SubsetExists(size_0)) {
            continue;
        }

        // Retrieve subset indices (in current_target_sizes) that sum to size0.
        std::vector<int> cluster_indexes_0 =
            subset_sum_solver.GetSubset(size_0);
        std::vector<int> cluster_ids_0;
        for(const int cluster_index : cluster_indexes_0) {
            cluster_ids_0.push_back(cluster_ids[cluster_index]);
        }

        // Partition cluster_ids into two groups: one for comp0 and one for comp1.
        std::vector<int> in_cluster_ids_0(input.cluster_count, 0);
        for(const int cluster_id : cluster_ids_0) {
            in_cluster_ids_0[cluster_id] = 1;
        }
        std::vector<int> cluster_ids_1;
        for(const int cluster_id : cluster_ids) {
            if(!in_cluster_ids_0[cluster_id]) {
                cluster_ids_1.push_back(cluster_id);
            }
        }

        // Get vertices in each component using DFS
        std::vector<int> comp0, comp1;
        auto get_subtree_vertices =
            [&](auto &&get_subtree_vertices, const int v, const int p,
                std::vector<int> &subtree_vertices) -> void {
            subtree_vertices.push_back(v);
            for(const auto &edge : graph.OutEdges(v)) {
                if(edge.t != p) {
                    get_subtree_vertices(get_subtree_vertices, edge.t, v,
                                         subtree_vertices);
                }
            }
        };
        get_subtree_vertices(get_subtree_vertices, v1, v0, comp1);

        // Get component 0 vertices as complement of component 1
        std::vector<int> in_comp1(kVertexCount);
        for(const int v : comp1) {
            in_comp1[v] = 1;
        }
        for(const int v : vs) {
            if(!in_comp1[v]) {
                comp0.push_back(v);
            }
        }

        // Extract MST edges corresponding to each connected component.
        std::vector<Edge> mst_edges0, mst_edges1;
        ExtractMstEdges(comp0, mst_edges, mst_edges0);
        ExtractMstEdges(comp1, mst_edges, mst_edges1);

        // Recurse on both components.
        if(RunClustering(input, comp0, mst_edges0, cluster_ids_0, points,
                         distance_matrix, assignment) &&
           RunClustering(input, comp1, mst_edges1, cluster_ids_1, points,
                         distance_matrix, assignment)) {
            return true;
        } else {
            UNEXPECTED();
        }
    }

    // No valid split found.
    int target_size = 0;
    for(int size = int(vs.size()) / 2; size >= 1; size--) {
        if(subset_sum_solver.SubsetExists(size)) {
            target_size = size;
            break;
        }
    }
    if(target_size == 0) {
        UNEXPECTED();
    }

    std::vector<int> cluster_indexes_0 =
        subset_sum_solver.GetSubset(target_size);
    std::vector<int> cluster_ids_0;
    for(const int cluster_index : cluster_indexes_0) {
        cluster_ids_0.push_back(cluster_ids[cluster_index]);
    }

    // Partition cluster_ids into two groups: one for comp0 and one for comp1.
    std::vector<int> in_cluster_ids_0(input.cluster_count, 0);
    for(const int cluster_id : cluster_ids_0) {
        in_cluster_ids_0[cluster_id] = 1;
    }
    std::vector<int> cluster_ids_1;
    for(const int cluster_id : cluster_ids) {
        if(!in_cluster_ids_0[cluster_id]) {
            cluster_ids_1.push_back(cluster_id);
        }
    }

    // for (const int i : std::views::iota(0, int(cluster_ids.size()) / 2)) {
    //   target_size += input.cluster_sizes[cluster_ids[i]];
    //   cluster_ids_0.push_back(cluster_ids[i]);
    // }
    // for (const int i :
    //      std::views::iota(int(cluster_ids.size()) / 2, int(cluster_ids.size()))) {
    //   cluster_ids_1.push_back(cluster_ids[i]);
    // }

    std::vector<int> sorted_vertices = vs;
    std::sort(sorted_vertices.begin(), sorted_vertices.end(),
              [&points](int v0, int v1) {
                  if(points[v0].x != points[v1].x) {
                      return points[v0].x < points[v1].x;
                  }
                  return points[v0].y < points[v1].y;
              });

    std::vector<int> vs_0(sorted_vertices.begin(),
                          sorted_vertices.begin() + target_size);
    std::vector<int> vs_1(sorted_vertices.begin() + target_size,
                          sorted_vertices.end());

    // Do K-means.
    for(const int iter : std::views::iota(0, 10)) {
        // Calculate the mean (kernel) of vs_0 and vs_1
        Double2D kernel_0 = {0.0, 0.0};
        Double2D kernel_1 = {0.0, 0.0};

        // Calculate kernel_0 (mean of vs_0)
        for(const int v : vs_0) {
            kernel_0.x += points[v].x;
            kernel_0.y += points[v].y;
        }
        if(!vs_0.empty()) {
            kernel_0.x /= vs_0.size();
            kernel_0.y /= vs_0.size();
        }

        // Calculate kernel_1 (mean of vs_1)
        for(const int v : vs_1) {
            kernel_1.x += points[v].x;
            kernel_1.y += points[v].y;
        }
        if(!vs_1.empty()) {
            kernel_1.x /= vs_1.size();
            kernel_1.y /= vs_1.size();
        }

        // Sort vertices by the difference in distance to the two kernels
        std::vector<std::pair<double, int>> distance_diffs;
        for(const int v : sorted_vertices) {
            double dist_to_kernel_0 = Distance2(points[v], kernel_0);
            double dist_to_kernel_1 = Distance2(points[v], kernel_1);
            double diff = dist_to_kernel_0 - dist_to_kernel_1;
            distance_diffs.push_back({diff, v});
        }

        // Sort by difference in distance (vertices closer to kernel_0 will come
        // first)
        std::sort(distance_diffs.begin(), distance_diffs.end());

        // Update vs_0 and vs_1 based on the sorted differences
        vs_0.clear();
        vs_1.clear();

        // Take the first target_size vertices for vs_0
        for(int i = 0; i < target_size; ++i) {
            vs_0.push_back(distance_diffs[i].second);
        }

        // Take the remaining vertices for vs_1
        for(size_t i = target_size; i < distance_diffs.size(); ++i) {
            vs_1.push_back(distance_diffs[i].second);
        }
    }
    std::vector<Edge> mst_0 = FindMST(vs_0, distance_matrix);
    std::vector<Edge> mst_1 = FindMST(vs_1, distance_matrix);
    if(RunClustering(input, vs_0, mst_0, cluster_ids_0, points, distance_matrix,
                     assignment) &&
       RunClustering(input, vs_1, mst_1, cluster_ids_1, points, distance_matrix,
                     assignment)) {
        return true;
    }

    UNEXPECTED();
    return false;
}

void RunClusteringPhase(const ProblemInput &input,
                        const std::vector<int> &reserved_isolated_vertices,
                        const std::array<Double2D, kVertexCount> &points,
                        const DoubleMatrix &distance_matrix,
                        std::vector<int> &assignment) {
    std::array<int, kVertexCount> v_to_reserved{};
    for(const int v : reserved_isolated_vertices) {
        v_to_reserved[v] = 1;
    }

    int reserved_cluster_count = 0;
    std::vector<int> cluster_to_reserved(input.cluster_count);
    if(reserved_isolated_vertices.size() > 0) {
        for(const int cluster_id : std::views::iota(0, input.cluster_count)) {
            if(input.cluster_sizes[cluster_id] == 1) {
                cluster_to_reserved[cluster_id] = 1;
                assignment[reserved_isolated_vertices[reserved_cluster_count]] =
                    cluster_id;
                reserved_cluster_count++;
                if(reserved_cluster_count == reserved_isolated_vertices.size()) {
                    break;
                }
            }
        }
    }

    std::vector<int> vs;
    vs.reserve(kVertexCount);
    for(const int i : std::views::iota(0, kVertexCount)) {
        if(v_to_reserved[i]) {
            continue;
        }
        vs.push_back(i);
    }
    std::vector<Edge> mst_edges = FindMST(vs, distance_matrix);
    std::vector<int> cluster_ids;
    cluster_ids.reserve(input.cluster_count);
    for(const int i : std::views::iota(0, input.cluster_count)) {
        if(cluster_to_reserved[i]) {
            continue;
        }
        cluster_ids.push_back(i);
    }

    if(!RunClustering(input, vs, mst_edges, cluster_ids, points, distance_matrix,
                      assignment)) {
        UNEXPECTED();
    }
}

#endif // CLUSTERING_CPP_

#ifndef CLUSTERING_ITERATION_CPP
#define CLUSTERING_ITERATION_CPP

#include <cmath>

constexpr double kClusteringIterationTimeLimit = 1900.0;

// Assumption: score is to maximize.
bool AcceptAnnealing(const double score_delta, const double temperature) {
    if(temperature == 0.0) {
        return score_delta >= 0.0;
    }
    if(score_delta > 0)
        return true;
    if(score_delta < -3.0 * temperature)
        return false;
    const double probability = std::exp(score_delta / temperature);
    return g_random.RandomDouble(0.0, 1.0) < probability;
}

double ExpInterpolation(const double x0, const double y0, const double x1,
                        const double y1, const double x) {
    const double log_y0 = std::log(y0);
    const double log_y1 = std::log(y1);
    const double log_y = log_y0 + (x - x0) * (log_y1 - log_y0) / (x1 - x0);
    return std::exp(log_y);
}

struct ClusteringIterationConfig {
    std::array<Double2D, kVertexCount> points;
    DoubleMatrix distance_matrix;
    ClusteringIterationConfig(const std::array<Double2D, kVertexCount> &points,
                              const DoubleMatrix &distance_matrix)
        : points(points), distance_matrix(distance_matrix) {
    }
};

struct ClusteringIterationState {
    std::vector<int> assignment;
    std::vector<std::vector<int>> cluster_to_vs;
    std::vector<std::vector<Edge>> cluster_to_mst;
    std::vector<double> cluster_to_mst_weight_sum;
    std::vector<double> cluster_to_mst_weight_max;
    std::vector<std::vector<Edge>> v_to_edges;
    double score = 0.0;
    ClusteringIterationState(const int cluster_count)
        : assignment(kVertexCount),
          cluster_to_vs(cluster_count),
          cluster_to_mst(cluster_count),
          cluster_to_mst_weight_sum(cluster_count),
          cluster_to_mst_weight_max(cluster_count),
          v_to_edges(kVertexCount) {
    }
};

void SetClusterToState(const ClusteringIterationConfig &config,
                       const int cluster_id, const std::vector<int> &vs,
                       const std::vector<Edge> &mst,
                       ClusteringIterationState &state) {
    state.score -= state.cluster_to_mst_weight_sum[cluster_id];

    for(const int v : vs) {
        state.assignment[v] = cluster_id;
    }
    state.cluster_to_vs[cluster_id] = vs;
    state.cluster_to_mst[cluster_id] = mst;
    for(const int v : vs) {
        state.v_to_edges[v].clear();
    }
    double weight_sum = 0.0;
    double weight_max = 0.0;
    for(const Edge &e : mst) {
        state.v_to_edges[e.s].push_back(e);
        state.v_to_edges[e.t].push_back(Edge(e.t, e.s));
        weight_sum += config.distance_matrix.Get(e.s, e.t);
        weight_max = std::max(weight_max, config.distance_matrix.Get(e.s, e.t));
    }
    state.cluster_to_mst_weight_sum[cluster_id] = weight_sum;
    state.cluster_to_mst_weight_max[cluster_id] = weight_max;

    state.score += weight_sum;
}

// The result of clustering into 2 clusters.
struct Clustering2Result {
    std::vector<int> vs_0;
    std::vector<int> vs_1;
    std::vector<Edge> mst_0;
    std::vector<Edge> mst_1;
    // The sum of weights of mst_0 and mst_1.
    double score;
};

std::vector<std::pair<int, int>> GetClosePairs(
    const DoubleMatrix &distance_matrix) {
    std::vector<std::pair<int, int>> close_pairs;
    // For each vertex, find the 10 closest vertices.
    for(int v = 0; v < kVertexCount; v++) {
        // Create a vector of pairs (distance, vertex_id) for sorting
        std::vector<std::pair<double, int>> distance_v_pairs;
        for(int u = 0; u < kVertexCount; u++) {
            if(v == u)
                continue; // Skip self
            distance_v_pairs.push_back({distance_matrix.Get(v, u), u});
        }

        // Sort by distance (ascending)
        std::sort(distance_v_pairs.begin(), distance_v_pairs.end());

        // Take the top 10 closest vertices.
        for(const int i : std::views::iota(0, 10)) {
            int u = distance_v_pairs[i].second;
            // Store the vertex pair (smaller index first for consistency)
            if(v < u) {
                close_pairs.push_back({v, u});
            } else {
                close_pairs.push_back({u, v});
            }
        }
    }
    // Remove duplicate pairs
    std::sort(close_pairs.begin(), close_pairs.end());
    close_pairs.erase(std::unique(close_pairs.begin(), close_pairs.end()),
                      close_pairs.end());
    return close_pairs;
}

double EdgesWeight(const ClusteringIterationConfig &config,
                   const std::vector<Edge> &edges) {
    double w = 0.0;
    for(const Edge &e : edges) {
        w += config.distance_matrix.Get(e.s, e.t);
    }
    return w;
}

bool TryVertexSwap(const ClusteringIterationConfig &config,
                   const double temperature, ClusteringIterationState &state,
                   const int v0, const int c0, const int v1, const int c1) {
    if(state.v_to_edges[v0].size() != 1 || state.v_to_edges[v1].size() != 1) {
        return false;
    }

    int p0 = state.v_to_edges[v0][0].t;
    int p1 = state.v_to_edges[v1][0].t;

    const double d00 = config.distance_matrix.Get(v0, p0);
    const double d01 = config.distance_matrix.Get(v0, p1);
    const double d10 = config.distance_matrix.Get(v1, p0);
    const double d11 = config.distance_matrix.Get(v1, p1);
    const double score_delta = (d01 + d10) - (d00 + d11);
    if(!AcceptAnnealing(-score_delta, temperature)) {
        return false;
    }

    std::vector<int> vs_0 = state.cluster_to_vs[c0];
    std::vector<int> vs_1 = state.cluster_to_vs[c1];
    vs_0.erase(std::find(vs_0.begin(), vs_0.end(), v0));
    vs_0.push_back(v1);
    vs_1.erase(std::find(vs_1.begin(), vs_1.end(), v1));
    vs_1.push_back(v0);

    const auto mst0 = FindMST(vs_0, config.distance_matrix);
    const auto mst1 = FindMST(vs_1, config.distance_matrix);
    SetClusterToState(config, c0, vs_0, mst0, state);
    SetClusterToState(config, c1, vs_1, mst1, state);
    return true;
}

std::optional<Clustering2Result> MaybeDoClustering2ByMergedMST(
    const ClusteringIterationConfig &config, const std::vector<int> &vs,
    const int target_size_0, const int target_size_1) {
    const auto mst = FindMST(vs, config.distance_matrix);
    const double merged_mst_w = EdgesWeight(config, mst);

    static std::vector<int> parents(kVertexCount, -1);
    static std::vector<int> sizes(kVertexCount, -1);
    static std::vector<std::vector<Edge>> v_to_merged_mst_edges(kVertexCount);
    // Reset static variables before use.
    for(const int v : vs) {
        parents[v] = -1;
        sizes[v] = -1;
        v_to_merged_mst_edges[v].clear();
    }
    for(const Edge &e : mst) {
        v_to_merged_mst_edges[e.s].push_back(e);
        v_to_merged_mst_edges[e.t].push_back(Edge(e.t, e.s));
    }

    auto dfs = [&](auto &&dfs, const int v, const int p) -> void {
        sizes[v] = 1;
        for(const Edge &edge : v_to_merged_mst_edges[v]) {
            if(edge.t == p)
                continue;
            parents[edge.t] = v;
            dfs(dfs, edge.t, v);
            sizes[v] += sizes[edge.t];
        }
    };
    dfs(dfs, vs[0], -1);

    // Sort MST edges in descending order of weight (using squared Euclidean
    // distance).
    std::vector<Edge> sorted_edges = mst;
    auto edge_weight = [&](const Edge &e) -> double {
        return config.distance_matrix.Get(e.s, e.t);
    };
    sort(sorted_edges.begin(), sorted_edges.end(),
         [&](const Edge &a, const Edge &b) {
             return edge_weight(a) > edge_weight(b);
         });

    // Try removing heavy edges and check if the resulting split fits a valid
    // partition.
    for(const auto &target_edge : sorted_edges) {
        int v0 = target_edge.s;
        int v1 = target_edge.t;
        if(parents[v1] == v0) {
            // Do nothing.
        } else if(parents[v0] == v1) {
            std::swap(v0, v1);
        } else {
            UNEXPECTED();
        }

        const int size_0 = int(vs.size()) - sizes[v1];
        const int size_1 = sizes[v1];
        if(!((size_0 == target_size_0 && size_1 == target_size_1) ||
             (size_0 == target_size_1 && size_1 == target_size_0))) {
            continue;
        }

        std::vector<int> component_0, component_1;
        auto get_subtree_vertices =
            [&](auto &&get_subtree_vertices, const int v, const int p,
                std::vector<int> &subtree_vertices) -> void {
            subtree_vertices.push_back(v);
            for(const auto &edge : v_to_merged_mst_edges[v]) {
                if(edge.t != p) {
                    get_subtree_vertices(get_subtree_vertices, edge.t, v,
                                         subtree_vertices);
                }
            }
        };
        get_subtree_vertices(get_subtree_vertices, v1, v0, component_1);

        // Get component 0 vertices as complement of component 1
        static std::vector<int> in_comp1(kVertexCount);
        // Reset before use.
        for(const int v : vs) {
            in_comp1[v] = 0;
        }
        for(const int v : component_1) {
            in_comp1[v] = 1;
        }
        for(const int v : vs) {
            if(!in_comp1[v]) {
                component_0.push_back(v);
            }
        }

        if(size_0 != target_size_0) {
            std::swap(component_0, component_1);
        }

        std::vector<Edge> mst0 = FindMST(component_0, config.distance_matrix);
        std::vector<Edge> mst1 = FindMST(component_1, config.distance_matrix);
        return Clustering2Result{
            component_0, component_1, mst0, mst1,
            EdgesWeight(config, mst0) + EdgesWeight(config, mst1)};
    }
    return std::nullopt;
}

bool TryUpdateTwoClustersFromMergedMST(const ClusteringIterationConfig &config,
                                       const double temperature, const int c0,
                                       const int c1,
                                       ClusteringIterationState &state) {
    const int size_0 = state.cluster_to_vs[c0].size();
    const int size_1 = state.cluster_to_vs[c1].size();

    const double base_w =
        state.cluster_to_mst_weight_sum[c0] + state.cluster_to_mst_weight_sum[c1];

    std::vector<int> vs = state.cluster_to_vs[c0];
    vs.insert(vs.end(), state.cluster_to_vs[c1].begin(),
              state.cluster_to_vs[c1].end());

    std::optional<Clustering2Result> result =
        MaybeDoClustering2ByMergedMST(config, vs, size_0, size_1);
    if(!result) {
        return false;
    }

    const double new_w = result->score;
    const double score_delta = new_w - base_w;

    // Likely no change.
    // TODO: Consider proper implementation.
    if(std::abs(score_delta) < 0.001) {
        return false;
    }

    if(!AcceptAnnealing(-score_delta, temperature)) {
        return false;
    }

    SetClusterToState(config, c0, result->vs_0, result->mst_0, state);
    SetClusterToState(config, c1, result->vs_1, result->mst_1, state);
    return true;
}

Clustering2Result DoClustering2ByKMeans(const ClusteringIterationConfig &config,
                                        const std::vector<int> &vs,
                                        const int size_0, const int size_1) {
    std::vector<int> shuffled_vs = vs;
    const double theta = g_random.RandomDouble(0.0, 2.0 * M_PI);
    // Sort vertices by projecting them onto a random line with angle theta
    std::sort(shuffled_vs.begin(), shuffled_vs.end(), [&](int a, int b) {
        const double z_a = config.points[a].x * std::cos(theta) +
                           config.points[a].y * std::sin(theta);
        const double z_b = config.points[b].x * std::cos(theta) +
                           config.points[b].y * std::sin(theta);
        return z_a < z_b;
    });
    // for (const int i : std::views::iota(0, int(vs.size()) - 1)) {
    //   int j = g_random.RandomInt(i, vs.size());
    //   std::swap(shuffled_vs[i], shuffled_vs[j]);
    // }

    std::vector<int> vs_0;
    std::vector<int> vs_1;
    for(const int i : std::views::iota(0, size_0)) {
        vs_0.push_back(shuffled_vs[i]);
    }
    for(const int i : std::views::iota(size_0, size_0 + size_1)) {
        vs_1.push_back(shuffled_vs[i]);
    }

    for(const int iter : std::views::iota(0, 10)) {
        // Calculate the mean (kernel) of vs_0 and vs_1
        Double2D kernel_0 = {0.0, 0.0};
        Double2D kernel_1 = {0.0, 0.0};

        // Calculate kernel_0 (mean of vs_0)
        for(const int v : vs_0) {
            kernel_0.x += config.points[v].x;
            kernel_0.y += config.points[v].y;
        }
        if(!vs_0.empty()) {
            kernel_0.x /= vs_0.size();
            kernel_0.y /= vs_0.size();
        }

        // Calculate kernel_1 (mean of vs_1)
        for(const int v : vs_1) {
            kernel_1.x += config.points[v].x;
            kernel_1.y += config.points[v].y;
        }
        if(!vs_1.empty()) {
            kernel_1.x /= vs_1.size();
            kernel_1.y /= vs_1.size();
        }
        // Sort vertices by the difference in distance to the two kernels
        std::vector<std::pair<double, int>> distance_diffs;
        for(const int v : vs) {
            double dist_to_kernel_0 = Distance2(config.points[v], kernel_0);
            double dist_to_kernel_1 = Distance2(config.points[v], kernel_1);
            double diff = dist_to_kernel_0 - dist_to_kernel_1;
            distance_diffs.push_back({diff, v});
        }

        // Sort by difference in distance (vertices closer to kernel_0 will come
        // first)
        std::sort(distance_diffs.begin(), distance_diffs.end());

        // Update vs_0 and vs_1 based on the sorted differences
        vs_0.clear();
        vs_1.clear();

        // Take the first target_size vertices for vs_0
        for(int i = 0; i < size_0; ++i) {
            vs_0.push_back(distance_diffs[i].second);
        }

        // Take the remaining vertices for vs_1
        for(size_t i = size_0; i < distance_diffs.size(); ++i) {
            vs_1.push_back(distance_diffs[i].second);
        }
    }

    const auto mst0 = FindMST(vs_0, config.distance_matrix);
    const auto mst1 = FindMST(vs_1, config.distance_matrix);
    double new_w = EdgesWeight(config, mst0) + EdgesWeight(config, mst1);
    return {vs_0, vs_1, mst0, mst1, new_w};
}

bool TryUpdateTwoClustersFromKMeans(const ClusteringIterationConfig &config,
                                    const double temperature, const int c0,
                                    const int c1,
                                    ClusteringIterationState &state) {
    std::vector<int> vs = state.cluster_to_vs[c0];
    vs.insert(vs.end(), state.cluster_to_vs[c1].begin(),
              state.cluster_to_vs[c1].end());
    const Clustering2Result result =
        DoClustering2ByKMeans(config, vs, state.cluster_to_vs[c0].size(),
                              state.cluster_to_vs[c1].size());

    const double base_w = EdgesWeight(config, state.cluster_to_mst[c0]) +
                          EdgesWeight(config, state.cluster_to_mst[c1]);
    double new_w = result.score;
    const double score_delta = new_w - base_w;

    // Likely no change.
    // TODO: Consider proper implementation.
    if(std::abs(score_delta) < 0.001) {
        return false;
    }

    if(!AcceptAnnealing(-score_delta, temperature)) {
        return false;
    }

    SetClusterToState(config, c0, result.vs_0, result.mst_0, state);
    SetClusterToState(config, c1, result.vs_1, result.mst_1, state);
    return true;
}

// Assumption: size(c0) + size(c1) == size(c2)
bool TryUpdateByOneTwoClusterSwap(const ClusteringIterationConfig &config,
                                  const double temperature, const int c0,
                                  const int c1, const int c2,
                                  ClusteringIterationState &state) {
    const double base_w = state.cluster_to_mst_weight_sum[c0] +
                          state.cluster_to_mst_weight_sum[c1] +
                          state.cluster_to_mst_weight_sum[c2];

    std::vector<int> vs_2 = state.cluster_to_vs[c0];
    vs_2.insert(vs_2.end(), state.cluster_to_vs[c1].begin(),
                state.cluster_to_vs[c1].end());
    std::vector<Edge> mst_2 = FindMST(vs_2, config.distance_matrix);

    std::vector<int> vs = state.cluster_to_vs[c2];

    std::optional<Clustering2Result> opt_merged_mst_result =
        MaybeDoClustering2ByMergedMST(config, vs, state.cluster_to_vs[c0].size(),
                                      state.cluster_to_vs[c1].size());
    if(opt_merged_mst_result) {
        const double new_w =
            opt_merged_mst_result->score + EdgesWeight(config, mst_2);
        const double score_delta = new_w - base_w;
        if(AcceptAnnealing(-score_delta, temperature)) {
            SetClusterToState(config, c0, opt_merged_mst_result->vs_0,
                              opt_merged_mst_result->mst_0, state);
            SetClusterToState(config, c1, opt_merged_mst_result->vs_1,
                              opt_merged_mst_result->mst_1, state);
            SetClusterToState(config, c2, vs_2, mst_2, state);
            return true;
        }
    }

    Clustering2Result k_means_result =
        DoClustering2ByKMeans(config, vs, state.cluster_to_vs[c0].size(),
                              state.cluster_to_vs[c1].size());
    const double new_w = k_means_result.score + EdgesWeight(config, mst_2);
    const double score_delta = new_w - base_w;
    if(AcceptAnnealing(-score_delta, temperature)) {
        SetClusterToState(config, c0, k_means_result.vs_0, k_means_result.mst_0,
                          state);
        SetClusterToState(config, c1, k_means_result.vs_1, k_means_result.mst_1,
                          state);
        SetClusterToState(config, c2, vs_2, mst_2, state);
        return true;
    }

    return false;
}

void ExtractMstEdges2(const std::vector<int> &component,
                      const std::vector<Edge> &mst_edges,
                      std::vector<Edge> &component_mst_edges) {
    for(const auto &edge : mst_edges) {
        if(find(component.begin(), component.end(), edge.s) != component.end() &&
           find(component.begin(), component.end(), edge.t) != component.end())
            component_mst_edges.push_back(edge);
    }
}

// target_clusters: The target clusters to compute in this multi-clustering.
// cluster_indexes: The indexes of the clusters to handle in this specific
// call. The index points to a element in target_clusters.
void DoMultiClustering(
    const ProblemInput &input,
    const ClusteringIterationConfig &config,
    const std::vector<int> &target_clusters,
    const std::vector<int> &vs,
    const std::vector<int> &cluster_indexes,
    std::vector<std::vector<int>> &cluster_index_to_vs,
    std::vector<std::vector<Edge>> &cluster_index_to_mst) {
    if(cluster_indexes.size() == 1) {
        const int cluster_index = cluster_indexes[0];
        const std::vector<Edge> mst = FindMST(vs, config.distance_matrix);
        cluster_index_to_vs[cluster_index] = vs;
        cluster_index_to_mst[cluster_index] = mst;
        return;
    }

    if(cluster_indexes.size() == 2) {
        const int size_0 = input.cluster_sizes[target_clusters[cluster_indexes[0]]];
        const int size_1 = input.cluster_sizes[target_clusters[cluster_indexes[1]]];
        std::optional<Clustering2Result> opt_merged_mst_result =
            MaybeDoClustering2ByMergedMST(config, vs, size_0, size_1);
        if(opt_merged_mst_result) {
            cluster_index_to_vs[cluster_indexes[0]] = opt_merged_mst_result->vs_0;
            cluster_index_to_vs[cluster_indexes[1]] = opt_merged_mst_result->vs_1;
            cluster_index_to_mst[cluster_indexes[0]] = opt_merged_mst_result->mst_0;
            cluster_index_to_mst[cluster_indexes[1]] = opt_merged_mst_result->mst_1;
            return;
        }
        // if (vs.size() != size_0 + size_1) {
        //   std::cerr << "vs.size(): " << vs.size() << std::endl;
        //   std::cerr << "size_0: " << size_0 << std::endl;
        //   std::cerr << "size_1: " << size_1 << std::endl;
        //   UNEXPECTED();
        // }
        Clustering2Result k_means_result =
            DoClustering2ByKMeans(config, vs, size_0, size_1);
        cluster_index_to_vs[cluster_indexes[0]] = k_means_result.vs_0;
        cluster_index_to_vs[cluster_indexes[1]] = k_means_result.vs_1;
        cluster_index_to_mst[cluster_indexes[0]] = k_means_result.mst_0;
        cluster_index_to_mst[cluster_indexes[1]] = k_means_result.mst_1;
        return;
    }

    // More than 3 clusters.
    // std::vector<int> cluster_ids;
    // for (const int cluster_index : cluster_indexes) {
    //   cluster_ids.push_back(target_clusters[cluster_index]);
    // }

    std::vector<int> current_target_sizes;
    for(const int cluster_index : cluster_indexes) {
        current_target_sizes.push_back(input.cluster_sizes[target_clusters[cluster_index]]);
    }
    // std::cerr << "vs.size(): " << vs.size() << std::endl;
    // std::cerr << "current_target_sizes: " << current_target_sizes << std::endl;

    int total = std::accumulate(current_target_sizes.begin(),
                                current_target_sizes.end(), 0);

    SubsetSumSolver subset_sum_solver;
    subset_sum_solver.Solve(current_target_sizes, int(vs.size()));

    // TODO: Add Merged MST approach here.
    {
        const auto mst_edges = FindMST(vs, config.distance_matrix);
        std::vector<Edge> undirected_mst_edges;
        for(const auto &edge : mst_edges) {
            undirected_mst_edges.push_back(Edge(edge.s, edge.t));
            undirected_mst_edges.push_back(Edge(edge.t, edge.s));
        }
        Graph graph(kVertexCount, undirected_mst_edges);

        std::vector<int> parents(kVertexCount, -1);
        std::vector<int> sizes(kVertexCount, -1);

        auto dfs = [&](auto &&dfs, const int v, const int p) -> void {
            sizes[v] = 1;
            for(const Edge &edge : graph.OutEdges(v)) {
                if(edge.t == p)
                    continue;
                parents[edge.t] = v;
                dfs(dfs, edge.t, v);
                sizes[v] += sizes[edge.t];
            }
        };
        dfs(dfs, vs[0], -1);
        // Sort MST edges in descending order of weight (using squared Euclidean
        // distance).
        std::vector<Edge> sorted_edges = mst_edges;
        auto edge_weight = [&](const Edge &e) -> double {
            return config.distance_matrix.Get(e.s, e.t);
        };
        sort(sorted_edges.begin(), sorted_edges.end(),
             [&](const Edge &a, const Edge &b) {
                 return edge_weight(a) > edge_weight(b);
             });

        for(const auto &target_edge : sorted_edges) {
            int v0 = target_edge.s;
            int v1 = target_edge.t;
            if(parents[v1] == v0) {
                // Do nothing.
            } else if(parents[v0] == v1) {
                std::swap(v0, v1);
            } else {
                UNEXPECTED();
            }

            const int size_0 = int(vs.size()) - sizes[v1];
            const int size_1 = sizes[v1];
            if(!subset_sum_solver.SubsetExists(size_0)) {
                continue;
            }

            std::vector<int> cluster_local_indexes_0 =
                subset_sum_solver.GetSubset(size_0);

            // Partition cluster_ids into two groups: one for comp0 and one for
            // comp1.
            std::vector<int> in_cluster_local_indexes_0(target_clusters.size(), 0);
            for(const int cluster_local_index : cluster_local_indexes_0) {
                in_cluster_local_indexes_0[cluster_local_index] = 1;
            }
            std::vector<int> cluster_local_indexes_1;
            for(const int cluster_local_index :
                std::views::iota(0, int(cluster_indexes.size()))) {
                if(!in_cluster_local_indexes_0[cluster_local_index]) {
                    cluster_local_indexes_1.push_back(cluster_local_index);
                }
            }

            // Get vertices in each component using DFS
            std::vector<int> comp0, comp1;
            auto get_subtree_vertices =
                [&](auto &&get_subtree_vertices, const int v, const int p,
                    std::vector<int> &subtree_vertices) -> void {
                subtree_vertices.push_back(v);
                for(const auto &edge : graph.OutEdges(v)) {
                    if(edge.t != p) {
                        get_subtree_vertices(get_subtree_vertices, edge.t, v,
                                             subtree_vertices);
                    }
                }
            };
            get_subtree_vertices(get_subtree_vertices, v1, v0, comp1);

            // Get component 0 vertices as complement of component 1
            std::vector<int> in_comp1(kVertexCount);
            for(const int v : comp1) {
                in_comp1[v] = 1;
            }
            for(const int v : vs) {
                if(!in_comp1[v]) {
                    comp0.push_back(v);
                }
            }

            std::vector<int> cluster_indexes_0;
            for(const int cluster_local_index : cluster_local_indexes_0) {
                cluster_indexes_0.push_back(cluster_indexes[cluster_local_index]);
            }
            std::vector<int> cluster_indexes_1;
            for(const int cluster_local_index : cluster_local_indexes_1) {
                cluster_indexes_1.push_back(cluster_indexes[cluster_local_index]);
            }

            DoMultiClustering(input, config, target_clusters, comp0,
                              cluster_indexes_0, cluster_index_to_vs,
                              cluster_index_to_mst);
            DoMultiClustering(input, config, target_clusters, comp1,
                              cluster_indexes_1, cluster_index_to_vs,
                              cluster_index_to_mst);
            return;
        }
    }

    int target_size = 0;
    for(int size = int(vs.size()) / 2; size >= 1; size--) {
        if(subset_sum_solver.SubsetExists(size)) {
            target_size = size;
            break;
        }
    }
    if(target_size == 0) {
        UNEXPECTED();
    }

    std::vector<int> cluster_local_indexes_0 =
        subset_sum_solver.GetSubset(target_size);

    // Partition cluster_ids into two groups: one for comp0 and one for comp1.
    std::vector<int> in_cluster_local_indexes_0(target_clusters.size(), 0);
    for(const int cluster_local_index : cluster_local_indexes_0) {
        in_cluster_local_indexes_0[cluster_local_index] = 1;
    }
    std::vector<int> cluster_local_indexes_1;
    for(const int cluster_local_index :
        std::views::iota(0, int(cluster_indexes.size()))) {
        if(!in_cluster_local_indexes_0[cluster_local_index]) {
            cluster_local_indexes_1.push_back(cluster_local_index);
        }
    }

    const double theta = g_random.RandomDouble(0.0, 2.0 * M_PI);
    std::vector<int> sorted_vertices = vs;
    std::sort(sorted_vertices.begin(), sorted_vertices.end(),
              [&](int v0, int v1) {
                  const double z_0 = config.points[v0].x * std::cos(theta) +
                                     config.points[v0].y * std::sin(theta);
                  const double z_1 = config.points[v1].x * std::cos(theta) +
                                     config.points[v1].y * std::sin(theta);
                  return z_0 < z_1;
              });

    std::vector<int> vs_0(sorted_vertices.begin(),
                          sorted_vertices.begin() + target_size);
    std::vector<int> vs_1(sorted_vertices.begin() + target_size,
                          sorted_vertices.end());

    // Do K-means.
    for(const int iter : std::views::iota(0, 10)) {
        // Calculate the mean (kernel) of vs_0 and vs_1
        Double2D kernel_0 = {0.0, 0.0};
        Double2D kernel_1 = {0.0, 0.0};

        // Calculate kernel_0 (mean of vs_0)
        for(const int v : vs_0) {
            kernel_0.x += config.points[v].x;
            kernel_0.y += config.points[v].y;
        }
        if(!vs_0.empty()) {
            kernel_0.x /= vs_0.size();
            kernel_0.y /= vs_0.size();
        }

        // Calculate kernel_1 (mean of vs_1)
        for(const int v : vs_1) {
            kernel_1.x += config.points[v].x;
            kernel_1.y += config.points[v].y;
        }
        if(!vs_1.empty()) {
            kernel_1.x /= vs_1.size();
            kernel_1.y /= vs_1.size();
        }

        // Sort vertices by the difference in distance to the two kernels
        std::vector<std::pair<double, int>> distance_diffs;
        for(const int v : sorted_vertices) {
            double dist_to_kernel_0 = Distance2(config.points[v], kernel_0);
            double dist_to_kernel_1 = Distance2(config.points[v], kernel_1);
            double diff = dist_to_kernel_0 - dist_to_kernel_1;
            distance_diffs.push_back({diff, v});
        }

        // Sort by difference in distance (vertices closer to kernel_0 will come
        // first)
        std::sort(distance_diffs.begin(), distance_diffs.end());

        // Update vs_0 and vs_1 based on the sorted differences
        vs_0.clear();
        vs_1.clear();

        // Take the first target_size vertices for vs_0
        for(int i = 0; i < target_size; ++i) {
            vs_0.push_back(distance_diffs[i].second);
        }

        // Take the remaining vertices for vs_1
        for(size_t i = target_size; i < distance_diffs.size(); ++i) {
            vs_1.push_back(distance_diffs[i].second);
        }
    }

    std::vector<int> cluster_indexes_0;
    for(const int cluster_local_index : cluster_local_indexes_0) {
        cluster_indexes_0.push_back(cluster_indexes[cluster_local_index]);
    }
    std::vector<int> cluster_indexes_1;
    for(const int cluster_local_index : cluster_local_indexes_1) {
        cluster_indexes_1.push_back(cluster_indexes[cluster_local_index]);
    }

    DoMultiClustering(input, config, target_clusters, vs_0, cluster_indexes_0,
                      cluster_index_to_vs, cluster_index_to_mst);
    DoMultiClustering(input, config, target_clusters, vs_1, cluster_indexes_1,
                      cluster_index_to_vs, cluster_index_to_mst);
}

bool TryUpdateByMultiClustering(const ProblemInput &input,
                                const ClusteringIterationConfig &config,
                                const std::vector<int> &target_clusters,
                                const std::vector<int> &vs,
                                ClusteringIterationState &state) {
    double base_w = 0.0;
    for(const int cluster_id : target_clusters) {
        base_w += state.cluster_to_mst_weight_sum[cluster_id];
    }

    std::vector<int> cluster_indexes;
    for(const int index : std::views::iota(0, int(target_clusters.size()))) {
        cluster_indexes.push_back(index);
    }

    std::vector<std::vector<int>> cluster_index_to_vs(target_clusters.size());
    std::vector<std::vector<Edge>> cluster_index_to_mst(target_clusters.size());
    DoMultiClustering(input, config, target_clusters, vs, cluster_indexes,
                      cluster_index_to_vs, cluster_index_to_mst);

    double new_w = 0.0;
    for(const int cluster_index : std::views::iota(0, int(target_clusters.size()))) {
        new_w += EdgesWeight(config, cluster_index_to_mst[cluster_index]);
    }
    const double score_delta = new_w - base_w;
    if(AcceptAnnealing(-score_delta, 0.0)) {
        for(const int cluster_index :
            std::views::iota(0, int(target_clusters.size()))) {
            if(state.cluster_to_mst[target_clusters[cluster_index]].size() !=
               cluster_index_to_mst[cluster_index].size()) {
                std::cerr << "state.cluster_to_mst[target_clusters[cluster_index]].size(): " << state.cluster_to_mst[target_clusters[cluster_index]].size() << std::endl;
                std::cerr << "cluster_index_to_mst[cluster_index].size(): " << cluster_index_to_mst[cluster_index].size() << std::endl;
                UNEXPECTED();
            }
            const int cluster_id = target_clusters[cluster_index];
            std::vector<int> vs = cluster_index_to_vs[cluster_index];
            std::vector<Edge> mst = cluster_index_to_mst[cluster_index];
            SetClusterToState(config, cluster_id, vs, mst, state);
        }
        return true;
    }
    return false;
}

void RunClusteringIterationPhase(
    const ProblemInput &input, const std::array<Double2D, kVertexCount> &points,
    const DoubleMatrix &distance_matrix, std::vector<int> &assignment,
    std::vector<std::vector<Edge>> &cluster_to_mst) {
    std::vector<std::pair<int, int>> close_pairs = GetClosePairs(distance_matrix);
    ClusteringIterationConfig config(points, distance_matrix);

    // Initalize state.
    ClusteringIterationState state(input.cluster_count);
    std::vector<std::vector<int>> cluster_to_vs(input.cluster_count);
    for(const int v : std::views::iota(0, kVertexCount)) {
        cluster_to_vs[assignment[v]].push_back(v);
    }
    for(const int cluster_id : std::views::iota(0, input.cluster_count)) {
        SetClusterToState(config, cluster_id, cluster_to_vs[cluster_id],
                          FindMST(cluster_to_vs[cluster_id], distance_matrix),
                          state);
    }

    std::cerr << "initial_score: " << state.score << std::endl;

    // First, do hill climbing by vertex swaps.
    while(g_time.CurrentMs() < kClusteringIterationTimeLimit) {
        bool updated = false;
        for(const auto [v0, v1] : close_pairs) {
            const int c0 = state.assignment[v0];
            const int c1 = state.assignment[v1];
            if(c0 == c1) {
                continue;
            }
            if(TryVertexSwap(config, 0.0, state, v0, c0, v1, c1)) {
                updated = true;
                if(g_time.CurrentMs() > kClusteringIterationTimeLimit) {
                    break;
                }
            }
        }
        if(!updated) {
            break;
        }
    }

    std::cerr << "hill_climbing_score: " << state.score << std::endl;

    const int time_0 = g_time.CurrentMs();
    constexpr double kTemperature0 = 100.0;
    constexpr double kTemperature1 = 10.0;

    ClusteringIterationState best_state = state;
    double best_score = state.score;

    int annealing_transition_count = 0;
    int annealing_update_best_count = 0;
    auto update_best_state = [&]() {
        annealing_transition_count++;
        if(state.score < best_score) {
            best_score = state.score;
            best_state = state;
            annealing_update_best_count++;
        }
    };

    // Preparation for annealing.
    std::vector<std::vector<int>> cluster_size_to_cluster_ids(kVertexCount + 1);
    for(const int cluster_id : std::views::iota(0, input.cluster_count)) {
        cluster_size_to_cluster_ids[input.cluster_sizes[cluster_id]].push_back(
            cluster_id);
    }

    int swap_transition_count = 0;
    int merged_mst_transition_count = 0;
    int kmeans_transition_count = 0;
    int swap_one_two_cluster_transition_count = 0;
    int multi_update_2_count = 0;
    int multi_update_3_count = 0;
    int multi_update_4plus_count = 0;
    int try_update_count = 0;
    int try_update_true_count = 0;

    // The main function for annealing. Returns true if expensive operation is
    // performed.
    auto try_update = [&](int64 time_ms) -> bool {
        try_update_count++;
        const auto [v0, v1] =
            close_pairs[g_random.RandomInt(0, close_pairs.size())];
        const int c0 = state.assignment[v0];
        const int c1 = state.assignment[v1];
        if(c0 == c1) {
            return false;
        }

        const double temperature =
            ExpInterpolation(time_0, kTemperature0, kClusteringIterationTimeLimit,
                             kTemperature1, time_ms);

        if(TryVertexSwap(config, temperature, state, v0, c0, v1, c1)) {
            update_best_state();
            swap_transition_count++;
            try_update_true_count++;
            return true;
        }

        const int size_0 = input.cluster_sizes[c0];
        const int size_1 = input.cluster_sizes[c1];
        if(cluster_size_to_cluster_ids[size_0 + size_1].size() >= 1 &&
           g_random.RandomDouble(0.0, 1.0) < 0.5) {
            const std::vector<int> &c2_candidates =
                cluster_size_to_cluster_ids[size_0 + size_1];
            const int c2 = c2_candidates[g_random.RandomInt(0, c2_candidates.size())];
            const double d = config.distance_matrix.Get(v0, v1);
            if(d < state.cluster_to_mst_weight_max[c2] + temperature) {
                if(TryUpdateByOneTwoClusterSwap(config, temperature, c0, c1, c2,
                                                state)) {
                    update_best_state();
                    swap_one_two_cluster_transition_count++;
                    try_update_true_count++;
                    return true;
                }
                return true;
            }
        }

        // Do quick check to see if merging two clusters is promising. If not,
        // return without executing the expensive operation.
        if(const double d = config.distance_matrix.Get(v0, v1);
           state.cluster_to_mst_weight_max[c0] + temperature < d &&
           state.cluster_to_mst_weight_max[c1] + temperature < d) {
            return false;
        }

        const double r = g_random.RandomDouble(0.0, 1.0);
        if(r < 1.0 / 3.0) {
            if(TryUpdateTwoClustersFromMergedMST(config, temperature, c0, c1,
                                                 state)) {
                update_best_state();
                merged_mst_transition_count++;
            }
        } else {
            if(TryUpdateTwoClustersFromKMeans(config, temperature, c0, c1, state)) {
                update_best_state();
                kmeans_transition_count++;
            }
        }
        try_update_true_count++;
        return true;
    };

    auto try_multi_updte = [&]() -> void {
        const double center_x = g_random.RandomDouble(0.0, 10000.0);
        const double center_y = g_random.RandomDouble(0.0, 10000.0);
        const double radius = g_random.RandomDouble(500.0, 1000.0);
        const double min_x = center_x - radius;
        const double max_x = center_x + radius;
        const double min_y = center_y - radius;
        const double max_y = center_y + radius;
        std::vector<int> clusters;
        for(const int v : std::views::iota(0, kVertexCount)) {
            const double x = config.points[v].x;
            const double y = config.points[v].y;
            if(x >= min_x && x <= max_x && y >= min_y && y <= max_y) {
                clusters.push_back(state.assignment[v]);
            }
        }
        // Sort clusters and remove duplicates
        std::sort(clusters.begin(), clusters.end());
        clusters.erase(std::unique(clusters.begin(), clusters.end()), clusters.end());

        // Skip if we don't have enough clusters to work with
        if(clusters.size() < 3) {
            return;
        }

        // Collect vertices within the radius of the random center
        std::vector<int> vs;
        for(const int cluster_id : clusters) {
            for(const int v : state.cluster_to_vs[cluster_id]) {
                vs.push_back(v);
            }
        }

        if(TryUpdateByMultiClustering(input, config, clusters, vs, state)) {
            if(clusters.size() == 2) {
                multi_update_2_count++;
            } else if(clusters.size() == 3) {
                multi_update_3_count++;
            } else {
                multi_update_4plus_count++;
            }
        }
    };

    // Do annealing with more complex transitions.
    while(true) {
        const int64 time_ms = g_time.CurrentMs();
        if(time_ms > kClusteringIterationTimeLimit) {
            break;
        }

        if(g_random.RandomDouble(0.0, 1.0) < 0.1) {
            try_multi_updte();
            continue;
        }

        constexpr int kLoopCount = 100;
        for(const int i : std::views::iota(0, kLoopCount)) {
            if(try_update(time_ms)) {
                break;
            }
        }
    }

    std::cerr << "iteration_final_score: " << best_score << std::endl;
    std::cerr << std::endl;
    std::cerr << "annealing_transition_count: " << annealing_transition_count
              << std::endl;
    std::cerr << "annealing_update_best_count: " << annealing_update_best_count
              << std::endl;
    std::cerr << std::endl;
    std::cerr << "try_update_count: " << try_update_count << std::endl;
    std::cerr << "try_update_true_count: " << try_update_true_count << std::endl;
    std::cerr << "swap_transition_count: " << swap_transition_count << std::endl;
    std::cerr << "merged_mst_transition_count: " << merged_mst_transition_count
              << std::endl;
    std::cerr << "kmeans_transition_count: " << kmeans_transition_count
              << std::endl;
    std::cerr << "swap_one_two_cluster_transition_count: "
              << swap_one_two_cluster_transition_count << std::endl;
    std::cerr << "multi_update_2_count: " << multi_update_2_count << std::endl;
    std::cerr << "multi_update_3_count: " << multi_update_3_count << std::endl;
    std::cerr << "multi_update_4plus_count: " << multi_update_4plus_count
              << std::endl;
    std::cerr << std::endl;

    assignment = best_state.assignment;
    cluster_to_mst = best_state.cluster_to_mst;
}

#endif // CLUSTERING_ITERATION_CPP

#ifndef FINALIZER_CPP_
#define FINALIZER_CPP_

std::vector<MSTCluster>
FinalizeSolution(const ProblemInput &input, const std::vector<int> &assignment,
                 const std::vector<std::vector<Edge>> &cluster_to_mst) {
    std::vector<MSTCluster> clusters;
    for(const int cluster_id : std::views::iota(0, input.cluster_count)) {
        std::vector<int> vs;
        for(const int v : std::views::iota(0, kVertexCount)) {
            if(assignment[v] == cluster_id) {
                vs.push_back(v);
            }
        }

        std::vector<Edge> mst_edges = cluster_to_mst[cluster_id];
        clusters.emplace_back(vs, mst_edges);
    }
    return clusters;
}

#endif // FINALIZER_CPP_

#ifndef QUERY_PHASE_CPP_
#define QUERY_PHASE_CPP_

#include <set>

#ifndef QUERY_SELECTOR_CPP_
#define QUERY_SELECTOR_CPP_

double ScoreQuery(const int vertex_count, const std::vector<int> &vs,
                  const int sample_count,
                  const std::vector<DoubleMatrix> &distance_matrices,
                  const std::vector<double> &radiuses) {
    DoubleMatrix local_distance_matrix(vs.size(), vs.size());
    DoubleMatrix count_matrix(vs.size(), vs.size());
    std::vector<Edge> mst;
    mst.reserve(vs.size() - 1);
    for(const int sample_id : std::views::iota(0, sample_count)) {
        for(const int i : std::views::iota(0, int(vs.size()))) {
            for(const int j : std::views::iota(i + 1, int(vs.size()))) {
                const double d = distance_matrices[sample_id].Get(vs[i], vs[j]);
                local_distance_matrix.Set(i, j, d);
                local_distance_matrix.Set(j, i, d);
            }
        }
        mst.clear();
        FindMSTOptimized<kMaxQuerySize>(vs.size(), local_distance_matrix, mst);
        // mst = FindMST(vs.size(), local_distance_matrix);
        for(const auto &edge : mst) {
            count_matrix.Increment(edge.s, edge.t, 1);
            count_matrix.Increment(edge.t, edge.s, 1);
        }
    }

    double score = 0.0;
    for(const int i : std::views::iota(0, int(vs.size()))) {
        for(const int j : std::views::iota(i + 1, int(vs.size()))) {
            const double count = count_matrix.Get(i, j);
            // std::cerr << "i: " << i << ", j: " << j << ", count: " << count
            //           << std::endl;
            if(0.0 < count && count < sample_count) {
                const double p = count / sample_count;
                double information = -p * std::log(p) - (1.0 - p) * std::log(1.0 - p);
                score += information * (radiuses[vs[i]] + radiuses[vs[j]]);
            }
        }
    }
    // std::cerr << "score: " << score << std::endl;
    return score;
}

std::vector<int> SelectQueryFromCandidates(
    const std::array<Double2D, kVertexCount> &points,
    const std::vector<Double2DRange> &ranges,
    const std::array<int, kVertexCount> &v_to_reserved,
    const std::vector<int> &candidates, const int query_size) {
    constexpr double kBudget = 200000.0;
    const int sample_count =
        std::floor(kBudget / (std::max(query_size * query_size, 16) * query_size *
                              (candidates.size() - query_size)));
    // std::cerr << "sample_count: " << sample_count << std::endl;
    std::vector<DoubleMatrix> distance_matrices(
        sample_count, DoubleMatrix(candidates.size(), candidates.size()));
    for(const int sample_id : std::views::iota(0, sample_count)) {
        std::vector<Double2D> sample_points(candidates.size());
        for(const int i : std::views::iota(0, int(candidates.size()))) {
            const int v = candidates[i];
            const double r = (ranges[v].x_max - ranges[v].x_min) * 0.25 +
                             (ranges[v].y_max - ranges[v].y_min) * 0.25;
            const double sd = r * 0.5;
            const double u = g_random.RandomDouble(0.1, 1.0);
            const double magnitude = std::sqrt(-2.0 * std::log(u)) * sd;
            const double theta = g_random.RandomDouble(0.0, 2.0 * M_PI);
            sample_points[i].x = points[v].x + magnitude * std::cos(theta);
            sample_points[i].y = points[v].y + magnitude * std::sin(theta);
        }
        for(const int i : std::views::iota(0, int(candidates.size()))) {
            for(const int j : std::views::iota(0, i)) {
                const double d = Distance(sample_points[i], sample_points[j]);
                distance_matrices[sample_id].Set(i, j, d);
                distance_matrices[sample_id].Set(j, i, d);
            }
        }
    }
    std::vector<double> radiuses(candidates.size());
    for(const int i : std::views::iota(0, int(candidates.size()))) {
        const int v = candidates[i];
        radiuses[i] = (ranges[v].x_max - ranges[v].x_min) * 0.25 +
                      (ranges[v].y_max - ranges[v].y_min) * 0.25;
    }

    std::vector<int> current_indices;
    for(const int i : std::views::iota(0, query_size)) {
        current_indices.push_back(i);
    }
    double current_score = ScoreQuery(candidates.size(), current_indices,
                                      sample_count, distance_matrices, radiuses);

    if(query_size == 3) {
        for(const int i1 : std::views::iota(1, int(candidates.size()))) {
            for(const int i2 : std::views::iota(i1 + 1, int(candidates.size()))) {
                std::vector<int> indices = {0, i1, i2};
                const double score = ScoreQuery(candidates.size(), indices,
                                                sample_count, distance_matrices,
                                                radiuses);
                if(score > current_score) {
                    current_indices = indices;
                    current_score = score;
                }
            }
        }
    } else {
        for(const int target_index :
            std::views::iota(query_size, int(candidates.size()))) {
            int max_index = -1;
            double max_score = current_score;
            for(const int i : std::views::iota(1, query_size)) {
                std::vector<int> indices = current_indices;
                indices[i] = target_index;
                const double score =
                    ScoreQuery(candidates.size(), indices, sample_count,
                               distance_matrices, radiuses);
                if(score > max_score) {
                    max_score = score;
                    max_index = i;
                    // break;
                }
            }
            if(max_index >= 0) {
                current_indices[max_index] = target_index;
                current_score = max_score;
            }
        }
    }

    std::vector<int> max_vs(query_size);
    for(const int i : std::views::iota(0, query_size)) {
        max_vs[i] = candidates[current_indices[i]];
    }
    // std::cerr << "indices: " << current_indices << std::endl;
    // std::cerr << "query_score: " << current_score << std::endl;
    return max_vs;
}

// Select a query based on the root vertex (v0).
std::vector<int> SelectQuery(const std::array<Double2D, kVertexCount> &points,
                             const std::vector<Double2DRange> &ranges,
                             const std::array<int, kVertexCount> &v_to_reserved,
                             const int v0, const int query_size) {
    const double v_r = (ranges[v0].x_max - ranges[v0].x_min) * 0.25 +
                       (ranges[v0].y_max - ranges[v0].y_min) * 0.25;

    std::vector<std::pair<double, int>> scored_vs;
    scored_vs.reserve(kVertexCount);
    for(const int u : std::views::iota(0, kVertexCount)) {
        if(v_to_reserved[u]) {
            continue;
        }
        const double distance = Distance2(points[v0], points[u]);
        const double u_r = (ranges[u].x_max - ranges[u].x_min) * 0.25 +
                           (ranges[u].y_max - ranges[u].y_min) * 0.25;
        const double adjuster = v_r * v_r + u_r * u_r + 10000.0;
        const double score = distance / adjuster;
        scored_vs.emplace_back(score, u);
    }
    std::sort(scored_vs.begin(), scored_vs.end());
    std::vector<int> result;
    result.reserve(query_size);

    if(g_time.CurrentMs() > kQueryPhaseQuickModeTime) {
        for(const int i : std::views::iota(0, query_size)) {
            result.push_back(scored_vs[i].second);
        }
        return result;
    }

    std::vector<int> candidates;
    const int candidate_count = std::max(20, query_size * 2);
    for(const int i : std::views::iota(0, candidate_count)) {
        candidates.push_back(scored_vs[i].second);
    }

    // static int count = 0;
    // static int64 t_sum = 0;
    // int64 t0 = g_time.CurrentUs();
    result = SelectQueryFromCandidates(points, ranges, v_to_reserved, candidates,
                                       query_size);
    // int64 t1 = g_time.CurrentUs();
    // t_sum += t1 - t0;
    // count++;
    // std::cerr << "count: " << count << ", t_sum: " << t_sum << std::endl;

    return result;
}

#endif // QUERY_SELECTOR_CPP_

constexpr double kQueryPhaseTimeLimit = 1500.0;

// For local testing.
std::vector<Edge> MakeMSTFromActualPoints(const std::vector<int> &vs) {
    MergeFindSet mfs(kVertexCount);
    std::vector<WEdge> edges;
    edges.reserve(int(vs.size()) * (int(vs.size()) - 1) / 2);
    for(const int i : std::views::iota(0, int(vs.size()))) {
        for(const int j : std::views::iota(i + 1, int(vs.size()))) {
            edges.emplace_back(
                vs[i], vs[j],
                Distance2(g_actual_points[vs[i]], g_actual_points[vs[j]]));
        }
    }
    std::sort(edges.begin(), edges.end(),
              [](const WEdge &e0, const WEdge &e1) { return e0.w < e1.w; });

    std::vector<Edge> result;
    for(const auto &edge : edges) {
        if(mfs.Find(edge.s) != mfs.Find(edge.t)) {
            mfs.Merge(edge.s, edge.t);
            result.emplace_back(edge.s, edge.t);
            if(result.size() == int(vs.size()) - 1) {
                break;
            }
        }
    }
    return result;
}

std::vector<Edge> QueryMST(const std::vector<int> &vs) {
    if(g_is_local_mode) {
        return MakeMSTFromActualPoints(vs);
    }

    std::cout << "?";
    std::cout << " " << vs.size();
    for(const int v : vs) {
        std::cout << " " << v;
    }
    std::cout << std::endl;
    std::cout.flush();

    std::vector<Edge> result;
    for(const int i : std::views::iota(0, int(vs.size()) - 1)) {
        int s, t;
        std::cin >> s >> t;
        result.emplace_back(s, t);
    }
    return result;
}

void AddDistanceConstraints(const std::vector<int> &vs,
                            const std::vector<Edge> &mst_edges,
                            const std::vector<MCMCSample> &reference_samples,
                            ConstraintSet &distance_contraint_index) {
    static std::array<std::vector<Edge>, kVertexCount> v_to_edges;
    for(const int v : vs) {
        v_to_edges[v].clear();
    }

    for(const auto &edge : mst_edges) {
        v_to_edges[edge.s].push_back(edge);
        v_to_edges[edge.t].push_back(Edge(edge.t, edge.s));
    }

    static std::vector<Edge> current_path;
    auto dfs = [&](auto &&dfs, const int v, const int p) -> void {
        for(const auto &edge : v_to_edges[v]) {
            if(edge.t == p)
                continue;
            current_path.push_back(edge);
            if(current_path.size() >= 2) {
                const int i0 = current_path.front().s;
                const int i1 = current_path.back().t;
                if(i0 < i1) {
                    for(const auto &edge : current_path) {
                        auto [j0, j1] = edge;
                        if(j0 > j1)
                            std::swap(j0, j1);

                        bool required = false;
                        for(const auto &sample : reference_samples) {
                            const double d2i =
                                Distance2(sample.points[i0], sample.points[i1]);
                            const double d2j =
                                Distance2(sample.points[j0], sample.points[j1]);
                            // d(i0, i1) / sqrt(2) < d(j0, j1)
                            if(d2i * 0.5 < d2j) {
                                required = true;
                                break;
                            }
                        }

                        if(!required) {
                            continue;
                        }

                        const DistanceConstraint distance_constraint(i0, i1, j0, j1);
                        distance_contraint_index.AddDistanceConstraint(distance_constraint);
                    }
                }
            }
            dfs(dfs, edge.t, v);
            current_path.pop_back();
        }
    };
    for(const int v : vs) {
        dfs(dfs, v, -1);
    }
}

std::vector<int> FindNeighbors(const std::vector<int> &query,
                               std::array<Double2D, kVertexCount> &points) {
    // First, calculate the average point from the query vertices
    Double2D average_point = {0.0, 0.0};
    for(const int v : query) {
        average_point.x += points[v].x;
        average_point.y += points[v].y;
    }
    average_point.x /= query.size();
    average_point.y /= query.size();

    // Create a vector of pairs (distance to average, vertex_id)
    std::vector<std::pair<double, int>> distance_vertex_pairs;
    for(int v = 0; v < kVertexCount; v++) {
        // Skip vertices that are already in the query
        if(std::find(query.begin(), query.end(), v) != query.end()) {
            continue;
        }

        // Calculate distance from this vertex to the average point
        double dist = Distance(points[v], average_point);
        distance_vertex_pairs.push_back({dist, v});
    }

    // Sort by distance (ascending)
    std::sort(distance_vertex_pairs.begin(), distance_vertex_pairs.end());

    // Take the 50 closest vertices (or fewer if there aren't 50 available)
    std::vector<int> neighbors = query;
    constexpr int kNeighborSize = 50;
    for(const auto &[distance, v] : distance_vertex_pairs) {
        neighbors.push_back(v);
        if(neighbors.size() >= kNeighborSize) {
            break;
        }
    }

    return neighbors;
}

double GetRadiusAverage(const std::vector<Double2DRange> &ranges) {
    double r_sum = 0.0;
    for(const auto &range : ranges) {
        r_sum +=
            (range.x_max - range.x_min) * 0.25 + (range.y_max - range.y_min) * 0.25;
    }
    return r_sum / kVertexCount;
}

void RunSingleQueryStep(const ProblemInput &input, const int query_index,
                        const int v,
                        const std::array<int, kVertexCount> &v_to_reserved,
                        std::array<Double2D, kVertexCount> &points,
                        const std::vector<MCMCSample> &reference_samples,
                        ConstraintSet &distance_contraint_index,
                        std::vector<Double2DRange> &ranges) {
    const std::vector<int> query =
        SelectQuery(points, ranges, v_to_reserved, v, input.max_query_size);

    std::vector<Edge> mst_edges = QueryMST(query);

    AddDistanceConstraints(query, mst_edges, reference_samples,
                           distance_contraint_index);

    const double radius_average = GetRadiusAverage(ranges);
    // std::cerr << "radius_average: " << radius_average << std::endl;

    IndexSet updated_vertex_set(kVertexCount);
    IndexSet added_constraint_index_set(
        distance_contraint_index.distance_constraints.size());
    IndexSet merged_updated_vertex_set(kVertexCount);
    for(const int v : query) {
        updated_vertex_set.Add(v);
        merged_updated_vertex_set.Add(v);
    }

    double alpha = radius_average * 3.0;
    int iteration_count = 30;
    if(g_time.CurrentMs() > kQueryPhaseQuickModeTime) {
        alpha = radius_average * 1.5;
        iteration_count = 15;
    }
    for(const int i : std::views::iota(0, iteration_count)) {
        if(RunGradientDescent(distance_contraint_index, alpha, points,
                              updated_vertex_set, added_constraint_index_set)) {
            alpha *= 0.9;
            for(const int v : updated_vertex_set.Values()) {
                merged_updated_vertex_set.Add(v);
            }
        } else {
            break;
        }
    }

    std::vector<int> vs;
    if(g_time.CurrentMs() < kQueryPhaseQuickModeTime || query_index % 10 == 0) {
        for(const int v : std::views::iota(0, kVertexCount)) {
            vs.push_back(v);
        }
    } else {
        for(const int v : merged_updated_vertex_set.Values()) {
            vs.push_back(v);
        }
    }

    RunGibbsSampling(vs, distance_contraint_index, points, ranges, true);
}

void RunQueryPhase(const ProblemInput &input,
                   const std::vector<int> &reserved_isolated_vertices,
                   std::array<Double2D, kVertexCount> &points,
                   ConstraintSet &distance_contraint_index) {
    std::array<int, kVertexCount> v_to_reserved{};
    for(const int v : reserved_isolated_vertices) {
        v_to_reserved[v] = 1;
    }

    std::vector<std::pair<double, int>> scored_vertices;
    scored_vertices.reserve(kVertexCount);
    for(const int i : std::views::iota(0, kVertexCount)) {
        if(v_to_reserved[i]) {
            continue;
        }
        const double score = (input.x_maxs[i] - input.x_mins[i]) +
                             (input.y_maxs[i] - input.y_mins[i]);
        scored_vertices.emplace_back(score, i);
    }
    std::sort(scored_vertices.begin(), scored_vertices.end(), std::greater<>());

    std::vector<Double2DRange> ranges(kVertexCount);
    for(const int v : std::views::iota(0, kVertexCount)) {
        ranges[v] = Double2DRange(input.x_mins[v], input.x_maxs[v], input.y_mins[v],
                                  input.y_maxs[v]);
    }

    std::vector<MCMCSample> samples;

    for(const int query_index : std::views::iota(0, kMaxQueryCount)) {
        if(query_index == 0 || query_index == 100) {
            samples.clear();
            samples.push_back(MCMCSample(points));

            std::vector<int> vs;
            for(const int v : std::views::iota(0, kVertexCount)) {
                vs.push_back(v);
            }
            while(samples.size() < 100) {
                MCMCSample sample = samples.back();
                RunGibbsSampling(vs, distance_contraint_index, sample.points, ranges);
                samples.push_back(sample);
            }
        }

        if(g_time.CurrentMs() > kQueryPhaseTimeLimit) {
            std::cerr << "Reached time limit in query phase (" << query_index
                      << " queries)." << std::endl;
            break;
        }
        const int v = scored_vertices[query_index].second;
        RunSingleQueryStep(input, query_index, v, v_to_reserved, points, samples,
                           distance_contraint_index, ranges);
    }
}

#endif // QUERY_PHASE_CPP_

void InitializePoints(const ProblemInput &problem_input,
                      std::array<Double2D, kVertexCount> &points) {
    for(const int i : std::views::iota(0, kVertexCount)) {
        points[i].x = (problem_input.x_mins[i] + problem_input.x_maxs[i]) * 0.5;
        points[i].y = (problem_input.y_mins[i] + problem_input.y_maxs[i]) * 0.5;
    }
}

std::vector<RangeConstraint> MakeRangeConstraints(const ProblemInput &input) {
    std::vector<RangeConstraint> range_constraints;
    range_constraints.reserve(kVertexCount);
    for(const int i : std::views::iota(0, kVertexCount)) {
        range_constraints.emplace_back(i, input.x_mins[i], input.x_maxs[i],
                                       input.y_mins[i], input.y_maxs[i]);
    }
    return range_constraints;
}

std::vector<int> ReserveIsolatedVertices(const ProblemInput &input) {
    int isolated_cluster_count = 0;
    // return {};
    for(const int cluster_id : std::views::iota(0, input.cluster_count)) {
        if(input.cluster_sizes[cluster_id] == 1) {
            isolated_cluster_count++;
        }
    }
    std::cerr << "# isolated clusters: " << isolated_cluster_count << std::endl;

    int reserve_count = isolated_cluster_count / 2;
    if(reserve_count == 0) {
        return {};
    }

    if(input.max_query_size > 5 || input.max_error_width < 1000) {
        std::cerr << "isolated vertices reservation disabled" << std::endl;
        return {};
    }
    std::cerr << "isolated vertices reservation enabled" << std::endl;

    std::vector<int> reserved_certices;
    std::vector<std::pair<double, int>> scored_vertices;
    scored_vertices.reserve(kVertexCount);
    for(const int i : std::views::iota(0, kVertexCount)) {
        const double score = (input.x_maxs[i] - input.x_mins[i]) +
                             (input.y_maxs[i] - input.y_mins[i]);
        scored_vertices.emplace_back(score, i);
    }
    std::sort(scored_vertices.begin(), scored_vertices.end(), std::greater<>());

    std::vector<int> reserved_vertices;
    reserved_vertices.reserve(reserve_count);
    for(int i = 0; i < reserve_count; i++) {
        // std::cerr << "reserved_vertices: " << scored_vertices[i].second << " "
        //           << scored_vertices[i].first << std::endl;
        reserved_vertices.push_back(scored_vertices[i].second);
    }

    return reserved_vertices;
}

std::array<Double2D, kVertexCount>
MakeAveragePoints(const std::vector<MCMCSample> &samples) {
    std::array<Double2D, kVertexCount> points{};
    for(const auto &sample : samples) {
        for(const int i : std::views::iota(0, kVertexCount)) {
            points[i].x += sample.points[i].x / samples.size();
            points[i].y += sample.points[i].y / samples.size();
        }
    }
    return points;
}

std::vector<MSTCluster> Solve(Env &env, const ProblemInput &problem_input) {
    std::array<Double2D, kVertexCount> points;
    InitializePoints(problem_input, points);
    const std::vector<RangeConstraint> range_constraints =
        MakeRangeConstraints(problem_input);
    ConstraintSet distance_contraint_index;
    for(const auto &range_constraint : range_constraints) {
        distance_contraint_index.AddRangeConstraint(range_constraint);
    }
    std::vector<int> reserved_isolated_vertices =
        ReserveIsolatedVertices(problem_input);

    // // Phase 1: Query phase.
    // RunQueryPhase(problem_input, reserved_isolated_vertices, points,
    //               distance_contraint_index);

    // // Output information for the query phase.
    // std::cerr << "Query phase completed." << std::endl;

    // std::cerr << "time: " << g_time.CurrentMs() << "ms" << std::endl;
    // std::cerr << "# distance constraints: "
    //           << distance_contraint_index.distance_constraints.size()
    //           << std::endl;
    // CheckConstraints(distance_contraint_index, points);
    // std::cerr << std::endl;

    // // Phase 2: Sampling phase.
    // std::vector<MCMCSample> samples;
    // DoubleMatrix distance_matrix(kVertexCount, kVertexCount);
    // RunMCMCPhase(points, distance_contraint_index, samples, distance_matrix);

    // std::cerr << "Sampling phase completed." << std::endl;
    // std::cerr << "# time: " << g_time.CurrentMs() << "ms" << std::endl;
    // CheckConstraints(distance_contraint_index, samples.back().points);
    // std::cerr << std::endl;

    std::cerr << "Running coordinate estimation..." << std::endl;
    auto x_best = solve_coord_estimate(env);
    for(int i = 0; i < kVertexCount; i++) {
        points[i].x = x_best[i].first;
        points[i].y = x_best[i].second;
    }
    std::cerr << "Coordinate estimation completed." << std::endl;

    DoubleMatrix distance_matrix(kVertexCount, kVertexCount);
    for(const int i : std::views::iota(0, kVertexCount)) {
        for(const int j : std::views::iota(i + 1, kVertexCount)) {
            const double d = Distance(points[i], points[j]);
            distance_matrix.Set(i, j, d);
            distance_matrix.Set(j, i, d);
        }
    }

    // // Phase 3: Initial clustering phase.
    // points = MakeAveragePoints(samples);

    std::vector<int> assignment(kVertexCount, -1);
    RunClusteringPhase(problem_input, reserved_isolated_vertices, points,
                       distance_matrix, assignment);
    std::cerr << "Initial clustering phase completed." << std::endl;
    std::cerr << "# time: " << g_time.CurrentMs() << "ms" << std::endl;
    std::cerr << std::endl;

    // Phase 4: Clustering iteration phase.
    std::vector<std::vector<Edge>> cluster_to_mst;
    RunClusteringIterationPhase(problem_input, points, distance_matrix,
                                assignment, cluster_to_mst);
    std::cerr << "Clustering iteration phase completed." << std::endl;
    std::cerr << "# time: " << g_time.CurrentMs() << "ms" << std::endl;
    std::cerr << std::endl;

    // Phase 5: Finalize phase.
    const std::vector<MSTCluster> clusters =
        FinalizeSolution(problem_input, assignment, cluster_to_mst);
    std::cerr << "Finalize phase completed." << std::endl;
    std::cerr << "# time: " << g_time.CurrentMs() << "ms" << std::endl;
    std::cerr << std::endl;
    return clusters;
}

#endif // SOLVER_CPP_

ProblemInput ReadProblemInput() {
    ProblemInput problem_input;
    int unused_vertex_count;
    std::cin >> unused_vertex_count;
    std::cin >> problem_input.cluster_count;
    int unused_max_query_count;
    std::cin >> unused_max_query_count;
    std::cin >> problem_input.max_query_size;
    std::cin >> problem_input.max_error_width;

    problem_input.cluster_sizes.resize(problem_input.cluster_count);
    for(const int i : std::views::iota(0, problem_input.cluster_count)) {
        std::cin >> problem_input.cluster_sizes[i];
    }

    for(const int i : std::views::iota(0, kVertexCount)) {
        std::cin >> problem_input.x_mins[i];
        std::cin >> problem_input.x_maxs[i];
        std::cin >> problem_input.y_mins[i];
        std::cin >> problem_input.y_maxs[i];
    }
    return problem_input;
}

void ReadActualPoints() {
    for(const int i : std::views::iota(0, kVertexCount)) {
        std::cin >> g_actual_points[i].x;
        std::cin >> g_actual_points[i].y;
    }
}

void WriteSolution(const std::vector<MSTCluster> &clusters) {
    std::cerr << "Writing solution..." << std::endl;
    std::cout << "!" << std::endl;
    for(const auto &cluster : clusters) {
        for(const int i : std::views::iota(0, int(cluster.vs.size()))) {
            if(i > 0) {
                std::cout << " ";
            }
            std::cout << cluster.vs[i];
        }
        std::cout << std::endl;
        for(const auto &edge : cluster.mst_edges) {
            std::cout << edge.s << " " << edge.t << std::endl;
        }
    }
}

void Run() {
    auto env = EnvOnline();

    if(env.L <= SOLVE_L_THREAD) {
        solve_for_group(env, env.center_points);
        return;
    }

    for(int i = 0; i < GLOBAL_N; ++i) {
        x_estimate[i] = env.center_points[i];
    }
    for(int i = 0; i < GLOBAL_N; ++i) {
        for(int j = 0; j < 4; ++j) {
            global_rectangles[i][j] = env.rectangles[i][j];
        }
    }

    ProblemInput problem_input;
    problem_input.cluster_count = env.M;
    problem_input.max_query_size = env.L;
    problem_input.max_error_width = env.W;
    problem_input.cluster_sizes.resize(problem_input.cluster_count);
    for(const int i : std::views::iota(0, problem_input.cluster_count)) {
        problem_input.cluster_sizes[i] = env.G[i];
    }
    for(const int i : std::views::iota(0, kVertexCount)) {
        problem_input.x_mins[i] = env.rectangles[i][0];
        problem_input.x_maxs[i] = env.rectangles[i][1];
        problem_input.y_mins[i] = env.rectangles[i][2];
        problem_input.y_maxs[i] = env.rectangles[i][3];
    }

    std::cerr << "cluster_count: " << problem_input.cluster_count
              << ", max_query_size: " << problem_input.max_query_size
              << ", max_error_width: " << problem_input.max_error_width
              << "xmins[0]: " << problem_input.x_mins[0]
              << ", xmaxs[0]: " << problem_input.x_maxs[0]
              << ", ymins[0]: " << problem_input.y_mins[0]
              << ", ymaxs[0]: " << problem_input.y_maxs[0]
              << ", cluster_sizes[0]: " << problem_input.cluster_sizes[0]
              << std::endl;

    if(g_is_local_mode) {
        ReadActualPoints();
    }
    const std::vector<MSTCluster> solution = Solve(env, problem_input);
    WriteSolution(solution);
}

int main(int argc, char *argv[]) {
    for(int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if(arg == "l") {
            g_is_local_mode = true;
        }
        if(arg.starts_with("tf=")) {
            g_time_factor = std::stof(arg.substr(3));
        }
    }
    Run();
}
