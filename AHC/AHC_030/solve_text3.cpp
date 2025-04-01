#include <bits/stdc++.h>

int TEST_CASE_NUM = 100;

double SIMULATE_TIME_OUT = 0.1;
int SIMULATE_MAX = 100000;
int SIMULATE_MIN = 50;
int INIT_INDEX_CANDIDATES = 1;

struct Env {
    int N;
    int M;
    double esp;
    std::vector<std::vector<std::pair<int, int>>> grids;
    std::vector<std::vector<int>> ans_grid;
    int total_oil_num;
    double score;
    int ans_point_size;
    int _sum_oil;
    double _score;
    std::vector<std::pair<int, int>> plus_coordinates;
    std::vector<std::pair<std::pair<int, int>, int>> queries1;
    std::vector<std::vector<int>> fixed_board;

    Env(const std::string &file_path) {
        std::ifstream file(file_path);
        if (file.is_open()) {
            file >> N >> M >> esp;
            grids.resize(M);
            for (int i = 0; i < M; ++i) {
                int k;
                file >> k;
                grids[i].resize(k);
                for (int j = 0; j < k; ++j) {
                    file >> grids[i][j].first >> grids[i][j].second;
                }
            }
            int tmp;
            for (int i = 0; i < M; ++i) {
                file >> tmp;
                file >> tmp;
            }
            ans_grid.resize(N, std::vector<int>(N));
            for (int i = 0; i < N; ++i) {
                for (int j = 0; j < N; ++j) {
                    file >> ans_grid[i][j];
                }
            }
            file.close();
        }

        total_oil_num = 0;
        for (const auto &g : grids) {
            total_oil_num += g.size();
        }
        score = static_cast<double>(std::pow(10, 9));

        int cnt = 0;
        for (int y = 0; y < N; ++y) {
            for (int x = 0; x < N; ++x) {
                if (ans_grid[y][x] > 0) {
                    cnt++;
                }
            }
        }
        ans_point_size = cnt;

        _sum_oil = 0;
        _score = 0;

        plus_coordinates.clear();
        queries1.clear();
        fixed_board.assign(N, std::vector<int>(N, -1));
    }

    bool is_ok() { return _sum_oil == total_oil_num; }

    int query1(const std::pair<int, int> &coordinate) {
        int y = coordinate.first;
        int x = coordinate.second;
        _score += 1;
        int resp = ans_grid[y][x];
        queries1.push_back(std::make_pair(coordinate, resp));
        fixed_board[y][x] = resp;
        if (resp > 0) {
            _sum_oil += resp;
            plus_coordinates.push_back(coordinate);
        }
        return resp;
    }

    bool answer(const std::vector<std::pair<int, int>> &coordinates) {
        bool is_ok = total_oil_num == std::accumulate(coordinates.begin(), coordinates.end(), 0, [this](int sum, const std::pair<int, int> &coord) {
                         return sum + ans_grid[coord.first][coord.second];
                     });
        if (is_ok) {
            score = _score;
        } else {
            _score += 1;
        }
        return is_ok;
    }

    bool answer_from_board(const std::vector<std::vector<int>> &board) {
        std::vector<std::pair<int, int>> has_oils;
        for (int y = 0; y < N; ++y) {
            for (int x = 0; x < N; ++x) {
                if (board[y][x] > 0) {
                    has_oils.push_back(std::make_pair(y, x));
                }
            }
        }
        return answer(has_oils);
    }

    int query1_count() const { return queries1.size(); }
};

class BoardSimulator {

  public:
    double simulate_avg_num;
    int simulate_cnt;
    std::vector<int> m_list;
    std::vector<std::vector<std::pair<int, int>>> _indexes_candidate;
    std::vector<std::pair<int, int>> best_indexes;

    BoardSimulator(Env &env) : simulate_avg_num(0), simulate_cnt(0) {
        for (int i = 0; i < env.M; ++i) {
            m_list.push_back(i);
        }
        best_indexes = _get_indexes_candidate(env)[0];
    }

