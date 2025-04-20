// Copyright [2022] <Copyright Eita Aoki (Thunder) >
#include <string>
#include <vector>
#include <array>
#include <sstream>
#include <utility>
#include <random>
#include <assert.h>
#include <math.h>
#include <chrono>
#include <queue>
#include <algorithm>
std::random_device rnd;
std::mt19937 mt_for_action(0);

constexpr const int H = 5;   // 迷路の高さ
constexpr const int W = 5;   // 迷路の幅
constexpr int END_TURN = 20; // ゲーム終了ターン

using ScoreType = int64_t;
constexpr const ScoreType INF = 1000000000LL;

static const std::string dstr[4] = {"RIGHT", "LEFT", "DOWN", "UP"};

enum WinningStatus
{
    FIRST,  // プレイヤー0が勝った
    SECOND, // プレイヤー1が勝った
    DRAW,
    NONE,
};

// 同時二人ゲームの例
// 1ターンに上下左右四方向のいずれかに壁のない場所に1マスずつ進む。
// 床にあるポイントを踏むと自身のスコアとなり、床のポイントが消える。
// 勝利条件はEND_TURNの時点のスコアが敵より高いこと
class SimultaneousMazeState
{
private:
public:
    // AlternateMazeStateのコンストラクタで参照するために全メンバをpublicにする
    static constexpr const int dx[4] = {1, -1, 0, 0};
    static constexpr const int dy[4] = {0, 0, 1, -1};
    struct Character
    {
        int y_;
        int x_;
        int game_score_;
        Character(const int y = 0, const int x = 0) : y_(y), x_(x), game_score_(0) {}
    };
    std::vector<std::vector<int>> points_; // 床のポイントを1~9で表現する
    int turn_;                             // 現在のターン
    std::vector<Character> characters_;

    SimultaneousMazeState(const int seed) : points_(H, std::vector<int>(W)),
                                            turn_(0),
                                            characters_({Character(H / 2, (W / 2) - 1), Character(H / 2, (W / 2) + 1)})
    {
        auto mt_for_construct = std::mt19937(seed);

        for (int y = 0; y < H; y++)
            for (int x = 0; x < W / 2 + 1; x++)
            {
                int ty = y;
                int tx = x;
                int point = mt_for_construct() % 10;
                if (characters_[0].y_ == y && characters_[0].x_ == x)
                {
                    continue;
                }
                if (characters_[1].y_ == y && characters_[1].x_ == x)
                {
                    continue;
                }
                this->points_[ty][tx] = point;
                tx = W - 1 - x;
                this->points_[ty][tx] = point;
            }
    }

    // [どのゲームでも実装する] : 勝敗情報を取得する
    WinningStatus getWinningStatus() const
    {
        if (isDone())
        {
            if (characters_[0].game_score_ > characters_[1].game_score_)
                return WinningStatus::FIRST;
            else if (characters_[0].game_score_ < characters_[1].game_score_)
                return WinningStatus::SECOND;
            else
                return WinningStatus::DRAW;
        }
        else
        {
            return WinningStatus::NONE;
        }
    }

    // [どのゲームでも実装する] : ゲームが終了したか判定する
    bool isDone() const
    {
        return this->turn_ == END_TURN;
    }
    // [どのゲームでも実装する] : 指定したactionでゲームを1ターン進める
    void advance(const int action0, const int action1)
    {
        {
            auto &character = this->characters_[0];
            const auto &action = action0;
            character.x_ += dx[action];
            character.y_ += dy[action];
            const auto point = this->points_[character.y_][character.x_];
            if (point > 0)
            {
                character.game_score_ += point;
            }
        }
        {
            auto &character = this->characters_[1];
            const auto &action = action1;
            character.x_ += dx[action];
            character.y_ += dy[action];
            const auto point = this->points_[character.y_][character.x_];
            if (point > 0)
            {
                character.game_score_ += point;
            }
        }

        for (const auto &character : this->characters_)
        {
            this->points_[character.y_][character.x_] = 0;
        }
        this->turn_++;
    }

    // [どのゲームでも実装する] : 指定したプレイヤーが可能な行動を全て取得する
    std::vector<int> legalActions(const int player_id) const
    {
        std::vector<int> actions;
        const auto &character = this->characters_[player_id];
        for (int action = 0; action < 4; action++)
        {
            int ty = character.y_ + dy[action];
            int tx = character.x_ + dx[action];
            if (ty >= 0 && ty < H && tx >= 0 && tx < W)
            {
                actions.emplace_back(action);
            }
        }
        return actions;
    }

