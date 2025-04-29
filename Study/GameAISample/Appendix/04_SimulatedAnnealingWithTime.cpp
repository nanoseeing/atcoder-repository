// Copyright [2022] <Copyright Eita Aoki (Thunder) >
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <utility>
#include <random>
#include <assert.h>
#include <math.h>
#include <chrono>
#include <queue>
#include <algorithm>
#include <iostream>
#include <functional>

// 時間をDouble型で管理し、経過時間も取り出せるクラス
class TimeKeeperDouble
{
private:
    std::chrono::high_resolution_clock::time_point start_time_;
    double time_threshold_;

    double now_time_ = 0;

public:
    // 時間制限をミリ秒単位で指定してインスタンスをつくる。
    TimeKeeperDouble(const double time_threshold)
        : start_time_(std::chrono::high_resolution_clock::now()),
          time_threshold_(time_threshold)
    {
    }

    // 経過時間をnow_time_に格納する。
    void setNowTime()
    {
        auto diff = std::chrono::high_resolution_clock::now() - this->start_time_;
        this->now_time_ = std::chrono::duration_cast<std::chrono::microseconds>(diff).count() * 1e-3; // ms
    }

    // 経過時間をnow_time_に取得する。
    double getNowTime() const
    {
        return this->now_time_;
    }

    // インスタンス生成した時から指定した時間制限を超過したか判定する。
    bool isTimeOver() const
    {
        return now_time_ >= time_threshold_;
    }
};

std::mt19937 mt_for_action(0);

constexpr const int H = 5;     // 迷路の高さ
constexpr const int W = 5;     // 迷路の幅
constexpr int END_TURN = 5;    // ゲーム終了ターン
constexpr int CHARACTER_N = 3; // キャラクターの数

using ScoreType = int64_t;
constexpr const ScoreType INF = 1000000000LL;

class State
{
private:
public:
    // スコア計算をする。
    ScoreType getScore() const
    {

        return /*(ScoreType)評価結果*/;
    }

    // 初期化する
    void init()
    {
        /*処理*/
    }

    // 状態遷移する
    void transition()
    {
        /*処理*/
    }

    // // 必要に応じてアンコメント
    // // 現在のゲーム状況を文字列にする
    // std::string toString() const
    // {
    //     return /*(std::string)文字列化されたゲーム状況*/;
    // }
};

State simulatedAnnealingWithTimeThreshold(const State &state, int64_t time_threshold, double start_temp, double end_temp)
{
    auto time_keeper = TimeKeeperDouble(time_threshold);
    State now_state = state;
    now_state.init();
    ScoreType best_score = now_state.getScore();
    ScoreType now_score = best_score;
    auto best_state = now_state;

    for (;;)
    {
        auto next_state = now_state;
        next_state.transition();
        auto next_score = next_state.getScore();
        double temp = start_temp + (end_temp - start_temp) * (time_keeper.getNowTime() / time_threshold);
        double probability = exp((next_score - now_score) / temp); // 確率probで遷移する
        bool is_force_next = probability > (mt_for_action() % INF) / (double)INF;
        if (next_score > now_score || is_force_next)
        {
            now_score = next_score;
            now_state = next_state;
        }

        if (next_score > best_score)
        {
            best_score = next_score;
            best_state = next_state;
        }
        time_keeper.setNowTime();
        if (time_keeper.isTimeOver())
        {
            break;
        }
    }
    return best_state;
}