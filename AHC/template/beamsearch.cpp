#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using ull = unsigned long long;
using pii = pair<int, int>;
using pll = pair<ll, ll>;

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
