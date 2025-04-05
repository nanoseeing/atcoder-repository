#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Env {
  public:
    int N, M;
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
        file >> N >> M >> esp;

        grids.resize(M);
        for (int i = 0; i < M; ++i) {
            int count;
            file >> count;
            grids[i].resize(count);
            for (int j = 0; j < count; ++j) {
                file >> grids[i][j].first >> grids[i][j].second;
            }
        }

        ans_grid.resize(N, std::vector<int>(N));
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                file >> ans_grid[i][j];
            }
        }

        total_oil_num = 0;
        for (const auto &g : grids) {
            total_oil_num += g.size();
        }

        score = 1e9;

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

    bool is_ok() const { return _sum_oil == total_oil_num; }

    int query1(const std::pair<int, int> &coordinate) {
        int y = coordinate.first;
        int x = coordinate.second;
        _score += 1;
        int resp = ans_grid[y][x];
        queries1.emplace_back(coordinate, resp);
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
                    has_oils.emplace_back(y, x);
                }
            }
        }
        return answer(has_oils);
    }

    int query1_count() const { return queries1.size(); }
};

class BoardSimulator {
  public:
    static const int SIMULATE_NUM = 1e8;
    const float TIME_OUT = 2.0;

    Env env;
    float simulate_avg_num;
    int simulate_cnt;

    BoardSimulator(const Env &env) : env(env), simulate_avg_num(0), simulate_cnt(0) {}

    std::vector<std::pair<int, int>> search_placement_indexes() {
        int N = env.N;
        int M = env.M;
        const auto &f_board = env.fixed_board;
        const auto &grids = env.grids;

        std::vector<std::pair<int, int>> placement_indexes;
        for (int m = 0; m < M; ++m) {
            const auto &now_grids = grids[m];
            for (int y = 0; y < N; ++y) {
                for (int x = 0; x < N; ++x) {
                    bool flag = true;
                    for (const auto &coordinate : now_grids) {
                        int ny = y + coordinate.first;
                        int nx = x + coordinate.second;
                        if (ny < 0 || ny >= N || nx < 0 || nx >= N) {
                            flag = false;
                            break;
                        }
                        if (f_board[ny][nx] == -1) {
                            continue;
                        }
                        if (f_board[ny][nx] == 0) {
                            flag = false;
                            break;
                        }
                    }
                    if (flag) {
                        placement_indexes.emplace_back(y, x);
                    }
                }
            }
        }
        return placement_indexes;
    }

    float calc_cost_from_simulated_board(const std::vector<std::vector<int>> &simulated_board) {
        int N = env.N;
        const auto &f_board = env.fixed_board;

        float cost = 0;
        for (int y = 0; y < N; ++y) {
            for (int x = 0; x < N; ++x) {
                if (f_board[y][x] == -1) {
                    continue;
                }
                int v = abs(simulated_board[y][x] - f_board[y][x]);
                cost += v;
            }
        }
        return cost;
    }

    std::pair<std::pair<int, int>, std::vector<std::vector<int>>> simulate_board() {
        clock_t start_time = clock();

        int N = env.N;
        int M = env.M;
        const auto &f_board = env.fixed_board;
        const auto &grids = env.grids;
        std::vector<std::vector<int>> plus_count_board(N, std::vector<int>(N, 0));
        std::vector<std::vector<int>> ans_board;
        bool only_one_ans_board = false;

        auto placement_indexes = search_placement_indexes();
        for (int simulate_num = 0; simulate_num < SIMULATE_NUM; ++simulate_num) {
            std::vector<std::vector<int>> now_board(N, std::vector<int>(N, 0));
            for (int m = 0; m < M; ++m) {
                std::pair<int, int> sampling_index = placement_indexes[rand() % placement_indexes.size()];
                for (const auto &coord : grids[m]) {
                    int ny = coord.first + sampling_index.first;
                    int nx = coord.second + sampling_index.second;
                    now_board.at(ny).at(nx) += 1;
                }
            }
            float cost = calc_cost_from_simulated_board(now_board);
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
            if ((float)(clock() - start_time) / CLOCKS_PER_SEC > TIME_OUT / (N * N)) {
                break;
            }
        }

        simulate_avg_num = 1 / (1 + simulate_cnt) * (simulate_cnt + SIMULATE_NUM);

        std::pair<int, int> max_coordinate = std::make_pair(-1, -1);
        float max_cnt = -1;
        for (int y = 0; y < N; ++y) {
            for (int x = 0; x < N; ++x) {
                if (f_board[y][x] != -1) {
                    continue;
                }
                float cnt = plus_count_board[y][x];
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

    std::pair<std::pair<int, int>, std::vector<std::vector<int>>> get_next_coordinate() { return simulate_board(); }
};

void solve(Env &env) {
    std::cout << "SOLVE1" << std::endl;
    BoardSimulator board_simulator(env);
    std::cout << "SOLVE2" << std::endl;
    for (int i = 0; i < 500; ++i) {
        std::cout << "SOLVE3" << std::endl;
        if (env.is_ok()) {
            env.answer(env.plus_coordinates);
            break;
        }
        std::cout << "SOLVE4" << std::endl;
        auto [coordinate, ans_board] = board_simulator.get_next_coordinate();
        std::cout << "SOLVE5" << std::endl;
        if (!ans_board.empty()) {
            if (env.answer_from_board(ans_board)) {
                break;
            }
        }
        std::cout << "SOLVE6" << std::endl;
        int resp = env.query1(coordinate);
        std::cout << "SOLVE7" << std::endl;
    }
}

int main() {
    const int TEST_NUM = 100;
    srand(0);
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
