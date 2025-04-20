// Copyright [2022] <Copyright Eita Aoki (Thunder) >
#include <string>
#include <array>
#include <vector>
#include <sstream>
#include <utility>
#include <random>
#include <assert.h>
#include <math.h>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <functional>
#include <queue>
#include <set>
#pragma GCC diagnostic ignored "-Wsign-compare"
std::random_device rnd;
std::mt19937 mt_for_action(0);

// 時間を管理するクラス
class TimeKeeper
{
private:
    std::chrono::high_resolution_clock::time_point start_time_;
    int64_t time_threshold_;

public:
    // 時間制限をミリ秒単位で指定してインスタンスをつくる。
    TimeKeeper(const int64_t &time_threshold)
        : start_time_(std::chrono::high_resolution_clock::now()),
          time_threshold_(time_threshold)
    {
    }

    // インスタンス生成した時から指定した時間制限を超過したか判定する。
    bool isTimeOver() const
    {
        auto diff = std::chrono::high_resolution_clock::now() - this->start_time_;
        return std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() >= time_threshold_;
    }
};

constexpr const int H = 6; // 迷路の高さ
constexpr const int W = 7; // 迷路の幅

using ScoreType = int64_t;
constexpr const ScoreType INF = 1000000000LL;

enum WinningStatus
{
    WIN,
    LOSE,
    DRAW,
    NONE,
};

class ConnectFourState
{
private:
    static constexpr const int dx[2] = {1, -1};          // 移動方向のx成分
    static constexpr const int dy_right_up[2] = {1, -1}; // "／"方向のx成分
    static constexpr const int dy_leftt_up[2] = {-1, 1}; // "\"方向のx成分
    static constexpr const int dy[4] = {0, 0, 1, -1};    // 右、左、上、下への移動方向のy成分

    bool is_first_ = true; // 先手番であるか
    int my_board_[H][W] = {};
    int enemy_board_[H][W] = {};
    WinningStatus winning_status_ = WinningStatus::NONE;

public:
    ConnectFourState()
    {
    }

    // [どのゲームでも実装する] : ゲームが終了したか判定する
    bool isDone() const
    {
        return winning_status_ != WinningStatus::NONE;
    }

    // [どのゲームでも実装する] : 指定したactionでゲームを1ターン進め、次のプレイヤー視点の盤面にする
    void advance(const int action)
    {
        std::pair<int, int> coordinate;
        for (int y = 0; y < H; y++)
        {
            if (this->my_board_[y][action] == 0 && this->enemy_board_[y][action] == 0)
            {
                this->my_board_[y][action] = 1;
                coordinate = std::pair<int, int>(y, action);
                break;
            }
        }

        { // 横方向の連結判定

            auto que = std::deque<std::pair<int, int>>();
            que.emplace_back(coordinate);
            std::vector<std::vector<bool>> check(H, std::vector<bool>(W, false));
            int count = 0;
            while (!que.empty())
            {
                const auto &tmp_cod = que.front();
                que.pop_front();
                ++count;
                if (count >= 4)
                {
                    this->winning_status_ = WinningStatus::LOSE; // 自分の駒が揃ったら相手視点負け
                    break;
                }
                check[tmp_cod.first][tmp_cod.second] = true;

                for (int action = 0; action < 2; action++)
                {
                    int ty = tmp_cod.first;
                    int tx = tmp_cod.second + dx[action];

                    if (ty >= 0 && ty < H && tx >= 0 && tx < W && my_board_[ty][tx] == 1 && !check[ty][tx])
                    {
                        que.emplace_back(ty, tx);
                    }
                }
            }
        }
        if (!isDone())
        { // "／"方向の連結判定
            auto que = std::deque<std::pair<int, int>>();
            que.emplace_back(coordinate);
            std::vector<std::vector<bool>> check(H, std::vector<bool>(W, false));
            int count = 0;
            while (!que.empty())
            {
                const auto &tmp_cod = que.front();
                que.pop_front();
                ++count;
                if (count >= 4)
                {
                    this->winning_status_ = WinningStatus::LOSE; // 自分の駒が揃ったら相手視点負け
                    break;
                }
                check[tmp_cod.first][tmp_cod.second] = true;

                for (int action = 0; action < 2; action++)
                {
                    int ty = tmp_cod.first + dy_right_up[action];
                    int tx = tmp_cod.second + dx[action];

                    if (ty >= 0 && ty < H && tx >= 0 && tx < W && my_board_[ty][tx] == 1 && !check[ty][tx])
                    {
                        que.emplace_back(ty, tx);
                    }
                }
            }
        }

        if (!isDone())
        { // "\"方向の連結判定

            auto que = std::deque<std::pair<int, int>>();
            que.emplace_back(coordinate);
            std::vector<std::vector<bool>> check(H, std::vector<bool>(W, false));
            int count = 0;
            while (!que.empty())
            {
                const auto &tmp_cod = que.front();
                que.pop_front();
                ++count;
                if (count >= 4)
                {
                    this->winning_status_ = WinningStatus::LOSE; // 自分の駒が揃ったら相手視点負け
                    break;
                }
                check[tmp_cod.first][tmp_cod.second] = true;

                for (int action = 0; action < 2; action++)
                {
                    int ty = tmp_cod.first + dy_leftt_up[action];
                    int tx = tmp_cod.second + dx[action];

                    if (ty >= 0 && ty < H && tx >= 0 && tx < W && my_board_[ty][tx] == 1 && !check[ty][tx])
                    {
                        que.emplace_back(ty, tx);
                    }
                }
            }
        }
        if (!isDone())
        { // 縦方向の連結判定

            int ty = coordinate.first;
            int tx = coordinate.second;
            bool is_win = true;
            for (int i = 0; i < 4; i++)
            {
                bool is_mine = (ty >= 0 && ty < H && tx >= 0 && tx < W && my_board_[ty][tx] == 1);

                if (!is_mine)
                {
                    is_win = false;
                    break;
                }
                --ty;
            }
            if (is_win)
            {
                this->winning_status_ = WinningStatus::LOSE; // 自分の駒が揃ったら相手視点負け
            }
        }

        std::swap(my_board_, enemy_board_);
        is_first_ = !is_first_;
        if (this->winning_status_ == WinningStatus::NONE && legalActions().size() == 0)
        {
            this->winning_status_ = WinningStatus::DRAW;
        }
    }

