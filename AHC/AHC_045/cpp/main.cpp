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
const bool IS_ONLINE_JUDGE = false;

const int SOLVE_L_THREAD = 5; // <= SOLVE_L_THREADなら点推定しない

// 点推定
const double MAX_TIME = 1.00;
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
const int MEMO_SAVE_CAP = 10;
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

        if(is_best) {
            remain_simulate_cnt--;
        } else {
            if(cost_best <= FIN_THRESH) {
                debug_print("!!!!! FIN SA !!!!! (cost_best: ", cost_best, ")");
                is_best = true;
            }
        }

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

int solve_coord_estimate(Env &env) {

    // unordered_map<int, int> query_cnt = define_query_cnt(env, 1.5);
    // int total_query_cnt = 0;
    // for(auto &[g, cnt] : query_cnt) {
    //     total_query_cnt += cnt;
    // }

    const auto query_target = generate_query_target_simple(env, env.Q);

    // ------------------ クエリを実行して制約を得る ---------------------
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

    // ------------------ SA 2 ---------------------
    if(env.M > 1 && IS_POST_SA) {
        tie(ans_v, ans_edges) = simulated_annealing_define_group(
            ans_v, POST_OPT_T0, POST_OPT_T1,
            x_best_mean,
            ans_edges,
            ans_costs,
            POST_OPT_GRUOP_TIME,
            true);
    }

    // tie(ans_v, ans_edges) = update_mst(env, ans_v, ans_edges, query_cnt);

    auto [new_ans_v, new_ans_edges] = update_ans(env, ans_v, ans_edges);
    return env.answer(new_ans_v, new_ans_edges);
}

int solve(Env &env) {
    for(int i = 0; i < GLOBAL_N; ++i) {
        x_estimate[i] = env.center_points[i];
    }
    for(int i = 0; i < GLOBAL_N; ++i) {
        for(int j = 0; j < 4; ++j) {
            global_rectangles[i][j] = env.rectangles[i][j];
        }
    }

    if(env.L <= SOLVE_L_THREAD) {
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
            GLOBAL_START_TIME = chrono::steady_clock::now();
            debug_print("=====");

            stringstream in_path_ss, out_path_ss;
            in_path_ss << "../in/" << setfill('0') << setw(4) << file_num << ".txt";
            out_path_ss << "../out/" << setfill('0') << setw(4) << file_num << ".txt";

            string input_file_path = in_path_ss.str();
            string output_file_path = out_path_ss.str();

            EnvOffline env(input_file_path, output_file_path);
            int cost = solve(env);

            auto end = chrono::steady_clock::now();
            double elapsed = chrono::duration<double>(end - GLOBAL_START_TIME).count();

            debug_print("[", file_num, "/", END_FILE_NUM, "]: ", cost, " ", fixed,
                        setprecision(2), elapsed, "s", "(Q=", env.Q, "L=", env.L,
                        "W=", env.W, ")");

            costs.emplace_back(cost);
        }

        double avg = accumulate(costs.begin(), costs.end(), 0.0) / costs.size();
        double max_cost = *max_element(costs.begin(), costs.end());

        debug_print("avg: ", avg);
        debug_print("max: ", max_cost);

        ofstream result_file("../result/cpp_fast4.txt");
        result_file << "avg: " << avg << "\n";
        for(int pi = 0; pi < costs.size(); ++pi) {
            result_file << setw(4) << setfill('0') << pi << " " << costs[pi] << "\n";
        }
    }

    return 0;
}