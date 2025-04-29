#include <bits/stdc++.h>
using namespace std;

// --- 定数 ---
constexpr int INF = 1e9;
constexpr int N = 20;
constexpr int M = 40;
const vector<string> COMMANDS = {"M", "S", "A"};
const vector<string> DIRECTIONS_STR = {"L", "R", "U", "D"};
const vector<pair<int, int>> DIRECTIONS_LIST = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
const double MAX_TIME_SEC = 1.80;

// --- 構造体 ---
struct Coord {
    int x, y;
    bool operator==(const Coord &other) const {
        return x == other.x && y == other.y;
    }
    bool operator<(const Coord &other) const {
        return tie(x, y) < tie(other.x, other.y);
    }
};

struct ProblemInput {
    Coord start;
    vector<Coord> target_coords;
};

struct Action {
    string command;
    string direction;
};

// --- グローバル変数 ---
mt19937 rng(0);

// --- 関数定義 ---
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

void problem_output(const vector<Action> &action_list) {
    for(auto &action : action_list) {
        cout << action.command << " " << action.direction << '\n';
    }
}

vector<int> _restore_path(const vector<int> &prev, int start, int goal) {
    vector<int> path;
    path.push_back(goal);
    while(path.back() != start) {
        if(prev[path.back()] == -1)
            return {};
        path.push_back(prev[path.back()]);
    }
    reverse(path.begin(), path.end());
    return path;
}

vector<Action> dijkstra(const vector<set<pair<int, int>>> &graph, int start, int goal) {
    int graph_N = graph.size();
    vector<int> d(graph_N, INF);
    vector<int> prev(graph_N, -1);
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> pq;
    pq.push({0, start});
    d[start] = 0;

    while(!pq.empty()) {
        auto [now_cost, v] = pq.top();
        pq.pop();
        if(d[v] < now_cost)
            continue;
        for(auto [cost, to] : graph[v]) {
            if(d[to] > d[v] + cost) {
                d[to] = d[v] + cost;
                prev[to] = v;
                pq.push({d[to], to});
            }
        }
    }

    auto path = _restore_path(prev, start, goal);
    if(path.empty())
        return {};

    vector<Action> actions;
    for(int i = 0; i < (int)path.size() - 1; ++i) {
        int prev_v = path[i];
        int next_v = path[i + 1];
        int x = prev_v % N, y = prev_v / N;
        int dx = next_v % N, dy = next_v / N;

        string direction;
        if(x == dx)
            direction = (y < dy) ? "D" : "U";
        else
            direction = (x < dx) ? "R" : "L";

        int diff = abs(x - dx) + abs(y - dy);
        string command = (diff == 1) ? "M" : "S";

        actions.push_back({command, direction});
    }
    return actions;
}

vector<set<pair<int, int>>> construct_graph(int n) {
    vector<set<pair<int, int>>> graph(n * n);
    for(int x = 0; x < n; ++x) {
        for(int y = 0; y < n; ++y) {
            int v = y * n + x;
            for(auto [dx, dy] : DIRECTIONS_LIST) {
                int nx = x + dx, ny = y + dy;
                if(nx >= 0 && nx < n && ny >= 0 && ny < n) {
                    graph[v].insert({1, ny * n + nx});
                }
            }
            if(x != 0)
                graph[v].insert({1, y * n + 0});
            if(x != n - 1)
                graph[v].insert({1, y * n + (n - 1)});
            if(y != 0)
                graph[v].insert({1, 0 * n + x});
            if(y != n - 1)
                graph[v].insert({1, (n - 1) * n + x});
        }
    }
    return graph;
}