    // [どのゲームでも実装する] : 現在のプレイヤーが可能な行動を全て取得する
    std::vector<int> legalActions() const
    {
        std::vector<int> actions;
        for (int x = 0; x < W; x++)
            for (int y = H - 1; y >= 0; y--)
            {
                if (my_board_[y][x] == 0 && enemy_board_[y][x] == 0)
                {
                    actions.emplace_back(x);
                    break;
                }
            }
        return actions;
    }

    // [どのゲームでも実装する] : 勝敗情報を取得する
    WinningStatus getWinningStatus() const
    {
        return this->winning_status_;
    }

    // [実装しなくてもよいが実装すると便利] : 現在のプレイヤーの勝率計算のためのスコアを計算する
    double getFirstPlayerScoreForWinRate() const
    {
        switch (this->getWinningStatus())
        {
        case (WinningStatus::WIN):
            if (this->is_first_)
            {
                return 1.;
            }
            else
            {
                return 0.;
            }
        case (WinningStatus::LOSE):
            if (this->is_first_)
            {
                return 0.;
            }
            else
            {
                return 1.;
            }
        default:
            return 0.5;
        }
    }

    // [実装しなくてもよいが実装すると便利] : 現在のゲーム状況を文字列にする
    std::string toString() const
    {
        std::stringstream ss("");

        ss << "is_first:\t" << this->is_first_ << "\n";
        for (int y = H - 1; y >= 0; y--)
        {
            for (int x = 0; x < W; x++)
            {
                char c = '.';
                if (my_board_[y][x] == 1)
                {
                    c = (is_first_ ? 'x' : 'o');
                }
                else if (enemy_board_[y][x] == 1)
                {
                    c = (is_first_ ? 'o' : 'x');
                }
                ss << c;
            }
            ss << "\n";
        }

        return ss.str();
    }
};

using State = ConnectFourState;

using AIFunction = std::function<int(const State &)>;
using StringAIPair = std::pair<std::string, AIFunction>;

// ランダムに行動を決定する
int randomAction(const State &state)
{
    auto legal_actions = state.legalActions();
    return legal_actions[mt_for_action() % (legal_actions.size())];
}

