#include <bits/stdc++.h>
#include <typeinfo>
using namespace std;

// =================================
// テンプレート
// =================================
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

class TimeKeeper {
  private:
    chrono::high_resolution_clock::time_point start_time_;
    double time_threshold_;

  public:
    TimeKeeper(const double &time_threshold)
        : start_time_(chrono::high_resolution_clock::now()),
          time_threshold_(time_threshold) {
    }

    double getElapsedTime() const {
        auto diff = chrono::high_resolution_clock::now() - this->start_time_;
        return chrono::duration<double, milli>(diff).count();
    }

    bool isTimeOver() const {
        return this->getElapsedTime() >= this->time_threshold_;
    }
};

template <typename Derived, typename UIntType>
class XorshiftBase {
  public:
    using UInt = UIntType;

    UInt next() {
        return static_cast<Derived *>(this)->next();
    }

    // 任意の整数型を返すようテンプレート化（戻り値型を明示）
    UInt randint(UInt max) {
        return next() % max;
    }

    UInt randint(UInt low, UInt high) {
        return low + next() % (high - low + 1);
    }

    double rand() {
        constexpr int bits = std::numeric_limits<UInt>::digits;         // 仮数部のbit数ではなく、整数としてのbit数
        constexpr int float_bits = std::numeric_limits<double>::digits; // 仮数部の精度bit数（float=24, double=53）

        if constexpr(bits >= float_bits) {
            UInt value = next() >> (bits - float_bits); // 上位 float_bits を使う
            return static_cast<double>(value) / static_cast<double>(UInt(1) << float_bits);
        } else {
            return static_cast<double>(next()) / static_cast<double>(std::numeric_limits<UInt>::max());
        }
    }

    // 離散分布サンプリング（常に int でOK）
    int sample_discrete(const std::vector<double> &weights) {
        double total = std::accumulate(weights.begin(), weights.end(), 0.0);
        double r = rand() * total;
        double cumulative = 0.0;
        for(size_t i = 0; i < weights.size(); ++i) {
            cumulative += weights[i];
            if(r < cumulative) {
                return static_cast<int>(i);
            }
        }
        return static_cast<int>(weights.size() - 1);
    }

    // イテレータから k 個サンプル（順序ランダム）
    template <typename Iterator>
    std::vector<typename std::iterator_traits<Iterator>::value_type>
    random_sample(Iterator begin, Iterator end, int k) {
        using T = typename std::iterator_traits<Iterator>::value_type;
        std::vector<T> pool(begin, end);
        int n = static_cast<int>(pool.size());
        for(int i = 0; i < k; ++i) {
            int j = i + randint(n - i);
            std::swap(pool[i], pool[j]);
        }
        return std::vector<T>(pool.begin(), pool.begin() + k);
    }

    // シャッフル
    template <typename T>
    void shuffle(std::vector<T> &vec) {
        for(int i = (int)(vec.size()) - 1; i > 0; --i) {
            int j = randint(i + 1);
            std::swap(vec[i], vec[j]);
        }
    }
};

class Xorshift32 : public XorshiftBase<Xorshift32, uint32_t> {
  private:
    uint32_t state;

  public:
    explicit Xorshift32(uint32_t seed = 2525)
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
};

class Xorshift64 : public XorshiftBase<Xorshift64, uint64_t> {
  private:
    uint64_t state;

  public:
    explicit Xorshift64(uint64_t seed = 202520252025ULL)
        : state(seed) {
    }

    uint64_t next() {
        uint64_t x = state;
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        state = x;
        return x;
    }
};

class Xorshift128Plus : public XorshiftBase<Xorshift128Plus, uint64_t> {
  private:
    uint64_t s[2];

  public:
    Xorshift128Plus(uint64_t seed1 = 123456789, uint64_t seed2 = 987654321) {
        s[0] = seed1 ? seed1 : 1;
        s[1] = seed2 ? seed2 : 2;
    }

