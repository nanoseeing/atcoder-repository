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

// ゲームを1回プレイしてゲーム状況を表示する
void playGame()
{
    using std::cout;
    using std::endl;
    auto state = State();
    cout << state.toString() << endl;
    while (!state.isDone())
    {
        // 1p
        {
            cout << "1p ------------------------------------" << endl;
            int action = randomAction(state);
            cout << "action " << action << endl;
            state.advance(action); // (a-1) ここで視点が入れ替わり、2p視点になる。
            cout << state.toString() << endl;
            if (state.isDone())
            {

                switch (state.getWinningStatus()) // (a-2) a-1で2P視点になっているので、WINなら2pの勝利
                {
                case (WinningStatus::WIN):
                    cout << "winner: "
                         << "2p" << endl;
                    break;
                case (WinningStatus::LOSE):
                    cout << "winner: "
                         << "1p" << endl;
                    break;
                default:
                    cout << "DRAW" << endl;
                    break;
                }
                break;
            }
        }
        // 2p
        {
            cout << "2p ------------------------------------" << endl;
            int action = randomAction(state);
            cout << "action " << action << endl;
            state.advance(action); // (b-1) ここで視点が入れ替わり、1p視点になる。
            cout << state.toString() << endl;
            if (state.isDone())
            {

                switch (state.getWinningStatus()) // (b-2) b-1で2P視点になっているので、WINなら1pの勝利
                {
                case (WinningStatus::WIN):
                    cout << "winner: "
                         << "1p" << endl;
                    break;
                case (WinningStatus::LOSE):
                    cout << "winner: "
                         << "2p" << endl;
                    break;
                default:
                    cout << "DRAW" << endl;
                    break;
                }
                break;
            }
        }
    }
}

int main()
{
    playGame();

    return 0;
}