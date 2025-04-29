// 1盤面あたり 48 * 48 * 0.77 * 8 = 14192
// 30ケース で 14192 * 30 = 425760

#include <bits/stdc++.h>
#include <iostream>
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

struct pair_hash {
    size_t operator()(const pii &p) const {
        return hash<int>()(p.first) ^ (hash<int>()(p.second) << 1);
    }
};

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
    explicit Xorshift32(uint32_t seed = 252525)
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
    explicit Xorshift64(uint64_t seed = 252525ULL)
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
    Xorshift128Plus(uint64_t seed1 = 123, uint64_t seed2 = 456) {
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
Xorshift64 x64rng;
Xorshift128Plus x128rng;

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

    int choice() {
        double r = x32rng.rand() * n;
        int i = static_cast<int>(r);
        if(r - i < p[i])
            return i;
        else
            return a[i];
    }
};

const int MAX_X = 50;
const int MAX_Y = 50;
const int MAX_K = 10;
const int MAX_P = 6;
uint64_t piece_keys[MAX_X][MAX_Y][MAX_K][MAX_P];

class ZobristHash {
  public:
    uint64_t hash;

    void initHash(const vector<uint64_t> &board) {
        hash = 0;
        for(const auto &piece : board) {
            hash ^= piece;
        }
    }

    void updateHash(uint64_t piece, uint64_t old_piece) {
        hash ^= piece;
        hash ^= old_piece;
    }

    uint64_t getHash() const {
        return hash;
    }
};
// =================================
// テンプレートここまで
// =================================

struct ProblemInput {
    int N;
    int K;
    int H;
    int W;
    int T;
    vector<vector<vector<char>>> map;
};

ProblemInput ReadInput() {
    int N, K, H, W, T;
    cin >> N >> K >> H >> W >> T;
    vector<vector<vector<char>>> game_map(N, vector<vector<char>>(H, vector<char>(W)));
    for(int n : views::iota(0, N)) {
        for(int h : views::iota(0, H)) {
            string s;
            cin >> s;
            for(int w : views::iota(0, W)) {
                game_map[n][h][w] = s[w];
            }
        }
    }

    ProblemInput input = {N, K, H, W, T, game_map};
    return input;
}

struct Operation {
    char direction;
};

struct History {
    Operation op;
    shared_ptr<History> parent;
    History(const Operation &op, shared_ptr<History> &parent)
        : op(op), parent(parent) {
    }
};

struct Stack {
    shared_ptr<History> head;

    Operation top() {
        return head->op;
    }
    Stack push(const Operation &op) {
        return Stack({make_shared<History>(op, head)});
    }
    Stack pop() {
        return Stack({head->parent});
    }
};

int to_piece(int x, int y, int k, char c) {
    char chars[6] = {' ', '#', 'o', 'x', '@', 'z'}; // zは罠を踏んだ

    int p = -1;
    for(int i : views::iota(0, 6)) {
        if(chars[i] == c) {
            p = i;
            break;
        }
    }
    assert(0 <= x && x < MAX_X);
    assert(0 <= y && y < MAX_Y);
    assert(0 <= k && k < MAX_K);
    if(p == -1) {
        cerr << "Invalid character: [" << c << "]" << endl;
        assert(false);
    }

    return piece_keys[x][y][k][p];
}

using ScoreType = double;

ScoreType scoreFormula(ScoreType score, int trap, int turn, int max_turn) {
    int remain_turn = max_turn - turn;
    return score - trap * remain_turn * 0.9;
}
struct State {
    ScoreType score_;
    int coin_;
    int turn_;
    int trap_;
    ZobristHash hash_;
    ProblemInput problem_input_;
    vector<pii> player_positions_;
    Stack move_history;

    // pii getBaseCoor() {
    //     int start_turn = problem_input_.T - (problem_input_.W - 2) * (problem_input_.H - 2);
    //     if(turn_ < start_turn) {
    //         return {1, 1};
    //     } else {
    //         int elapsed_turn = turn_ - start_turn;
    //         int y = elapsed_turn / 48;
    //         int x = elapsed_turn % 48;
    //         if(elapsed_turn / 48 % 2 == 1) {
    //             x = -x;
    //         }
    //         x++;
    //         y++;
    //         return {y, x};
    //     }
    // }