    // [実装しなくてもよいが実装すると便利] : プレイヤー0の勝率計算のためのスコアを計算する
    double getFirstPlayerScoreForWinRate() const
    {
        switch (this->getWinningStatus())
        {
        case (WinningStatus::FIRST):
            return 1.;
        case (WinningStatus::SECOND):
            return 0.;
        default:
            return 0.5;
        }
    }

    // [実装しなくてもよいが実装すると便利] : 現在のゲーム状況を文字列にする
    std::string toString() const
    {
        std::stringstream ss("");
        ss << "turn:\t" << this->turn_ << "\n";
        for (int player_id = 0; player_id < this->characters_.size(); player_id++)
        {
            ss << "score(" << player_id << "):\t" << this->characters_[player_id].game_score_ << "\n";
        }
        for (int h = 0; h < H; h++)
        {
            for (int w = 0; w < W; w++)
            {
                bool is_written = false; // この座標に書く文字が決定したか
                for (int player_id = 0; player_id < this->characters_.size(); player_id++)
                {
                    const auto &character = this->characters_[player_id];
                    if (character.y_ == h && character.x_ == w)
                    {
                        if (player_id == 0)
                        {
                            ss << 'A';
                        }
                        else
                        {
                            ss << 'B';
                        }
                        is_written = true;
                    }
                }
                if (!is_written)
                {
                    if (this->points_[h][w] > 0)
                    {
                        ss << points_[h][w];
                    }
                    else
                    {
                        ss << '.';
                    }
                }
            }
            ss << '\n';
        }

        return ss.str();
    }
};
using State = SimultaneousMazeState;

// 指定したプレイヤーの行動をランダムに決定する
int randomAction(const State &state, const int player_id)
{
    auto legal_actions = state.legalActions(player_id);
    return legal_actions[mt_for_action() % (legal_actions.size())];
}

class AlternateMazeState
{
private:
    static constexpr const int END_TURN_ = END_TURN * 2; // 同時着手の1ターンは交互着手では2ターン分
    static constexpr const int dx[4] = {1, -1, 0, 0};
    static constexpr const int dy[4] = {0, 0, 1, -1};
    std::vector<std::vector<int>> points_; // 床のポイントを1~9で表現する
    int turn_;                             // 現在のターン
    using Character = SimultaneousMazeState::Character;
    std::vector<Character> characters_;

public:
    AlternateMazeState(const SimultaneousMazeState &base_state, const int player_id) : points_(base_state.points_),
                                                                                       turn_(base_state.turn_ * 2), // 同時着手の1ターンは交互着手では2ターン分
                                                                                       characters_(player_id == 0 ? base_state.characters_ : std::vector<Character>{base_state.characters_[1], base_state.characters_[0]})
    {
    }

    // [どのゲームでも実装する] : 勝敗情報を取得する
    WinningStatus getWinningStatus() const
    {
        if (isDone())
        {
            if (characters_[0].game_score_ > characters_[1].game_score_)
                return WinningStatus::FIRST; // WIN
            else if (characters_[0].game_score_ < characters_[1].game_score_)
                return WinningStatus::SECOND; // LOSE
            else
                return WinningStatus::DRAW;
        }
        else
        {
            return WinningStatus::NONE;
        }
    }

    // [どのゲームでも実装する] : ゲームが終了したか判定する
    bool isDone() const
    {
        return this->turn_ == END_TURN_;
    }

    // [どのゲームでも実装する] : 指定したactionでゲームを1ターン進め、次のプレイヤー視点の盤面にする
    void advance(const int action)
    {
        auto &character = this->characters_[0];
        character.x_ += dx[action];
        character.y_ += dy[action];
        auto &point = this->points_[character.y_][character.x_];
        if (point > 0)
        {
            character.game_score_ += point;
            point = 0;
        }
        this->turn_++;
        std::swap(this->characters_[0], this->characters_[1]);
    }

    // [どのゲームでも実装する] : 現在のプレイヤーが可能な行動を全て取得する
    std::vector<int> legalActions() const
    {
        std::vector<int> actions;
        constexpr const int player_id = 0;
        const auto &character = this->characters_[player_id];
        for (int action = 0; action < 4; action++)
        {
            int ty = character.y_ + dy[action];
            int tx = character.x_ + dx[action];
            if (ty >= 0 && ty < H && tx >= 0 && tx < W)
            {
                actions.emplace_back(action);
            }
        }
        return actions;
    }
};
using AlternateState = AlternateMazeState;

