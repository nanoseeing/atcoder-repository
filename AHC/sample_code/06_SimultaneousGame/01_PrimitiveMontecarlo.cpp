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

public:
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
};
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

        StringAIPair("primitiveMontecarloAction", [](const State &state, const int player_id)
                     { return primitiveMontecarloAction(state, player_id, 1000); }),
        StringAIPair("randomAction", [](const State &state, const int player_id)
                     { return randomAction(state, player_id); }),
    };

    testFirstPlayerWinRate(ais, 500);

    return 0;
}