    pii getBaseCoor() {
        int start_turn = problem_input_.T - (problem_input_.W - 2) * (problem_input_.H - 2);
        if(turn_ < start_turn) {
            int elapsed_turn = turn_;
            double diff = (double)elapsed_turn / start_turn * 24.0;
            int y = 24 - diff;
            int x = 24 - diff;
            return {y, x};
        } else {
            int x, y;
            int elapsed_turn = turn_ - start_turn;
            int quota_num = (problem_input_.T - start_turn) / 4;
            int now_quota = min(3, elapsed_turn / quota_num);
            if(now_quota == 0) {
                y = 0;
                x = 47 - (quota_num * (now_quota + 1) - elapsed_turn) / (double)quota_num * 47.0;
            } else if(now_quota == 1) {
                y = 47 - (quota_num * (now_quota + 1) - elapsed_turn) / (double)quota_num * 47.0;
                x = 47;
            } else if(now_quota == 2) {
                y = 47;
                x = (quota_num * (now_quota + 1) - elapsed_turn) / (double)quota_num * 47.0;
            } else {
                y = (quota_num * (now_quota + 1) - elapsed_turn) / (double)quota_num * 47.0;
                x = 0;
            }
            // cerr << "getBaseCoor(: " << now_quota << ") " << "x, y: " << x << ", " << y << endl;
            return {y, x};
        }
    }

    pii getBaseCoor2() {
        int start_turn = problem_input_.T - (problem_input_.W - 2) * (problem_input_.H - 2);
        if(turn_ < start_turn) {
            int elapsed_turn = turn_;
            double diff = (double)elapsed_turn / start_turn * 20.0;
            int y = 24 - diff;
            int x = 24 - diff;
            return {y, x};
        } else {
            int x, y;
            int elapsed_turn = turn_ - start_turn;
            int quota_num = (problem_input_.T - start_turn) / 4;
            int now_quota = min(3, elapsed_turn / quota_num);
            if(now_quota == 0) {
                y = 4;
                x = 47 - (quota_num * (now_quota + 1) - elapsed_turn) / (double)quota_num * 40.0;
            } else if(now_quota == 1) {
                y = 47 - (quota_num * (now_quota + 1) - elapsed_turn) / (double)quota_num * 40.0;
                x = 43;
            } else if(now_quota == 2) {
                y = 43;
                x = 4 + (quota_num * (now_quota + 1) - elapsed_turn) / (double)quota_num * 40.0;
            } else {
                y = 4 + (quota_num * (now_quota + 1) - elapsed_turn) / (double)quota_num * 40.0;
                x = 4;
            }
            // cerr << "getBaseCoor(: " << now_quota << ") " << "x, y: " << x << ", " << y << endl;
            return {y, x};
        }
    }

    ScoreType evalWeightedCoinScore(int x, int y) {
        if(turn_ >= 2000) {
            return 1.0;
        }
        int base_y, base_x;
        tie(base_y, base_x) = getBaseCoor();
        int dist = max(abs(y - base_y), abs(x - base_x));
        int MAX_DIST = problem_input_.H + problem_input_.W - 4;
        // ScoreType weight_score = (double)(MAX_DIST - dist) / MAX_DIST * 100.0;
        ScoreType weight_score = 50.0 / (dist + 1);
        return weight_score;
    }

    State(ProblemInput problem_input) {
        score_ = 0.0;
        coin_ = 0;
        trap_ = 0;
        turn_ = 0;
        hash_ = ZobristHash();
        problem_input_ = problem_input;

        player_positions_.resize(problem_input_.K);
        for(int i : views::iota(0, problem_input_.K)) {
            for(int h : views::iota(0, problem_input_.H)) {
                for(int w : views::iota(0, problem_input_.W)) {
                    if(problem_input_.map[i][h][w] == '@') {
                        player_positions_[i] = {h, w};
                    }
                }
            }
        }

        vector<uint64_t> board;
        for(int i : views::iota(0, problem_input_.K)) {
            for(int h : views::iota(0, problem_input_.H)) {
                for(int w : views::iota(0, problem_input_.W)) {
                    board.emplace_back(to_piece(w, h, i, problem_input_.map[i][h][w]));
                }
            }
        }
        hash_.initHash(board);

        move_history = Stack{nullptr};
    }

    ScoreType getScore() const {
        return scoreFormula(score_, trap_, turn_, problem_input_.T);
    }