    std::vector<int> get_random_index(int num) {
        std::vector<int> result;

        std::vector<int> indices(m_list.size());
        std::iota(indices.begin(), indices.end(), 0);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(indices.begin(), indices.end(), gen);

        for (int i = 0; i < num; ++i) {
            result.push_back(m_list[indices[i]]);
        }

        return result;
    }

    std::vector<std::vector<std::pair<int, int>>> _search_placement_indexes(Env &env) {
        int N = env.N;
        int M = env.M;
        auto &f_board = env.fixed_board;
        auto &grids = env.grids;

        std::vector<std::vector<std::pair<int, int>>> placement_indexes;
        for (int m = 0; m < M; ++m) {
            auto &now_grids = grids[m];
            std::vector<std::pair<int, int>> now_placement_indexes;
            std::vector<std::pair<int, int>> is_M_indexes;
            for (int y = 0; y < N; ++y) {
                for (int x = 0; x < N; ++x) {
                    bool flag = true;
                    int max_num = -1;
                    for (auto &coordinate : now_grids) {
                        int ny = y + coordinate.first;
                        int nx = x + coordinate.second;
                        if (ny < 0 || ny >= N || nx < 0 || nx >= N) {
                            flag = false;
                            break;
                        }
                        if (f_board[ny][nx] == 0) {
                            flag = false;
                            break;
                        }
                        max_num = std::max(max_num, f_board[ny][nx]);
                    }
                    if (flag) {
                        now_placement_indexes.push_back(std::make_pair(y, x));
                        if (max_num == M) {
                            is_M_indexes.push_back(std::make_pair(y, x));
                        }
                    }
                }
            }
            if (is_M_indexes.size() > 0) {
                placement_indexes.push_back(is_M_indexes);
            } else {
                placement_indexes.push_back(now_placement_indexes);
            }
        }
        return placement_indexes;
    }

    std::vector<std::vector<std::pair<int, int>>> _get_indexes_candidate(Env &env) {
        int M = env.M;
        std::random_device rd;
        std::mt19937 gen(rd());

        if (_indexes_candidate.empty()) {
            std::vector<std::vector<std::pair<int, int>>> placement_indexes = _search_placement_indexes(env);

            for (int i = 0; i < INIT_INDEX_CANDIDATES; ++i) {
                std::vector<std::pair<int, int>> sampling_indexes;
                for (int m = 0; m < M; ++m) {
                    std::uniform_int_distribution<> dis(0, placement_indexes[m].size() - 1);
                    int random_index = dis(gen);
                    std::pair<int, int> sampling_index = placement_indexes[m][random_index];
                    sampling_indexes.push_back(sampling_index);
                }
                _indexes_candidate.push_back(sampling_indexes);
            }

            return _indexes_candidate;
        } else {
            return _indexes_candidate;
        }
    }

    double _calc_cost_from_simulated_board(Env &env, const std::vector<std::vector<int>> &simulated_board) {
        int N = env.N;
        auto &f_board = env.fixed_board;

        double cost = 0;
        for (int y = 0; y < N; ++y) {
            for (int x = 0; x < N; ++x) {
                if (f_board[y][x] == -1) {
                    continue;
                }
                double v = std::abs(simulated_board[y][x] - f_board[y][x]);
                cost += v;
            }
        }
        return cost;
    }

