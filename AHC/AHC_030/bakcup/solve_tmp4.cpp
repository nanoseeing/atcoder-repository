#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Env {
    int N;
    int M;
    float esp;
    std::vector<std::vector<std::pair<int, int>>> grids;
    std::vector<std::vector<int>> ans_grid;
    int total_oil_num;
    int score;
    int ans_point_size;
    int _sum_oil;
    float _score;
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
        score = static_cast<int>(std::pow(10, 9));

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

    static const int SIMULATE_NUM = 100000000;
    const float TIME_OUT = 2.0;

  public:
    Env env;
    float simulate_avg_num;
    int simulate_cnt;

    BoardSimulator(const Env &env) : env(env), simulate_avg_num(0), simulate_cnt(0) {}

    std::vector<std::vector<std::pair<int, int>>> _search_placement_indexes() {
        int N = env.N;
        int M = env.M;
        std::vector<std::vector<int>> f_board = env.fixed_board;
        std::vector<std::vector<std::pair<int, int>>> grids = env.grids;

        std::vector<std::vector<std::pair<int, int>>> placement_indexes;
        for (int m = 0; m < M; ++m) {
            std::vector<std::pair<int, int>> now_grids = grids[m];
            std::vector<std::pair<int, int>> now_placement_indexes;
            for (int y = 0; y < N; ++y) {
                for (int x = 0; x < N; ++x) {
                    bool flag = true;
                    for (auto &coordinate : now_grids) {
                        int ny, nx;
                        std::tie(ny, nx) = coordinate;
                        if (ny + y < 0 || ny + y >= N || nx + x < 0 || nx + x >= N) {
                            flag = false;
                            break;
                        }
                        if (f_board[ny + y][nx + x] == -1) {
                            continue;
                        }
                        if (f_board[ny + y][nx + x] == 0) {
                            flag = false;
                            break;
                        }
                    }
                    if (flag) {
                        now_placement_indexes.push_back(std::make_pair(y, x));
                    }
                }
            }
            placement_indexes.push_back(now_placement_indexes);
        }
        return placement_indexes;
    }

    float _calc_cost_from_simulated_board(const std::vector<std::vector<int>> &simulated_board) {
        int N = env.N;
        std::vector<std::vector<int>> f_board = env.fixed_board;

        float cost = 0;
        for (int y = 0; y < N; ++y) {
            for (int x = 0; x < N; ++x) {
                if (f_board[y][x] == -1) {
                    continue;
                }
                int v = std::abs(simulated_board[y][x] - f_board[y][x]);
                cost += v;
            }
        }
        return cost;
    }

    std::pair<std::pair<int, int>, std::vector<std::vector<int>>> get_next_coordinate() {
        float start_time = static_cast<float>(clock()) / CLOCKS_PER_SEC;

        int N = env.N;
        int M = env.M;
        std::vector<std::vector<int>> f_board = env.fixed_board;
        std::vector<std::vector<std::pair<int, int>>> grids = env.grids;
        std::vector<std::vector<int>> plus_count_board(N, std::vector<int>(N, 0));
        std::vector<std::vector<int>> ans_board;
        bool only_one_ans_board = false;

        auto placement_indexes = _search_placement_indexes();
        for (int simulate_num = 0; simulate_num < SIMULATE_NUM; ++simulate_num) {
            std::vector<std::vector<int>> now_board(N, std::vector<int>(N, 0));
            for (int m = 0; m < M; ++m) {
                std::pair<int, int> sampling_index = placement_indexes[m][rand() % placement_indexes[m].size()];
                for (auto &coord : grids[m]) {
                    int y, x;
                    std::tie(y, x) = coord;
                    int ny = y + sampling_index.first;
                    int nx = x + sampling_index.second;
                    now_board[ny][nx] += 1;
                }
            }
            float cost = _calc_cost_from_simulated_board(now_board);
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
                        plus_count_board[y][x] += 1 / (cost + 1);
                    }
                }
            }
            if (static_cast<float>(clock()) / CLOCKS_PER_SEC - start_time > TIME_OUT / (N * N)) {
                // シミュレート部分に合計3秒以上かけるとまずい
                break;
            }
        }
        simulate_avg_num = 1 / (1 + simulate_cnt) * (simulate_cnt + SIMULATE_NUM);

        std::pair<int, int> max_coordinate = std::make_pair(-1, -1);
        int max_cnt = -1;
        for (int y = 0; y < N; ++y) {
            for (int x = 0; x < N; ++x) {
                if (f_board[y][x] != -1) {
                    continue;
                }
                int cnt = plus_count_board[y][x];
                if (cnt > max_cnt) {
                    max_cnt = cnt;
                    max_coordinate = std::make_pair(y, x);
                }
            }
        }

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
        auto [coordinate, ans_board] = board_simulator.get_next_coordinate();
        if (!ans_board.empty()) {
            if (env.answer_from_board(ans_board)) {
                break;
            }
        }
        int resp = env.query1(coordinate);
    }
}

int main() {
    const int TEST_NUM = 100;
    std::srand(0);
    for (int test_num = 0; test_num < TEST_NUM; ++test_num) {
        std::cout << "===== " << test_num + 1 << "/" << TEST_NUM << " =====" << std::endl;
        std::string S = std::to_string(test_num);
        S = std::string(std::max(0, 4 - (int)S.size()), '0') + S;
        std::string file_path = "./in/" + S + ".txt";
        Env env(file_path);
        solve(env);
        std::cout << "score:" << env.score << std::endl;
    }
    return 0;
}