    bool isEnd() const {
        return problem_input_.T == turn_;
    }
    ull getHash() const {
        return hash_.getHash();
    }
    pair<ScoreType, ull> moveHelper(const Operation &op, bool update) {
        ScoreType new_score = score_;
        int new_coin = coin_;
        int new_trap = trap_;
        ZobristHash new_hash = hash_;

        for(int i : views::iota(0, problem_input_.K)) {
            auto &now_map = problem_input_.map[i];
            int x = player_positions_[i].second;
            int y = player_positions_[i].first;
            int new_x = x;
            int new_y = y;

            if(now_map[y][x] != 'z') {
                if(op.direction == 'U' && 0 < y && now_map[y - 1][x] != '#') {
                    new_y = y - 1;
                } else if(op.direction == 'D' && y < problem_input_.H - 1 && now_map[y + 1][x] != '#') {
                    new_y = y + 1;
                } else if(op.direction == 'L' && 0 < x && now_map[y][x - 1] != '#') {
                    new_x = x - 1;
                } else if(op.direction == 'R' && x < problem_input_.W - 1 && now_map[y][x + 1] != '#') {
                    new_x = x + 1;
                }
            }

            if(x != new_x || y != new_y) {
                char next_char;
                if(now_map[new_y][new_x] == 'o' || now_map[new_y][new_x] == ' ') {
                    next_char = '@';
                } else if(now_map[new_y][new_x] == 'x') {
                    next_char = 'z';
                } else {
                    assert(false);
                }

                int pre_map_old_piece = to_piece(x, y, i, now_map[y][x]);
                int pre_map_new_piece = to_piece(x, y, i, ' ');
                new_hash.updateHash(pre_map_old_piece, pre_map_new_piece);

                int old_piece = to_piece(new_x, new_y, i, now_map[new_y][new_x]);
                int new_piece = to_piece(new_x, new_y, i, next_char);
                new_hash.updateHash(new_piece, old_piece);

                if(now_map[new_y][new_x] == 'o') {
                    new_score += evalWeightedCoinScore(new_x, new_y);
                    new_coin += 1;
                }
                if(next_char == 'z') {
                    new_trap += 1;
                }

                if(update) {
                    now_map[y][x] = ' ';
                    now_map[new_y][new_x] = next_char;
                    player_positions_[i] = {new_y, new_x};
                }
            }
        }

        if(update) {
            hash_ = new_hash;
            score_ = new_score;
            coin_ = new_coin;
            trap_ = new_trap;
            turn_ += 1;
        }

        ScoreType s = scoreFormula(new_score, new_trap, turn_ + 1, problem_input_.T);
        return {s, new_hash.getHash()};
    }

    pair<ScoreType, ull> tryMove(const Operation &op) {
        return moveHelper(op, false);
    }

    void applyMove(const Operation &op) {
        moveHelper(op, true);
    }
    vector<Operation> getValidOperations() const {
        return {Operation{'U'}, Operation{'D'}, Operation{'L'}, Operation{'R'}};
    }

    string to_string() const {
        string ret;
        for(int map_num : views::iota(0, problem_input_.K)) {
            for(int h : views::iota(0, problem_input_.H)) {
                string s = string(problem_input_.map[map_num][h].begin(), problem_input_.map[map_num][h].end());
                ret += s + "\n";
            }
            ret += "-----\n";
        }
        ret += "score: " + std::to_string(score_) + "\n";
        ret += "coin: " + std::to_string(coin_) + "\n";
        ret += "turn: " + std::to_string(turn_) + "\n";
        ret += "hash: " + std::to_string(hash_.getHash());
        return ret;
    }
};

struct TempNode {
    ScoreType score;
    uint64_t hash;
    int node_index;
    uint64_t rand_value;
    Operation op;

    TempNode(int score, uint64_t hash, int node_index, Operation &op)
        : score(score), hash(hash), node_index(node_index), op(op) {
        rand_value = x64rng.next();
    }
};