    uint64_t next() {
        uint64_t s1 = s[0];
        uint64_t s0 = s[1];
        s[0] = s0;
        s1 ^= s1 << 23;
        s[1] = s1 ^ s0 ^ (s1 >> 17) ^ (s0 >> 26);
        return s[1] + s0;
    }
};

Xorshift32 x32rng;

class ZobristHash {
  public:
    vector<uint64_t> piece_keys;
    uint64_t hash;
    int piece_count;

    ZobristHash(int piece_count) {
        Xorshift64 rng;
        for(int i = 0; i < piece_count; ++i) {
            piece_keys.push_back(rng.next());
        }
        hash = 0;
        this->piece_count = piece_count;
    }

    void initHash(const vector<int> &board) {
        hash = 0;
        for(const auto &piece : board) {
            hash ^= piece_keys[piece];
        }
    }

    void updateHash(int piece, int old_piece) {
        hash ^= piece_keys.at(piece);
        hash ^= piece_keys.at(old_piece);
    }

    uint64_t getHash() const {
        return hash;
    }
};

class WalkersAlias {
  private:
    size_t n;
    std::vector<double> p; // probability threshold
    std::vector<int> a;    // alias index

  public:
    void setWeight(const std::vector<double> &weights) {
        n = weights.size();
        p.resize(n);
        a.resize(n);

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

        for(int i : large)
            p[i] = 1.0;
        for(int i : small)
            p[i] = 1.0;
    }

    int choice(Xorshift32 &rng) {
        double r = rng.rand() * n;
        int i = static_cast<int>(r);
        if(r - i < p[i])
            return i;
        else
            return a[i];
    }
};

// --- 定数 ---
constexpr int INF = 1e9;
constexpr int N = 20;
constexpr int M = 40;
const string DIRECTIONS_STR[4] = {"L", "R", "U", "D"};
const int DX[4] = {-1, 1, 0, 0};
const int DY[4] = {0, 0, -1, 1};
constexpr double MAX_TIME_SEC = 200.0;

// --- 構造体 ---
struct Coord {
    int x, y;
    bool operator==(const Coord &other) const {
        return x == other.x && y == other.y;
    }
};

namespace std {
template <>
struct hash<Coord> {
    size_t operator()(const Coord &c) const {
        return (size_t)c.x * 10007 + c.y;
    }
};
} // namespace std

struct ProblemInput {
    Coord start;
    vector<Coord> target_coords;
};

struct Action {
    string command, direction;
};

// --- 入出力 ---
ProblemInput problem_input() {
    int dummy_N, dummy_M;
    cin >> dummy_N >> dummy_M;
    int sy, sx;
    cin >> sy >> sx;
    Coord start = {sx, sy};

    vector<Coord> targets;
    for(int i = 0; i < M - 1; ++i) {
        int y, x;
        cin >> y >> x;
        targets.push_back({x, y});
    }
    return {start, targets};
}

void problem_output(const vector<Action> &actions) {
    for(auto &act : actions) {
        cout << act.command << " " << act.direction << '\n';
    }
}

// --- 経路復元 ---
vector<int> restore_path(const vector<int> &prev, int start, int goal) {
    if(start == goal)
        return {};
    vector<int> path = {goal};
    while(path.back() != start) {
        if(prev[path.back()] == -1)
            return {};
        path.push_back(prev[path.back()]);
    }
    reverse(path.begin(), path.end());
    return path;
}

