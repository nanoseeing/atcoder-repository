// Score 74.84

#include <algorithm>
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
#include <vector>

int TEST_CASE_NUM = 100;

double SIMULATE_TIME_OUT = 2.0;
int SIMULATE_MAX = 100000;
int SIMULATE_MIN = 50;

int INIT_INDEX_CANDIDATES = 100;
int MIN_INDEX_CANDIDATES = 100;
double RANDOM_INDEX_PROB = 0.1;
int DISCOUNT_NUM = 0;

class Env {
  public:
    int N, M;
    double esp;
    double score;
    int total_oil_num;
    std::vector<std::vector<std::pair<int, int>>> grids;
    std::vector<std::vector<int>> fixed_board;
    std::vector<std::pair<int, int>> plus_coordinates;

  private:
    int _sum_oil;
    double _score;
    std::vector<std::pair<std::pair<int, int>, int>> queries1;
    std::vector<int> queries2;

  public:
    Env() {
        std::cin >> N >> M >> esp;

        grids.resize(M);
        for (int i = 0; i < M; ++i) {
            int k;
            std::cin >> k;
            for (int j = 0; j < k; ++j) {
                int y, x;
                std::cin >> y >> x;
                grids[i].emplace_back(y, x);
            }
        }

        total_oil_num = 0;
        for (const auto &g : grids) {
            total_oil_num += g.size();
        }

        score = 1.0e9;

        _sum_oil = 0;
        _score = 0.0;

        plus_coordinates.clear();
        queries1.clear();
        queries2.clear();
        fixed_board.assign(N, std::vector<int>(N, -1));
    }

    bool is_ok() { return _sum_oil == total_oil_num; }

    int query1(std::pair<int, int> coordinate) {
        std::cout << "q 1 " << coordinate.first << " " << coordinate.second << std::endl;
        int resp;
        std::cin >> resp;
        queries1.emplace_back(coordinate, resp);

        int y = coordinate.first;
        int x = coordinate.second;
        fixed_board[y][x] = resp;
        if (resp > 0) {
            _sum_oil += resp;
            plus_coordinates.emplace_back(coordinate);
        }

        _score += 1.0;
        return resp;
    }

    int query2(const std::vector<std::pair<int, int>> &coordinates) {
        std::cout << "q " << coordinates.size();
        for (const auto &coord : coordinates) {
            std::cout << " " << coord.first << " " << coord.second;
        }
        std::cout << std::endl;

        int resp;
        std::cin >> resp;

        _score += 1.0 / std::sqrt(coordinates.size());
        queries2.push_back(resp);
        return resp;
    }

    bool answer(const std::vector<std::pair<int, int>> &has_oils) {
        std::cout << "a " << has_oils.size();
        for (const auto &oil : has_oils) {
            std::cout << " " << oil.first << " " << oil.second;
        }
        std::cout << std::endl;

        std::string resp;
        std::cin >> resp;

        bool is_ok = resp == "1";
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
    double simulate_avg_num;
    int simulate_cnt;
    std::vector<std::vector<std::pair<int, int>>> _indexes_candidate;

    BoardSimulator() : simulate_avg_num(0), simulate_cnt(0) {}

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

    void _update_indexes_candidate(const std::vector<std::vector<std::pair<int, int>>> &stock_indexes, const std::vector<float> &stock_costs) {
        int index_num = std::max(MIN_INDEX_CANDIDATES, INIT_INDEX_CANDIDATES - simulate_cnt * DISCOUNT_NUM);
        index_num = std::min(index_num, static_cast<int>(stock_indexes.size()));
        std::vector<std::pair<float, std::vector<std::pair<int, int>>>> cost_index_pairs;
        for (size_t i = 0; i < stock_costs.size(); ++i) {
            cost_index_pairs.push_back(std::make_pair(stock_costs[i], stock_indexes[i]));
        }

        std::sort(cost_index_pairs.begin(), cost_index_pairs.end());

        _indexes_candidate.clear();
        for (int i = 0; i < index_num; ++i) {
            _indexes_candidate.push_back(cost_index_pairs[i].second);
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

        auto indexes_candidate = _get_indexes_candidate(env);

        std::vector<std::vector<std::pair<int, int>>> stock_indexes;
        std::vector<float> stock_costs;
        int simulate_num = 0;

        double random_index_prob = std::max(1.0 / M * 1.74, 1.0 - (simulate_cnt / (std::pow(N, 2) / 2.83)));
        // double random_index_prob = std::max(RANDOM_INDEX_PROB, 1.0);
        for (; simulate_num < SIMULATE_MAX; ++simulate_num) {
            std::vector<std::vector<int>> now_board(N, std::vector<int>(N, 0));
            std::vector<std::pair<int, int>> sampling_indexes;
            sampling_indexes = indexes_candidate[rand() % indexes_candidate.size()];
            for (int m = 0; m < M; ++m) {
                if (rand() / static_cast<float>(RAND_MAX) < random_index_prob) {
                    sampling_indexes[m] = placement_indexes[m][rand() % placement_indexes[m].size()];
                }
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
            stock_indexes.push_back(sampling_indexes);
            stock_costs.push_back(cost);
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
                        // double x_diff = std::abs(N / 2 - x * N / (N - 1));
                        // double y_diff = std::abs(N / 2 - y * N / (N - 1));
                        // double center_cost = (x_diff + y_diff) / N * 0.05;
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
        _update_indexes_candidate(stock_indexes, stock_costs);

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
    BoardSimulator board_simulator;
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
    std::srand(0);
    Env env;
    solve(env);
    return 0;
}