namespace altanate_motecalo
{
    // ランダムに行動を決定する
    int randomAction(const AlternateState &state)
    {
        auto legal_actions = state.legalActions();
        return legal_actions[mt_for_action() % (legal_actions.size())];
    }
    // ランダムプレイアウトをして勝敗スコアを計算する
    double playout(AlternateState *state)
    { // const&にすると再帰中にディープコピーが必要になるため、高速化のためポインタにする。(constでない参照でも可)
        switch (state->getWinningStatus())
        {
        case (WinningStatus::FIRST): // WIN
            return 1.;
        case (WinningStatus::SECOND): // LOSE
            return 0.;
        case (WinningStatus::DRAW):
            return 0.5;
        default:
            state->advance(randomAction(*state));
            return 1. - playout(state);
        }
    }
    constexpr const double C = 1.;             // UCB1の計算に使う定数
    constexpr const int EXPAND_THRESHOLD = 10; // ノードを展開する閾値

    // MCTSの計算に使うノード
    class Node
    {
    private:
        AlternateState state_;
        double w_;

    public:
        std::vector<Node> child_nodes;
        double n_;

        // ノードの評価を行う
        double evaluate()
        {
            if (this->state_.isDone())
            {
                double value = 0.5;
                switch (this->state_.getWinningStatus())
                {
                case (WinningStatus::FIRST):
                    value = 1.;
                    break;
                case (WinningStatus::SECOND):
                    value = 0.;
                    break;
                default:
                    break;
                }
                this->w_ += value;
                ++this->n_;
                return value;
            }
            if (this->child_nodes.empty())
            {
                AlternateState state_copy = this->state_;
                double value = playout(&state_copy);
                this->w_ += value;
                ++this->n_;

                if (this->n_ == EXPAND_THRESHOLD)
                    this->expand();

                return value;
            }
            else
            {
                double value = 1. - this->nextChildNode().evaluate();
                this->w_ += value;
                ++this->n_;
                return value;
            }
        }

        // ノードを展開する
        void expand()
        {
            auto legal_actions = this->state_.legalActions();
            this->child_nodes.clear();
            for (const auto action : legal_actions)
            {
                this->child_nodes.emplace_back(this->state_);
                this->child_nodes.back().state_.advance(action);
            }
        }

        // どのノードを評価するか選択する
        Node &nextChildNode()
        {
            for (auto &child_node : this->child_nodes)
            {
                if (child_node.n_ == 0)
                    return child_node;
            }
            double t = 0;
            for (const auto &child_node : this->child_nodes)
            {
                t += child_node.n_;
            }
            double best_value = -INF;
            int best_action_index = -1;
            for (int i = 0; i < this->child_nodes.size(); i++)
            {
                const auto &child_node = this->child_nodes[i];
                double ucb1_value = 1. - child_node.w_ / child_node.n_ + (double)C * std::sqrt(2. * std::log(t) / child_node.n_);
                if (ucb1_value > best_value)
                {
                    best_action_index = i;
                    best_value = ucb1_value;
                }
            }
            return this->child_nodes[best_action_index];
        }

        Node(const AlternateState &state) : state_(state), w_(0), n_(0) {}
    };

    // プレイアウト数を指定してMCTSで行動を決定する
    int mctsAction(const State &base_state, const int player_id, const int playout_number)
    {
        auto state = AlternateState(base_state, player_id);
        Node root_node = Node(state);
        root_node.expand();
        for (int i = 0; i < playout_number; i++)
        {
            root_node.evaluate();
        }
        auto legal_actions = state.legalActions();

        int best_action_searched_number = -1;
        int best_action_index = -1;
        assert(legal_actions.size() == root_node.child_nodes.size());
        for (int i = 0; i < legal_actions.size(); i++)
        {
            int n = root_node.child_nodes[i].n_;
            if (n > best_action_searched_number)
            {
                best_action_index = i;
                best_action_searched_number = n;
            }
        }
        return legal_actions[best_action_index];
    }

}
using altanate_motecalo::mctsAction;

