class BoardSimulator {

    static const int SIMULATE_NUM = 100000000;
    static const float TIME_OUT = 2.0;

public:
    Env env;
    float simulate_avg_num;
    int simulate_cnt;

    BoardSimulator(const Env& env) : env(env), simulate_avg_num(0), simulate_cnt(0) {}

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
                    for (auto& coordinate : now_grids) {
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

    float _calc_cost_from_simulated_board(const std::vector<std::vector<int>>& simulated_board) {
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

    std::pair<int, std::vector<std::vector<int>>> get_next_coordinate() {
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
                for (auto& coord : grids[m]) {
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
            return std::make_pair(max_coordinate.first, ans_board);
        } else {
            return std::make_pair(max_coordinate.first, std::vector<std::vector<int>>());
        }
    }
};