State BeamSearch(State &init_state, const int max_depth, const int beam_width, const ProblemInput &problem_input) {
    int MAX_TIME = 38500;
    TimeKeeper time_keeper(MAX_TIME);

    vector<State> states, next_states, end_states;
    states.emplace_back(init_state);
    states.back().move_history = Stack{nullptr};

    vector<TempNode> temp_nodes;    // スコア比較用の仮ノードを保管
    unordered_set<uint64_t> fields; // 重複除去用

    int apply_beam_width = beam_width;
    int total_beams = 0;
    for(int turn = 0; turn < max_depth; turn++) {
        if(states.empty()) {
            break;
        }

        temp_nodes.clear();
        fields.clear();

        for(const int i : views::iota(0, (int)states.size())) {
            // cerr << states[i].to_string() << endl;

            if(states[i].isEnd()) {
                end_states.emplace_back(states[i]);
                continue;
            }

            for(auto &op : states[i].getValidOperations()) {
                auto [next_score, next_hash] = states[i].tryMove(op);
                temp_nodes.emplace_back(next_score, next_hash, i, op);

                // 必要なら重複除去
                if(fields.contains(temp_nodes.back().hash)) {
                    temp_nodes.pop_back();
                } else {
                    fields.insert(temp_nodes.back().hash);
                }
            }
        }

        // 候補がビーム幅より多いなら上位beam_width個を選ぶ
        int node_size = temp_nodes.size();
        if(node_size > apply_beam_width) {
            nth_element(temp_nodes.begin(), temp_nodes.begin() + apply_beam_width, temp_nodes.end(),
                        [](TempNode &n1, TempNode &n2) {
                            if(n1.score == n2.score) {
                                return n1.rand_value < n2.rand_value;
                            } else {
                                return n1.score > n2.score;
                            }
                        });
            temp_nodes = vector<TempNode>(temp_nodes.begin(), temp_nodes.begin() + apply_beam_width);
        }

        // 仮ノードの情報から実際にノードを更新する
        for(const auto &temp_node : temp_nodes) {
            int index = temp_node.node_index;
            next_states.emplace_back(states[index]);
            next_states.back().applyMove(temp_node.op);
            next_states.back().move_history = states[index].move_history.push(temp_node.op);
        }

        swap(states, next_states);
        next_states.clear();
        total_beams += apply_beam_width;

        double elapsed_time = time_keeper.getElapsedTime();

        if(turn % 50 == 0) {
            cerr << "turn: " << turn << " / " << max_depth
                 << " score: " << states[0].getScore()
                 << " coin: " << states[0].coin_
                 << " elapsed_time: " << elapsed_time
                 << " beam_width: " << apply_beam_width << endl;
        }

        // if((turn + 1) % 50 == 0) {
        //     double obj_time = double(MAX_TIME) / problem_input.T * (turn + 1);
        //     double beam_speed = total_beams / elapsed_time;
        //     double obj_beam = beam_speed * obj_time;
        //     int diff_beam = min(50, (int)abs(obj_beam - total_beams));
        //     if(total_beams < obj_beam) {
        //         apply_beam_width = min(1300, apply_beam_width + diff_beam);
        //     } else {
        //         apply_beam_width = max(500, apply_beam_width - diff_beam);
        //     }
        // }
    }

    int arg_best = -1;
    ScoreType best_score = -1.0;
    for(int i : views::iota(0, (int)end_states.size())) {
        if(end_states[i].getScore() > best_score) {
            arg_best = i;
            best_score = end_states[i].getScore();
        }
    }

    assert(arg_best != -1);
    return end_states[arg_best];
}

void ProblemOutput(State best_state, vector<int> choice_k) {
    for(const int k : views::iota(0, (int)choice_k.size())) {
        if(k == (int)choice_k.size() - 1) {
            cout << choice_k[k];
        } else {
            cout << choice_k[k] << " ";
        }
    }
    cout << endl;

    auto best_history = best_state.move_history;
    vector<char> best_moves;
    while(best_history.head != nullptr) {
        best_moves.push_back(best_history.top().direction);
        best_history = best_history.pop();
    }
    reverse(best_moves.begin(), best_moves.end());
    for(const auto &move : best_moves) {
        cout << move;
    }
    cout << endl;

    cerr << "score: " << best_state.getScore() << endl;
    cerr << "coin: " << best_state.coin_ << endl;
    // cerr << best_state.to_string() << endl;
}

vector<int> ChoiceKmanyCoins(const ProblemInput &input) {
    vector<pii> coin_nums;
    for(int i : views::iota(0, input.N)) {
        int coin_num = 0;
        for(int h : views::iota(0, input.H)) {
            for(int w : views::iota(0, input.W)) {
                if(input.map[i][h][w] == 'o') {
                    coin_num++;
                }
            }
        }
        coin_nums.emplace_back(coin_num, i);
    }
    sort(coin_nums.rbegin(), coin_nums.rend());

    vector<int> choice_k;
    for(const int i : views::iota(0, input.K)) {
        choice_k.push_back(coin_nums[i].second);
        cerr << coin_nums[i].first << " ";
    }
    cerr << endl;

    return choice_k;
}
vector<int> ChoiceKnearStart(const ProblemInput &input) {
    vector<pii> coin_nums;
    for(int i : views::iota(0, input.N)) {
        int coin_num = 0;
        for(int h : views::iota(0, input.H)) {
            for(int w : views::iota(0, input.W)) {
                if(input.map[i][h][w] == 'o') {
                    coin_num++;
                }
            }
        }
        coin_nums.emplace_back(coin_num, i);
    }
    sort(coin_nums.rbegin(), coin_nums.rend());

    vector<pii> player_positions(input.N);
    for(int i : views::iota(0, input.N)) {
        for(int h : views::iota(0, input.H)) {
            for(int w : views::iota(0, input.W)) {
                if(input.map[i][h][w] == '@') {
                    player_positions[i] = {h, w};
                }
            }
        }
    }
    vector<vector<pii>> player_position_dists(input.N);
    for(int i : views::iota(0, input.N)) {
        for(int j : views::iota(0, input.N)) {
            if(i == j)
                continue;
            int dist = abs(player_positions[i].first - player_positions[j].first) +
                       abs(player_positions[i].second - player_positions[j].second);
            player_position_dists[i].push_back({dist, j});
        }
    }
    for(int i : views::iota(0, input.N)) {
        sort(player_position_dists[i].begin(), player_position_dists[i].end(), [](pii &a, pii &b) {
            return a.first < b.first;
        });
    }
    int min_k = 1e9;
    vector<pii> target_dist_inds;
    for(int i : views::iota(0, input.N)) {
        int now_dist = player_position_dists[i][input.K].first;
        if(min_k > now_dist) {
            min_k = now_dist;
            target_dist_inds = player_position_dists[i];
        }
    }
    cerr << "min_k: " << min_k << endl;

    vector<int> target_inds;
    for(const auto &pair : target_dist_inds) {
        target_inds.push_back(pair.second);
    }
    vector<int> choice_k;
    for(int i : views::iota(0, input.K)) {
        choice_k.push_back(target_inds[i]);
    }

    for(int i : views::iota(0, input.K)) {
        cerr << choice_k[i] << " ";
    }
    cerr << endl;

    return choice_k;
}