namespace montecarlo
{
    // プレイヤー0視点での評価
    double playout(State *state)
    { // const&にすると再帰中にディープコピーが必要になるため、高速化のためポインタにする。(constでない参照でも可)
        switch (state->getWinningStatus())
        {
        case (WinningStatus::FIRST):
            return 1.;
        case (WinningStatus::SECOND):
            return 0.;
        case (WinningStatus::DRAW):
            return 0.5;
        default:
            state->advance(randomAction(*state, 0), randomAction(*state, 1));
            return playout(state);
        }
    }
    // プレイアウト数を指定して原始モンテカルロ法で指定したプレイヤーの行動を決定する
    int primitiveMontecarloAction(const State &state, const int player_id, const int playout_number)
    {
        auto my_legal_actions = state.legalActions(player_id);
        auto opp_legal_actions = state.legalActions((player_id + 1) % 2);
        double best_value = -INF;
        int best_action_index = -1;
        for (int i = 0; i < my_legal_actions.size(); i++)
        {
            double value = 0;
            for (int j = 0; j < playout_number; j++)
            {
                State next_state = state;
                if (player_id == 0)
                {
                    next_state.advance(my_legal_actions[i], opp_legal_actions[mt_for_action() % opp_legal_actions.size()]);
                }
                else
                {
                    next_state.advance(opp_legal_actions[mt_for_action() % opp_legal_actions.size()], my_legal_actions[i]);
                }
                double player0_win_rate = playout(&next_state);
                double win_rate = (player_id == 0 ? player0_win_rate : 1. - player0_win_rate);
                value += win_rate;
            }
            if (value > best_value)
            {
                best_action_index = i;
                best_value = value;
            }
        }
        return my_legal_actions[best_action_index];
    }

    constexpr const double C = 1.;            // UCB1の計算に使う定数
    constexpr const int EXPAND_THRESHOLD = 5; // ノードを展開する閾値

    // DUCTの計算に使うノード
    class Node
    {
    private:
        State state_;
        double w_;

    public:
        std::vector<std::vector<Node>> child_nodeses_;
        double n_;

        // プレイヤー0視点でノードの評価を行う
        double evaluate()
        {
            if (this->state_.isDone())
            {
                double value = 0.5;
                switch (this->state_.getWinningStatus())
                {
                case (WinningStatus::FIRST):
                    value = 1.;
                    break;
                case (WinningStatus::SECOND):
                    value = 0.;
                    break;
                default:
                    break;
                }
                this->w_ += value;
                ++this->n_;
                return value;
            }
            if (this->child_nodeses_.empty())
            {
                State state_copy = this->state_;
                double value = playout(&state_copy);
                this->w_ += value;
                ++this->n_;

                if (this->n_ == EXPAND_THRESHOLD)
                    this->expand();

                return value;
            }
            else
            {
                double value = this->nextChildNode().evaluate();
                this->w_ += value;
                ++this->n_;
                return value;
            }
        }
        // ノードを展開する
        void expand()
        {
            auto legal_actions0 = this->state_.legalActions(0);
            auto legal_actions1 = this->state_.legalActions(1);
            this->child_nodeses_.clear();
            for (const auto &action0 : legal_actions0)
            {
                this->child_nodeses_.emplace_back();
                auto &target_nodes = this->child_nodeses_.back();
                for (const auto &action1 : legal_actions1)
                {
                    target_nodes.emplace_back(this->state_);
                    auto &target_node = target_nodes.back();
                    target_node.state_.advance(action0, action1);
                }
            }
        }
        // どのノードを評価するか選択する
        Node &nextChildNode()
        {
            for (auto &child_nodes : this->child_nodeses_)
            {
                for (auto &child_node : child_nodes)
                {
                    if (child_node.n_ == 0)
                        return child_node;
                }
            }
            double t = 0;
            for (auto &child_nodes : this->child_nodeses_)
            {
                for (auto &child_node : child_nodes)
                {
                    t += child_node.n_;
                }
            }
            int best_is[] = {-1, -1};

            // プレイヤー0側の行動選択(思考するプレイヤーがどちらかに関係なくここではプレイヤー0側の行動)
            double best_value = -INF;
            for (int i = 0; i < this->child_nodeses_.size(); i++)
            {
                const auto &childe_nodes = this->child_nodeses_[i];
                double w = 0;
                double n = 0;
                for (int j = 0; j < childe_nodes.size(); j++)
                {
                    const auto &child_node = childe_nodes[j];
                    w += child_node.w_;
                    n += child_node.n_;
                }

                double ucb1_value = w / n + (double)C * std::sqrt(2. * std::log(t) / n);
                if (ucb1_value > best_value)
                {
                    best_is[0] = i;
                    best_value = ucb1_value;
                }
            }
            // プレイヤー1側の行動選択(思考するプレイヤーがどちらかに関係なくここではプレイヤー1側の行動)
            best_value = -INF;
            for (int j = 0; j < this->child_nodeses_[0].size(); j++)
            {
                double w = 0;
                double n = 0;
                for (int i = 0; i < this->child_nodeses_.size(); i++)
                {
                    const auto &child_node = child_nodeses_[i][j];
                    w += child_node.w_;
                    n += child_node.n_;
                }
                // 敵側の行動選択フェーズなので、ここは評価を反転する必要がある
                double ucb1_value = 1. - w / n + (double)C * std::sqrt(2. * std::log(t) / n);
                if (ucb1_value > best_value)
                {
                    best_is[1] = j;
                    best_value = ucb1_value;
                }
            }

            return this->child_nodeses_[best_is[0]][best_is[1]];
        }