namespace montecarlo
{
    // ランダムプレイアウトをして勝敗スコアを計算する
    double playout(State *state)
    { // const&にすると再帰中にディープコピーが必要になるため、高速化のためポインタにする。(constでない参照でも可)
        switch (state->getWinningStatus())
        {
        case (WinningStatus::WIN):
            return 1.;
        case (WinningStatus::LOSE):
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
        State state_;
        double w_;

    public:
        std::vector<Node> child_nodes_;
        double n_;

        Node(const State &state) : state_(state), w_(0), n_(0) {}

        // ノードの評価を行う
        double evaluate()
        {
            if (this->state_.isDone())
            {
                double value = 0.5;
                switch (this->state_.getWinningStatus())
                {
                case (WinningStatus::WIN):
                    value = 1.;
                    break;
                case (WinningStatus::LOSE):
                    value = 0.;
                    break;
                default:
                    break;
                }
                this->w_ += value;
                ++this->n_;
                return value;
            }
            if (this->child_nodes_.empty())
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
            this->child_nodes_.clear();
            for (const auto action : legal_actions)
            {
                this->child_nodes_.emplace_back(this->state_);
                this->child_nodes_.back().state_.advance(action);
            }
        }

        // どのノードを評価するか選択する
        Node &nextChildNode()
        {
            for (auto &child_node : this->child_nodes_)
            {
                if (child_node.n_ == 0)
                    return child_node;
            }
            double t = 0;
            for (const auto &child_node : this->child_nodes_)
            {
                t += child_node.n_;
            }
            double best_value = -INF;
            int best_action_index = -1;
            for (int i = 0; i < this->child_nodes_.size(); i++)
            {
                const auto &child_node = this->child_nodes_[i];
                double ucb1_value = 1. - child_node.w_ / child_node.n_ + (double)C * std::sqrt(2. * std::log(t) / child_node.n_);
                if (ucb1_value > best_value)
                {
                    best_action_index = i;
                    best_value = ucb1_value;
                }
            }
            return this->child_nodes_[best_action_index];
        }
    };

    // 制限時間(ms)を指定してMCTSで行動を決定する
    int mctsActionWithTimeThreshold(const State &state, const int64_t time_threshold)
    {
        Node root_node = Node(state);
        root_node.expand();
        auto time_keeper = TimeKeeper(time_threshold);
        for (int cnt = 0;; cnt++)
        {
            if (time_keeper.isTimeOver())
            {
                break;
            }
            root_node.evaluate();
        }
        auto legal_actions = state.legalActions();

        int best_action_searched_number = -1;
        int best_action_index = -1;
        assert(legal_actions.size() == root_node.child_nodes_.size());
        for (int i = 0; i < legal_actions.size(); i++)
        {
            int n = root_node.child_nodes_[i].n_;
            if (n > best_action_searched_number)
            {
                best_action_index = i;
                best_action_searched_number = n;
            }
        }
        return legal_actions[best_action_index];
    }
}

using montecarlo::mctsActionWithTimeThreshold;

// ゲームをgame_number×2(先手後手を交代)回プレイしてaisの0番目のAIの勝率を表示する。
void testFirstPlayerWinRate(const std::array<StringAIPair, 2> &ais, const int game_number)
{
    using std::cout;
    using std::endl;

    double first_player_win_rate = 0;
    for (int i = 0; i < game_number; i++)
    {
        auto base_state = State();
        for (int j = 0; j < 2; j++)
        { // 先手後手平等に行う
            auto state = base_state;
            auto &first_ai = ais[j];
            auto &second_ai = ais[(j + 1) % 2];
            for (int k = 0;; k++)
            {
                state.advance(first_ai.second(state));
                if (state.isDone())
                    break;
                state.advance(second_ai.second(state));
                if (state.isDone())
                    break;
            }
            double win_rate_point = state.getFirstPlayerScoreForWinRate();
            if (j == 1)
                win_rate_point = 1 - win_rate_point;
            if (win_rate_point >= 0)
            {
                state.toString();
            }
            first_player_win_rate += win_rate_point;
        }
        cout << "i " << i << " w " << first_player_win_rate / ((i + 1) * 2) << endl;
    }
    first_player_win_rate /= (double)(game_number * 2);
    cout << "Winning rate of " << ais[0].first << " to " << ais[1].first << ":\t" << first_player_win_rate << endl;
}

int main()
{
    auto ais = std::array<StringAIPair, 2>{
        StringAIPair("mctsActionWithTimeThreshold 1ms", [](const State &state)
                     { return mctsActionWithTimeThreshold(state, 1); }),
        StringAIPair("randomAction", [](const State &state)
                     { return randomAction(state); }),
    };
    testFirstPlayerWinRate(ais, 100);

    return 0;
}