// --- dijkstra ---
vector<Action> dijkstra(const vector<vector<pair<int, int>>> &graph, int start, int goal) {
    vector<int> dist(N * N, INF), prev(N * N, -1);
    vector<bool> used(N * N, false);
    using P = pair<int, int>;
    priority_queue<P, vector<P>, greater<>> pq;
    dist[start] = 0;
    pq.push({0, start});

    while(!pq.empty()) {
        auto [d, v] = pq.top();
        pq.pop();
        if(used[v])
            continue;
        used[v] = true;
        for(auto [cost, to] : graph[v]) {
            if(dist[to] > dist[v] + cost) {
                dist[to] = dist[v] + cost;
                prev[to] = v;
                pq.push({dist[to], to});
            }
        }
    }

    auto path = restore_path(prev, start, goal);
    if(path.empty())
        return {};

    vector<Action> actions;
    for(int i = 0; i < (int)path.size() - 1; ++i) {
        int v1 = path[i], v2 = path[i + 1];
        int x1 = v1 % N, y1 = v1 / N;
        int x2 = v2 % N, y2 = v2 / N;

        string dir;
        if(x1 == x2)
            dir = (y1 < y2 ? "D" : "U");
        else
            dir = (x1 < x2 ? "R" : "L");

        string cmd = (abs(x1 - x2) + abs(y1 - y2) == 1) ? "M" : "S";
        actions.push_back({cmd, dir});
    }
    return actions;
}

// --- グラフ構築 ---
vector<vector<pair<int, int>>> construct_graph() {
    vector<vector<pair<int, int>>> graph(N * N);
    for(int x = 0; x < N; ++x) {
        for(int y = 0; y < N; ++y) {
            int v = y * N + x;
            for(int d = 0; d < 4; ++d) {
                int nx = x + DX[d], ny = y + DY[d];
                if(nx >= 0 && nx < N && ny >= 0 && ny < N) {
                    graph[v].push_back({1, ny * N + nx});
                }
            }
            if(x != 0)
                graph[v].push_back({1, y * N + 0});
            if(x != N - 1)
                graph[v].push_back({1, y * N + (N - 1)});
            if(y != 0)
                graph[v].push_back({1, 0 * N + x});
            if(y != N - 1)
                graph[v].push_back({1, (N - 1) * N + x});
        }
    }
    return graph;
}

// --- 壁追加 ---
void update_edge(vector<vector<pair<int, int>>> &graph, const Coord &wall) {
    int wx = wall.x, wy = wall.y;
    int v = wy * N + wx;
    graph[v].clear();

    for(int fx = 0; fx < N; ++fx) {
        int fv = wy * N + fx;
        vector<pair<int, int>> new_edges;
        for(auto [cost, to] : graph[fv]) {
            int tx = to % N, ty = to / N;
            if(fx == wx)
                continue;
            if(fx < wx && wx <= tx) {
                int nwx = wx - 1;
                if(fx != nwx)
                    new_edges.push_back({cost, ty * N + nwx});
            } else if(tx <= wx && wx < fx) {
                int nwx = wx + 1;
                if(fx != nwx)
                    new_edges.push_back({cost, ty * N + nwx});
            } else {
                new_edges.push_back({cost, to});
            }
        }
        graph[fv] = move(new_edges);
    }

    for(int fy = 0; fy < N; ++fy) {
        int fv = fy * N + wx;
        vector<pair<int, int>> new_edges;
        for(auto [cost, to] : graph[fv]) {
            int tx = to % N, ty = to / N;
            if(fy == wy)
                continue;
            if(fy < wy && wy <= ty) {
                int nwy = wy - 1;
                if(fy != nwy)
                    new_edges.push_back({cost, nwy * N + wx});
            } else if(ty <= wy && wy < fy) {
                int nwy = wy + 1;
                if(fy != nwy)
                    new_edges.push_back({cost, nwy * N + wx});
            } else {
                new_edges.push_back({cost, to});
            }
        }
        graph[fv] = move(new_edges);
    }
}