        Node(const State &state) : state_(state), w_(0), n_(0) {}
    };

    // プレイアウト数を指定してDUCTで指定したプレイヤーの行動を決定する
    int ductAction(const State &state, const int player_id, const int playout_number)
    {
        Node root_node = Node(state);
        root_node.expand();
        for (int i = 0; i < playout_number; i++)
        {
            root_node.evaluate();
        }
        auto legal_actions = state.legalActions(player_id);
        int i_size = root_node.child_nodeses_.size();
        int j_size = root_node.child_nodeses_[0].size();

        if (player_id == 0)
        {
            int best_action_searched_number = -1;
            int best_action_index = -1;
            for (int i = 0; i < i_size; i++)
            {
                int n = 0;
                for (int j = 0; j < j_size; j++)
                {
                    n += root_node.child_nodeses_[i][j].n_;
                }
                if (n > best_action_searched_number)
                {
                    best_action_index = i;
                    best_action_searched_number = n;
                }
            }
            return legal_actions[best_action_index];
        }
        else
        {
            int best_action_searched_number = -1;
            int best_j = -1;
            for (int j = 0; j < j_size; j++)
            {
                int n = 0;
                for (int i = 0; i < i_size; i++)
                {
                    n += root_node.child_nodeses_[i][j].n_;
                }
                if (n > best_action_searched_number)
                {
                    best_j = j;
                    best_action_searched_number = n;
                }
            }
            return legal_actions[best_j];
        }
    }
};
using ::montecarlo::ductAction;
using ::montecarlo::primitiveMontecarloAction;

#include <iostream>
#include <functional>

using AIFunction = std::function<int(const State &, const int)>;
using StringAIPair = std::pair<std::string, AIFunction>;

// ゲームをgame_number回プレイしてaisの0番目のAIの勝率を表示する。
void testFirstPlayerWinRate(std::array<StringAIPair, 2> &ais, const int game_number)
{
    using std::cout;
    using std::endl;
    std::mt19937 mt_for_construct(0);

    double first_player_win_rate = 0;
    for (int i = 0; i < game_number; i++)
    {
        auto state = State(mt_for_construct());
        auto &first_ai = ais[0];
        auto &second_ai = ais[1];
        while (true)
        {
            state.advance(first_ai.second(state, 0), second_ai.second(state, 1));
            if (state.isDone())
                break;
        }
        double win_rate_point = state.getFirstPlayerScoreForWinRate();
        if (win_rate_point >= 0)
        {
            state.toString();
        }
        first_player_win_rate += win_rate_point;

        cout << "i " << i << " w " << first_player_win_rate / (i + 1) << endl;
    }
    first_player_win_rate /= (double)game_number;
    cout << "Winning rate of " << ais[0].first << " to " << ais[1].first << ":\t" << first_player_win_rate << endl;
}

int main()
{
    auto ais = std::array<StringAIPair, 2>{
        StringAIPair("ductAction", [](const State &state, const int player_id)
                     { return ductAction(state, player_id, 1000); }),
        StringAIPair("mctsAction", [](const State &state, const int player_id)
                     { return mctsAction(state, player_id, 1000); }),
    };

    testFirstPlayerWinRate(ais, 500);

    return 0;
}