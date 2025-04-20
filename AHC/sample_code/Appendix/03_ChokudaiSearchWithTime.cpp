// Copyright [2022] <Copyright Eita Aoki (Thunder) >
// 本書のサンプルゲーム以外でも時間制限付きビームサーチをするためにコピー&ペーストで使えるサンプルコード
#include <string>
#include <sstream>
#include <random>
#include <iostream>
#include <queue>
#include <chrono>

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

std::mt19937 mt_for_action(0);                // 行動選択用の乱数生成器を初期化
using ScoreType = int64_t;                    // ゲームの評価スコアの型を決めておく。
constexpr const ScoreType INF = 1000000000LL; // あり得ないぐらい大きなスコアの例を用意しておく

class State
{
private:
public:
    ScoreType evaluated_score_ = 0; // 探索上で評価したスコア
    int first_action_ = -1;         // 探索木のルートノードで最初に選択した行動
    State() {}

    // ゲームの終了判定
    bool isDone() const
    {
        /*処理*/
        return /*(bool)終了判定*/;
    }

    // 探索用の盤面評価をする
    void evaluateScore()
    {
        this->evaluated_score_ = /*(ScoreType)評価結果*/;
    }

    // 指定したactionでゲームを1ターン進める
    void advance(const int action)
    {
        /*処理*/
    }

    // 現在の状況でプレイヤーが可能な行動を全て取得する
    std::vector<int> legalActions() const
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

// 探索時のソート用に評価を比較する
bool operator<(const State &state_1, const State &state_2)
{
    return state_1.evaluated_score_ < state_2.evaluated_score_;
}

// ビーム1本あたりのビームの幅と深さ、制限時間(ms)を指定してchokudaiサーチで行動を決定する
int chokudaiSearchActionWithTimeThreshold(
    const State &state, const int beam_width, const int beam_depth, const int64_t time_threshold)
{
    auto time_keeper = TimeKeeper(time_threshold);
    auto beam = std::vector<std::priority_queue<State>>(beam_depth + 1);
    for (int t = 0; t < beam_depth + 1; t++)
    {
        beam[t] = std::priority_queue<State>();
    }
    beam[0].push(state);
    for (int count = 0;; count++)
    {
        for (int t = 0; t < beam_depth; t++)
        {
            auto &now_beam = beam[t];
            auto &next_beam = beam[t + 1];
            for (int i = 0; i < beam_width; i++)
            {
                if (now_beam.empty())
                    break;
                State now_state = now_beam.top();
                if (now_state.isDone())
                {
                    break;
                }
                now_beam.pop();
                auto legal_actions = now_state.legalActions();

                for (const auto &action : legal_actions)
                {
                    State next_state = now_state;
                    next_state.advance(action);
                    next_state.evaluateScore();
                    if (t == 0)
                        next_state.first_action_ = action;
                    next_beam.push(next_state);
                }
            }
        }
        if (time_keeper.isTimeOver())
        {
            break;
        }
    }
    for (int t = beam_depth; t >= 0; t--)
    {
        const auto &now_beam = beam[t];
        if (!now_beam.empty())
        {
            return now_beam.top().first_action_;
        }
    }

    return -1;
}