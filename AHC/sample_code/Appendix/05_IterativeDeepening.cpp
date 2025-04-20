// Copyright [2022] <Copyright Eita Aoki (Thunder) >
#include <string>
#include <array>
#include <vector>
#include <sstream>
#include <random>
#include <math.h>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <queue>
#include <set>

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
    WIN,
    LOSE,
    DRAW,
    NONE,
};

class State
{
private:
public:
    // ゲームが終了したか判定する
    bool isDone() const
    {
        /*処理*/
        return /*(bool)終了判定*/;
    }

    // 指定したactionでゲームを1ターン進め、次のプレイヤー視点の盤面にする
    void advance(const int action)
    {
        /*処理*/
    }

    // 現在のプレイヤーが可能な行動を全て取得する
    std::vector<int> legalActions() const
    {
        std::vector<int> actions;
        /*処理*/
        return actions;
    }

    // 勝敗情報を取得する
    WinningStatus getWinningStatus() const
    {
        return /*(WinningStatus)勝敗*/;
    }

    // 現在のプレイヤー視点の盤面評価をする
    ScoreType getScore() const
    {
        return /*(ScoreType)現在のプレイヤー視点の盤面評価値*/;
    }

    // // 必要に応じてアンコメント
    // // 現在のゲーム状況を文字列にする
    // std::string toString() const
    // {
    //     return /*(std::string)文字列化されたゲーム状況*/;
    // }
};

namespace iterativedeepening
{
    // 制限時間が切れた際に停止できるalphabetaのためのスコア計算
    ScoreType alphaBetaScore(const State &state, ScoreType alpha, const ScoreType beta, const int depth, const TimeKeeper &time_keeper)
    {
        if (time_keeper.isTimeOver())
            return 0;
        if (state.isDone() || depth == 0)
        {
            return state.getScore();
        }
        auto legal_actions = state.legalActions();
        if (legal_actions.empty())
        {
            return state.getScore();
        }
        for (const auto action : legal_actions)
        {
            State next_state = state;
            next_state.advance(action);
            ScoreType score = -alphaBetaScore(next_state, -beta, -alpha, depth - 1, time_keeper);
            if (time_keeper.isTimeOver())
                return 0;
            if (score > alpha)
            {
                alpha = score;
            }
            if (alpha >= beta)
            {
                return alpha;
            }
        }
        return alpha;
    }
    // 深さと制限時間(ms)を指定してalphabetaで行動を決定する
    int alphaBetaActionWithTimeThreshold(const State &state, const int depth, const TimeKeeper &time_keeper)
    {
        ScoreType best_action = -1;
        ScoreType alpha = -INF;
        for (const auto action : state.legalActions())
        {
            State next_state = state;
            next_state.advance(action);
            ScoreType score = -alphaBetaScore(next_state, -INF, -alpha, depth, time_keeper);
            if (time_keeper.isTimeOver())
                return 0;
            if (score > alpha)
            {
                best_action = action;
                alpha = score;
            }
        }
        return best_action;
    }

    // 制限時間(ms)を指定して反復深化で行動を決定する
    int iterativeDeepeningAction(const State &state, const int64_t time_threshold)
    {
        auto time_keeper = TimeKeeper(time_threshold);
        int best_action = -1;
        for (int depth = 1;; depth++)
        {
            int action = alphaBetaActionWithTimeThreshold(state, depth, time_keeper);

            if (time_keeper.isTimeOver())
            {
                break;
            }
            else
            {
                best_action = action;
            }
        }
        return best_action;
    }
}