// --- 経路作成 ---
vector<Action> walking(ProblemInput &problem_data, vector<vector<pair<int, int>>> graph, const vector<Coord> &walk_inds) {
    unordered_set<Coord> walk_set(walk_inds.begin(), walk_inds.end());
    vector<Action> actions;
    Coord start = problem_data.start;

    for(const auto &goal : problem_data.target_coords) {
        int sx = start.x, sy = start.y;
        for(int d = 0; d < 4; ++d) {
            int nx = sx + DX[d], ny = sy + DY[d];
            if(walk_set.count({nx, ny})) {
                update_edge(graph, {nx, ny});
                walk_set.erase({nx, ny});
                actions.push_back({"A", DIRECTIONS_STR[d]});
            }
        }
        int sv = sy * N + sx, gv = goal.y * N + goal.x;
        auto path = dijkstra(graph, sv, gv);
        if(path.empty())
            return {};
        actions.insert(actions.end(), path.begin(), path.end());
        start = goal;
    }
    return actions;
}

vector<Action> greedy(ProblemInput &problem_data) {
    auto graph = construct_graph();
    auto ret = walking(problem_data, graph, {});
    assert(!ret.empty());
    return ret;
}

unordered_set<int> neighbor(const unordered_set<int> &now_inds, int walk_n) {
    auto new_inds = now_inds;
    // 確率p
    if(now_inds.size() == 1 || x32rng.rand() < 0.5)
        // 追加
        while(true) {
            int i = x32rng.randint(0, (int)walk_n - 1);
            if(!now_inds.contains(i)) {
                new_inds.insert(i);
                assert(now_inds.size() + 1 == new_inds.size());
                return new_inds;
            }
        }
    else {
        // 削除
        while(true) {
            int k = x32rng.randint(0, (int)now_inds.size() - 1);
            vector<int> inds_vec = {new_inds.begin(), new_inds.end()};
            auto remove_v = inds_vec[k];
            new_inds.erase(remove_v);
            assert(now_inds.size() - 1 == new_inds.size());
            return new_inds;
        }
    }
}

vector<Action> random_walls(ProblemInput &problem_data) {
    unordered_set<Coord> walls;
    for(auto &coord : problem_data.target_coords) {
        for(int d = 0; d < 4; ++d) {
            int nx = coord.x + DX[d], ny = coord.y + DY[d];
            if(nx >= 0 && nx < N && ny >= 0 && ny < N) {
                walls.insert({nx, ny});
            }
        }
    }
    vector<Coord> wall_list(walls.begin(), walls.end());

    auto best_action = greedy(problem_data);
    int best_cost = best_action.size();

    auto start = chrono::high_resolution_clock::now();

    int k = 20;
    unordered_set<int> selected;
    for(int i = 0; i < k; ++i) {
        selected.emplace(x32rng.randint(0, (int)wall_list.size() - 1));
    }
    unordered_set<int> best_selected = selected;

    int simulate_cnt = 0;
    while(true) {
        auto now = chrono::high_resolution_clock::now();
        double elapsed = chrono::duration<double>(now - start).count();
        if(elapsed > MAX_TIME_SEC)
            break;

        auto new_selected = neighbor(selected, wall_list.size());

        auto graph = construct_graph();
        vector<Coord> selected_coords;
        for(int i : new_selected) {
            selected_coords.push_back(wall_list[i]);
        }
        auto actions = walking(problem_data, graph, selected_coords);
        if(actions.empty()) {
            continue;
        }

        int cost = actions.size();
        if((int)actions.size() < best_cost) {
            best_cost = actions.size();
            best_action = move(actions);
            best_selected = new_selected;
            selected = new_selected;
        }
        simulate_cnt++;
        if(simulate_cnt % 100 == 0) {
            cerr << format("simulate_cnt: {} best_cost: {} ({})", simulate_cnt, best_cost, selected.size()) << endl;
            // for(auto &selected : selected_coords) {
            //     cerr << format("({},{}) ", selected.x, selected.y);
            // }
            // cerr << endl;
        }
    }
    return best_action;
}

// --- メイン ---
void solve() {
    auto problem_data = problem_input();
    auto actions = random_walls(problem_data);
    problem_output(actions);
}

int main() {
    solve();
}