    std::pair<std::pair<int, int>, std::vector<std::vector<int>>> get_next_coordinate(Env &env) {
        double start_time = static_cast<double>(clock()) / CLOCKS_PER_SEC;

        int N = env.N;
        int M = env.M;
        auto &f_board = env.fixed_board;
        auto &grids = env.grids;

        std::vector<std::vector<double>> plus_count_board(N, std::vector<double>(N, 0.0));
        std::vector<std::vector<int>> ans_board;
        bool only_one_ans_board = false;

        auto placement_indexes = _search_placement_indexes(env);

        std::vector<std::vector<std::pair<int, int>>> stock_indexes;
        std::vector<float> stock_costs;
        int simulate_num = 0;

        double base_cnt = std::pow(N, 2) / 3;
        double random_index_num = std::max(0.15, (base_cnt - simulate_cnt) / base_cnt);
        random_index_num = std::max(2, (int)(M * random_index_num));

        std::vector<std::pair<int, int>> now_best_indexes = best_indexes;
        double now_cost = 1000000000.0;

        for (; simulate_num < SIMULATE_MAX; ++simulate_num) {
            std::vector<std::vector<int>> now_board(N, std::vector<int>(N, 0));
            std::vector<std::pair<int, int>> sampling_indexes = now_best_indexes;

            std::vector<int> random_index = get_random_index(random_index_num);
            for (int m : random_index) {
                sampling_indexes[m] = placement_indexes[m][rand() % placement_indexes[m].size()];
            }
            for (int m = 0; m < M; ++m) {
                std::pair<int, int> sampling_index = sampling_indexes[m];
                for (const auto &coordinate : grids[m]) {
                    int ny = coordinate.first + sampling_index.first;
                    int nx = coordinate.second + sampling_index.second;
                    now_board[ny][nx] += 1;
                }
            }
            double cost = _calc_cost_from_simulated_board(env, now_board);
            if (cost < now_cost) {
                now_cost = cost;
                now_best_indexes = best_indexes;
            }
            if (cost == 0) {
                if (ans_board.empty()) {
                    ans_board = now_board;
                    only_one_ans_board = true;
                } else {
                    only_one_ans_board = false;
                }
            }
            for (int y = 0; y < N; ++y) {
                for (int x = 0; x < N; ++x) {
                    if (now_board[y][x] > 0) {
                        double center_cost = 0;
                        plus_count_board[y][x] += (center_cost + now_board[y][x]) / (cost + 1);
                    }
                }
            }
            if (simulate_num < SIMULATE_MIN) {
                continue;
            }
            if (static_cast<double>(clock()) / CLOCKS_PER_SEC - start_time > SIMULATE_TIME_OUT / (N * N)) {
                break;
            }
        }
        best_indexes = now_best_indexes;

        std::pair<int, int> max_coordinate = std::make_pair(-1, -1);
        double max_cnt = -1;
        for (int y = 0; y < N; ++y) {
            for (int x = 0; x < N; ++x) {
                if (f_board[y][x] != -1) {
                    continue;
                }
                double cnt = plus_count_board[y][x];
                if (cnt > max_cnt) {
                    max_cnt = cnt;
                    max_coordinate = std::make_pair(y, x);
                }
            }
        }

        simulate_cnt++;
        if (only_one_ans_board) {
            return std::make_pair(max_coordinate, ans_board);
        } else {
            return std::make_pair(max_coordinate, std::vector<std::vector<int>>());
        }
    }
};

void solve(Env &env) {
    BoardSimulator board_simulator(env);
    for (int i = 0; i < 500; ++i) {
        if (env.is_ok()) {
            env.answer(env.plus_coordinates);
            break;
        }
        auto [coordinate, ans_board] = board_simulator.get_next_coordinate(env);
        if (!ans_board.empty()) {
            if (env.answer_from_board(ans_board)) {
                break;
            }
        }
        int resp = env.query1(coordinate);
    }
}

int main() {
    double mean_score = 0;
    std::srand(0);
    for (int test_num = 0; test_num < TEST_CASE_NUM; ++test_num) {
        std::cout << "===== " << test_num + 1 << "/" << TEST_CASE_NUM << " =====" << std::endl;
        std::string S = std::to_string(test_num);
        S = std::string(std::max(0, 4 - (int)S.size()), '0') + S;
        std::string file_path = "./in/" + S + ".txt";
        Env env(file_path);
        solve(env);
        std::cout << "score:" << env.score << std::endl;
        mean_score += env.score;
    }
    mean_score = mean_score / TEST_CASE_NUM;
    std::cout << mean_score << std::endl;
    return 0;
}
