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
                                return x32rng.rand() < 0.5 ? true : false;
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
    ZobristHash zobrist_hash(100);
    vector<int> board(100);
    for(int i = 0; i < 100; ++i) {
        board[i] = i;
    }
    zobrist_hash.initHash(board);
    cout << "Initial Hash: " << zobrist_hash.getHash() << endl;

    zobrist_hash.updateHash(99, 0);
    cout << "Updated Hash: " << zobrist_hash.getHash() << endl;

    zobrist_hash.updateHash(50, 15);
    cout << "Updated Hash: " << zobrist_hash.getHash() << endl;

    zobrist_hash.updateHash(15, 50);
    cout << "Updated Hash: " << zobrist_hash.getHash() << endl;

    zobrist_hash.updateHash(0, 99);
    cout << "Updated Hash: " << zobrist_hash.getHash() << endl;
}

int main() {
    solve();
    return 0;
}