vector<int> ChoiceKgrid(ProblemInput &input) {
    vector<pii> coin_nums;
    for(int i : views::iota(0, input.N)) {
        int coin_num = 0;
        for(int h : views::iota(0, input.H)) {
            for(int w : views::iota(0, input.W)) {
                if(input.map[i][h][w] == 'o') {
                    coin_num++;
                }
            }
        }
        coin_nums.emplace_back(coin_num, i);
    }
    sort(coin_nums.rbegin(), coin_nums.rend());

    unordered_map<string, vector<int>> map_div;
    vector<pii> player_positions(input.N);
    for(int i : views::iota(0, input.N)) {
        for(int h : views::iota(0, input.H)) {
            for(int w : views::iota(0, input.W)) {
                if(input.map[i][h][w] == '@') {
                    player_positions[i] = {h, w};
                    int yy = (h + 1) % 4; // 4分割
                    int xx = (w + 1) % 3; // 3分割
                    string key = to_string(yy) + "_" + to_string(xx);
                    if(map_div.find(key) == map_div.end()) {
                        map_div[key] = vector<int>();
                    }
                    map_div[key].push_back(i);
                }
            }
        }
    }

    vector<int> target_inds;
    for(const auto &pair : map_div) {
        cerr << pair.first << " : " << pair.second.size() << endl;
        if(!pair.second.empty() && (int)pair.second.size() >= input.K) {
            target_inds = pair.second;
            break;
        }
    }
    assert((int)target_inds.size() >= input.K);
    sort(target_inds.begin(), target_inds.end(), [&](int a, int b) {
        return coin_nums[a].first > coin_nums[b].first;
    });

    vector<int> choice_k;
    for(int i : views::iota(0, input.K)) {
        choice_k.push_back(target_inds[i]);
    }
    return choice_k;
}

vector<int> RandomChoice(const ProblemInput &input) {
    vector<int> all_ind;
    for(int i : views::iota(0, input.N)) {
        all_ind.push_back(i);
    }
    x32rng.shuffle(all_ind);

    vector<int> choice_k;
    for(int i : views::iota(0, input.K)) {
        choice_k.push_back(all_ind[i]);
    }
    return choice_k;
}

void solve() {
    auto input = ReadInput();
    auto choice_k = ChoiceKnearStart(input);

    auto target_input = input;
    vector<vector<vector<char>>> new_map(input.K);
    for(int i : views::iota(0, input.K)) {
        cerr << choice_k[i] << " ";
        new_map[i] = input.map[choice_k[i]];
    }
    cerr << endl;

    target_input.map = new_map;

    State init_state = State(target_input);

    const int BEAM_WIDTH = 1000;
    auto best_state = BeamSearch(init_state, input.T + 2, BEAM_WIDTH, target_input);

    ProblemOutput(best_state, choice_k);
}

int main() {
    // Init ZobristHash
    for(int i : views::iota(0, MAX_X)) {
        for(int j : views::iota(0, MAX_Y)) {
            for(int k : views::iota(0, MAX_K)) {
                for(int p : views::iota(0, MAX_P)) {
                    piece_keys[i][j][k][p] = x128rng.next();
                }
            }
        }
    }

    solve();
    return 0;
}
