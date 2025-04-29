// Copyright [2022] <Copyright Eita Aoki (Thunder) >
#include <string>
#include <vector>
#include <array>
#include <sstream>
#include <utility>
#include <random>
#include <math.h>
#include <chrono>
#include <queue>
#include <algorithm>

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

using ScoreType = int64_t;
constexpr const ScoreType INF = 1000000000LL;

enum WinningStatus
{
    FIRST,  // プレイヤー0が勝った
    SECOND, // プレイヤー1が勝った
    DRAW,
    NONE,
};

class State
{
private:
public:
    // 勝敗情報を取得する
    WinningStatus getWinningStatus() const
    {
        return /*(WinningStatus)勝者*/;
    }

    // ゲームが終了したか判定する
    bool isDone() const
    {
        /*処理*/
        return /*(bool)終了判定*/;
    }

    // 指定したactionでゲームを1ターン進める
    void advance(const int action0, const int action1)
    {
        /*処理*/
    }

    // 指定したプレイヤーが可能な行動を全て取得する
    std::vector<int> legalActions(const int player_id) const
    {
        std::vector<int> actions;
        /*処理*/
        return actions;
    }

    // // 必要に応じてアンコメント
    // // 現在のゲーム状況を文字列にする
    // std::string toString() const
    // {
    //     return /*(std::string)文字列化されたゲーム状況*/;
    // }
};

// 指定したプレイヤーの行動をランダムに決定する
int randomAction(const State &state, const int player_id)
{
    auto legal_actions = state.legalActions(player_id);
    return legal_actions[mt_for_action() % (legal_actions.size())];
}

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

    // 制限時間(ms)を指定してDUCTで指定したプレイヤーの行動を決定する
    int ductActionWithTimeThreshold(const State &state, const int player_id, const int64_t time_threshold)
    {
        auto time_keeper = TimeKeeper(time_threshold);
        Node root_node = Node(state);
        root_node.expand();
        for (int i = 0;; i++)
        {
            if (time_keeper.isTimeOver())
            {
                break;
            }
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