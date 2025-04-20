#include <bits/stdc++.h>
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

class Xorshift32 {
  private:
    uint32_t state;

    // ランダムなuint32_t整数を返す
    uint32_t next() {
        uint32_t x = state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state = x;
        return x;
    }

  public:
    explicit Xorshift32(uint32_t seed = 2525)
        : state(seed) {
    }

    // [0, max) の範囲でランダムな整数を返す
    int randint(int max) {
        return next() % max;
    }

    // [low, high] の整数を一様分布でランダムに返す
    int randint(int low, int high) {
        return low + next() % (high - low + 1);
    }

    // [0.0, 1.0) のランダムな小数を返す
    double rand() {
        return static_cast<double>(next()) / static_cast<double>(UINT32_MAX);
    }

    // 重みに応じてインデックスをサンプリング（離散分布サンプリング）
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
        return static_cast<int>(weights.size() - 1); // fallback
    }

    // イテレータ範囲から k 個ランダムサンプルを抽出（順序ランダム）
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

    // ベクトルの要素をランダムに並び替える（Fisher-Yates シャッフル）
    template <typename T>
    void shuffle(std::vector<T> &vec) {
        for(int i = static_cast<int>(vec.size()) - 1; i > 0; --i) {
            int j = randint(i + 1);
            std::swap(vec[i], vec[j]);
        }
    }
};
Xorshift32 xor_shift_rng;

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
        double r = xor_shift_rng.rand() * n;
        int i = static_cast<int>(r);
        if(r - i < p[i])
            return i;
        else
            return a[i];
    }
};

// =================================
// Simulated Annealing
// =================================

double exponential_schedule(double init, double obj, double elapsed_time, double max_time) {
    double lambda_param = log(obj / init) / max_time;
    return init * exp(lambda_param * elapsed_time);
}

double linear_schedule(double init, double obj, double elapsed_time, double max_time) {
    return init + (obj - init) * (elapsed_time / max_time);
}

bool acceptance_probability(double delta_cost, double temp) {
    return delta_cost < 0 || xor_shift_rng.rand() < exp(-delta_cost / temp) ? true : false;
}

template <typename T>
tuple<T, double> SimulatedAnnealing(
    const T &x0,
    double t0,
    double t1,
    double max_time,
    int display_interval = 10000) {

    T x = x0;
    T best_x = x;

    double current_cost = 0; // TODO
    double best_cost = current_cost;

    TimeKeeper time_keeper(max_time);

    int iteration = 0;
    for(iteration = 0;; iteration++) {
        int elapsed = time_keeper.getElapsedTime();
        if(elapsed >= max_time)
            break;

        double temp = linear_schedule(t0, t1, elapsed, max_time);

        T new_x = x;         // TODO
        double new_cost = 0; // TODO

        double delta_cost = new_cost - current_cost;
        if(delta_cost < 0 || xor_shift_rng.rand() < exp(-delta_cost / temp)) { // TODO Tempをexp内に含めると勾配が急になる（≒受理確率が下がる）
            current_cost = new_cost;
            x = new_x;
        }

        if(current_cost < best_cost) {
            best_x = x;
            best_cost = current_cost;
        }

        iteration++;
        if(iteration % display_interval == 0) {
            cerr << "Iteration: " + to_string(iteration) +
                        ", Current cost: " + to_string(current_cost) +
                        ", Best cost: " + to_string(best_cost) +
                        ", Temp: " + to_string(temp)
                 << endl;
        }
    }

    cerr << "Iteration: " + to_string(iteration) +
                ", Best cost: " + to_string(best_cost)
         << endl;

    return {best_x, best_cost};
}

// =================================
// Beam Search
// =================================
struct Operation {
    int x;
    int y;
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

struct State {
    Stack move_history;

    State() {
    }
    int getScore() const {
        return 0;
    }
    bool isEnd() const {
        return false;
    }
    ull getHash() const {
        return 0;
    }
    pair<int, ull> tryMove(const Operation &op) const {
        return {0, 0};
    }
    void applyMove(const Operation &op) {
    }
    vector<Operation> getValidOperations() const {
        return {};
    }
};

// スコアだけ計算して上位を選ぶために用いる仮ノード
struct TempNode {
    int score;
    ull hash;
    int node_index;
    Operation op;

    TempNode(int score, ull hash, int node_index, Operation &op)
        : score(score), hash(hash), node_index(node_index), op(op) {
    }
};

State BeamSearch(State &init_state, const int max_depth, const int beam_width) {
    vector<State> states, next_states, end_states;
    states.emplace_back(init_state);
    states.back().move_history = Stack{nullptr};

    vector<TempNode> temp_nodes; // スコア比較用の仮ノードを保管
    unordered_set<ull> fields;   // 重複除去用

    for(int turn = 1; turn <= max_depth; turn++) {
        if(states.empty()) {
            break;
        }

        temp_nodes.clear();
        fields.clear();

        for(const int i : views::iota(0, (int)states.size())) {
            if(states[i].isEnd()) {
                end_states.emplace_back(states[i]);
                continue;
            }

            for(auto &op : states[i].getValidOperations()) {
                auto [next_score, next_hash] = states[i].tryMove(op);
                temp_nodes.emplace_back(next_score, next_hash, i, op);

                // 必要なら重複除去
                if(fields.count(temp_nodes.back().hash)) {
                    temp_nodes.pop_back();
                } else {
                    fields.insert(temp_nodes.back().hash);
                }
            }
        }

        // 候補がビーム幅より多いなら上位beam_width個を選ぶ
        int node_size = temp_nodes.size();
        if(node_size > beam_width) {
            nth_element(temp_nodes.begin(), temp_nodes.begin() + beam_width, temp_nodes.end(),
                        [](TempNode &n1, TempNode &n2) {
                            if(n1.score == n2.score) {
                                return xor_shift_rng.rand() < 0.5 ? true : false;
                            } else {
                                return n1.score > n2.score;
                            }
                        });
        }

        // 仮ノードの情報から実際にノードを更新する
        for(const auto &temp_node : temp_nodes) {
            int index = temp_node.node_index;
            next_states.emplace_back(states[index]);
            next_states.back().applyMove(temp_node.op);
            assert(next_states.back().getScore() == temp_node.score);
            assert(next_states.back().getHash() == temp_node.hash);
            next_states.back().move_history = states[index].move_history.push(temp_node.op);
        }

        swap(states, next_states);
        next_states.clear();
    }

    int arg_best = -1, best_score = 0;
    for(int i : views::iota(0, (int)states.size())) {
        if(end_states[i].getScore() > best_score) {
            arg_best = i;
            best_score = end_states[i].getScore();
        }
    }
    return end_states[arg_best];
}

// =================================
// テンプレートここまで
// =================================

void solve() {
    // ここに問題を解くコードを書く
}

int main() {
    solve();
    return 0;
}
