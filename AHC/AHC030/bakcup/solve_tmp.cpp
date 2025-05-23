#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <vector>
using namespace std;

bool is_out_of_range(int y, int x, int N) { return !((0 <= x && x < N) && (0 <= y && y < N)); }

// ----- Domain ----- //

class Env {
  public:
    int N, M;
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
    std::vector<int> queries2;
    std::vector<std::vector<int>> fixed_board;

    Env(const std::string &file_path) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file" << std::endl;
            return;
        }

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
        for (const auto &row : grids) {
            total_oil_num += row.size();
        }

        score = 1e9;

        int cnt = 0;
        for (int y = 0; y < N; ++y) {
            for (int x = 0; x < N; ++x) {
                if (ans_grid[y][x] > 0) {
                    cnt += 1;
                }
            }
        }
        ans_point_size = cnt;

        _sum_oil = 0;
        _score = 0;

        plus_coordinates.clear();
        queries1.clear();
        queries2.clear();
        fixed_board.assign(N, std::vector<int>(N, -1));
    }

    bool is_ok() { return _sum_oil == total_oil_num; }

    int query1(const std::pair<int, int> &coordinate) {
        int y = coordinate.first;
        int x = coordinate.second;
        _score += 1;
        int resp = ans_grid[y][x];
        queries1.emplace_back(coordinate, resp);
        fixed_board[y][x] = resp;
        if (resp > 0) {
            _sum_oil += resp;
            plus_coordinates.emplace_back(coordinate);
        }
        return resp;
    }

    int query2(const std::vector<std::pair<int, int>> &coordinates) {
        int k = coordinates.size();
        int vs = 0;
        for (const auto &coord : coordinates) {
            int y = coord.first;
            int x = coord.second;
            vs += ans_grid[y][x];
        }
        double mu, sigma;
        std::tie(mu, sigma) = calc_mu_sigma(k, esp, vs);
        std::default_random_engine generator;
        std::normal_distribution<double> distribution(mu, sigma);
        int resp = std::max(0, static_cast<int>(std::round(distribution(generator))));
        _score += 1 / std::sqrt(k);
        queries2.push_back(resp);
        return resp;
    }

    bool answer(const std::vector<std::pair<int, int>> &coordinates) {
        bool is_ok = total_oil_num == std::accumulate(coordinates.begin(), coordinates.end(), 0, [this](int sum, const std::pair<int, int> &coord) {
                         int y = coord.first;
                         int x = coord.second;
                         return sum + ans_grid[y][x];
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

    size_t query1_count() const { return queries1.size(); }

  private:
    std::pair<double, double> calc_mu_sigma(int k, double e, int vs) const {
        double mu = (k - vs) * e + vs * (1 - e);
        double sigma = k * e * (1 - e);
        return std::make_pair(mu, sigma);
    }
};

// ----- Solver -----
class BoardSimulator {
  public:
    static const int SIMULATE_NUM = 100000000;
    static const double TIME_OUT;

    Env *env;
    int simulate_avg_num;
    int simulate_cnt;

    BoardSimulator(Env *env) : env(env), simulate_avg_num(0), simulate_cnt(0) { std::cout << "OK" << endl; }

    vector<pair<int, int>> search_placement_indexes() {
        int N = env->N;
        int M = env->M;
        auto &f_board = env->fixed_board;
        auto &grids = env->grids;

        vector<pair<int, int>> placement_indexes;
        for (int m = 0; m < M; ++m) {
            auto &now_grids = grids[m];
            set<pair<int, int>> now_placement_indexes;
            for (int y = 0; y < N; ++y) {
                for (int x = 0; x < N; ++x) {
                    bool flag = true;
                    for (const auto &coordinate : now_grids) {
                        int ny = y + coordinate.first;
                        int nx = x + coordinate.second;
                        if (is_out_of_range(ny, nx, N)) {
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
                        now_placement_indexes.emplace(y, x);
                    }
                }
            }
            placement_indexes.insert(placement_indexes.end(), now_placement_indexes.begin(), now_placement_indexes.end());
        }

        return placement_indexes;
    }

    int calc_cost_from_simulated_board(const vector<vector<int>> &simulated_board) {
        int N = env->N;
        auto &f_board = env->fixed_board;

        int cost = 0;
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

    pair<int, int> simulate_board() {
        std::cout << "BOARD_01" << endl;
        double start_time = clock();

        int N = env->N;
        int M = env->M;
        auto &f_board = env->fixed_board;
        auto &grids = env->grids;

        vector<vector<double>> plus_count_board(N, vector<double>(N, 0.0));

        std::cout << "BOARD_02" << endl;
        pair<int, int> max_coordinate = make_pair(-1, -1);
        double max_cnt = -1.0;

        auto placement_indexes = search_placement_indexes();
        for (int simulate_num = 0; simulate_num < SIMULATE_NUM; ++simulate_num) {
            vector<vector<int>> now_board(N, vector<int>(N, 0));
            std::cout << "AAA" << endl;
            pair<int, int> sampling_index;
            for (int m = 0; m < M; ++m) {
                sampling_index = placement_indexes[rand() % placement_indexes.size()];
                std::cout << "BBB" << endl;
                for (const auto &coordinate : grids[m]) {
                    int ny = coordinate.first + sampling_index.first;
                    std::cout << "DDD" << endl;
                    int nx = coordinate.second + sampling_index.second;
                    std::cout << "EEE"
                              << " " << ny << " " << nx << endl;
                    std::cout << coordinate.first << " " << sampling_index.first << endl;
                    std::cout << coordinate.second << " " << sampling_index.second << " " << m << endl;
                    now_board[ny][nx] += 1;
                    std::cout << "FFF" << endl;
                }
                std::cout << "CCC" << endl;
            }

            int cost = calc_cost_from_simulated_board(now_board);
            std::cout << "BOARD_03" << endl;
            if (cost == 0) {
                if (max_coordinate.first == -1) {
                    max_coordinate = sampling_index;
                }
            }
            std::cout << "BOARD_05" << endl;
            for (int y = 0; y < N; ++y) {
                for (int x = 0; x < N; ++x) {
                    if (now_board[y][x] > 0) {
                        plus_count_board[y][x] += 1 / (cost + 1);
                    }
                }
            }
            std::cout << "BOARD_06" << endl;
            if ((clock() - start_time) / CLOCKS_PER_SEC > TIME_OUT / (N * N)) {
                break;
            }
            std::cout << "BOARD_07" << simulate_num << endl;
        }
        std::cout << "BOARD_09" << endl;

        simulate_avg_num = 1 / (1 + simulate_cnt) * (simulate_cnt + SIMULATE_NUM);

        for (int y = 0; y < N; ++y) {
            for (int x = 0; x < N; ++x) {
                if (f_board[y][x] == -1) {
                    continue;
                }
                double cnt = plus_count_board[y][x];
                if (cnt > max_cnt) {
                    max_cnt = cnt;
                    max_coordinate = make_pair(y, x);
                }
            }
        }

        return max_coordinate;
    }

    pair<int, int> get_next_coordinate() { return simulate_board(); }
};

const double BoardSimulator::TIME_OUT = 2.0;

// ----- Run -----
void solve(Env &env) {
    std::cout << "OK1" << endl;
    BoardSimulator board_simulator(&env);
    std::cout << "OK2" << endl;
    for (int i = 0; i < 500; ++i) {
        std::cout << "OK3" << endl;
        if (env.is_ok()) {
            env.answer(env.plus_coordinates);
            break;
        }
        std::cout << "OK4" << endl;
        std::pair<int, int> coordinate = board_simulator.get_next_coordinate();
        std::cout << "OK5" << endl;
        std::vector<std::vector<int>> ans_board;
        if (ans_board.size() > 0) {
            if (env.answer_from_board(ans_board)) {
                break;
            }
        }
        std::cout << "OK6" << endl;
        int resp = env.query1(coordinate);
    }
}

int main() {
    int TEST_NUM = 10;

    for (int test_num = 0; test_num < TEST_NUM; ++test_num) {
        std::cout << "===== " << test_num + 1 << "/" << TEST_NUM << " =====" << std::endl;
        std::string S = std::to_string(test_num);
        S = std::string(std::max(0, 4 - (int)S.size()), '0') + S;
        std::string file_path = "./in/" + S + ".txt";
        Env env(file_path);
        solve(env);
        std::cout << "score: " << env.score << std::endl;
    }
    return 0;
}