void update_edge(vector<set<pair<int, int>>> &graph, const Coord &wall) {
    int wall_y = wall.y, wall_x = wall.x;
    int v = wall_y * N + wall_x;
    graph[v].clear();

    for(int from_x = 0; from_x < N; ++from_x) {
        int from_v = wall_y * N + from_x;
        set<pair<int, int>> new_set;
        for(auto [c, to_v] : graph[from_v]) {
            int to_x = to_v % N, to_y = to_v / N;
            if(from_x == wall_x)
                continue;
            if(from_x < wall_x && wall_x <= to_x) {
                int new_to_x = wall_x - 1;
                if(from_x != new_to_x)
                    new_set.insert({c, to_y * N + new_to_x});
            } else if(to_x <= wall_x && wall_x < from_x) {
                int new_to_x = wall_x + 1;
                if(from_x != new_to_x)
                    new_set.insert({c, to_y * N + new_to_x});
            } else {
                new_set.insert({c, to_v});
            }
        }
        graph[from_v] = new_set;
    }

    for(int from_y = 0; from_y < N; ++from_y) {
        int from_v = from_y * N + wall_x;
        set<pair<int, int>> new_set;
        for(auto [c, to_v] : graph[from_v]) {
            int to_x = to_v % N, to_y = to_v / N;
            if(from_y == wall_y)
                continue;
            if(from_y < wall_y && wall_y <= to_y) {
                int new_to_y = wall_y - 1;
                if(from_y != new_to_y)
                    new_set.insert({c, new_to_y * N + wall_x});
            } else if(to_y <= wall_y && wall_y < from_y) {
                int new_to_y = wall_y + 1;
                if(from_y != new_to_y)
                    new_set.insert({c, new_to_y * N + wall_x});
            } else {
                new_set.insert({c, to_v});
            }
        }
        graph[from_v] = new_set;
    }
}

vector<Action> walking(ProblemInput &problem_data, vector<set<pair<int, int>>> graph, vector<Coord> walk_inds) {
    vector<Action> actions;
    Coord start = problem_data.start;
    set<Coord> walk_inds_set(walk_inds.begin(), walk_inds.end());

    for(const Coord &coord : problem_data.target_coords) {
        int sy = start.y, sx = start.x;
        for(int i = 0; i < 4; ++i) {
            auto [dx, dy] = DIRECTIONS_LIST[i];
            int nx = sx + dx, ny = sy + dy;
            if(walk_inds_set.count({nx, ny})) {
                update_edge(graph, {nx, ny});
                walk_inds_set.erase({nx, ny});
                actions.push_back({"A", DIRECTIONS_STR[i]});
            }
        }
        int gy = coord.y, gx = coord.x;
        int sv = sy * N + sx, gv = gy * N + gx;
        auto bfs_path = dijkstra(graph, sv, gv);
        if(bfs_path.empty())
            return {};
        actions.insert(actions.end(), bfs_path.begin(), bfs_path.end());
        start = coord;
    }
    return actions;
}

vector<Action> greedy(ProblemInput &problem_input) {
    auto graph = construct_graph(N);
    auto ret = walking(problem_input, graph, {});
    assert(!ret.empty());
    return ret;
}

vector<Action> random_walls(ProblemInput &problem_data) {
    set<Coord> target_wall_inds;
    for(auto &coord : problem_data.target_coords) {
        for(auto [dx, dy] : DIRECTIONS_LIST) {
            int nx = coord.x + dx, ny = coord.y + dy;
            if(nx >= 0 && nx < N && ny >= 0 && ny < N)
                target_wall_inds.insert({nx, ny});
        }
    }
    vector<Coord> target_wall_list(target_wall_inds.begin(), target_wall_inds.end());

    auto best_action = greedy(problem_data);
    int best_cost = best_action.size();
    int best_wall_delete = 0;
    int simulate_cnt = 0;

    auto start_time = chrono::high_resolution_clock::now();

    while(true) {
        auto now = chrono::high_resolution_clock::now();
        double elapsed = chrono::duration<double>(now - start_time).count();
        if(elapsed > MAX_TIME_SEC)
            break;

        int k = uniform_int_distribution<int>(1, 50)(rng);
        vector<Coord> target_coords;
        for(int i = 0; i < k; ++i) {
            target_coords.push_back(target_wall_list[uniform_int_distribution<int>(0, target_wall_list.size() - 1)(rng)]);
        }
        auto graph = construct_graph(N);
        auto actions = walking(problem_data, graph, target_coords);
        if(actions.empty())
            continue;
        if((int)actions.size() < best_cost) {
            best_cost = actions.size();
            best_action = actions;
            best_wall_delete = target_coords.size();
        }
        simulate_cnt++;
        cerr << simulate_cnt << " best_cost: " << best_cost << " (" << best_wall_delete << " del)" << endl;
    }

    return best_action;
}

void solve() {
    auto problem_data = problem_input();
    auto best_action = random_walls(problem_data);
    problem_output(best_action);
}

int main() {
    solve();
}
