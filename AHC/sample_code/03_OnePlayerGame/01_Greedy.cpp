// Copyright [2022] <Copyright Eita Aoki (Thunder) >
#include <string>
#include <sstream>
#include <random>
#include <iostream>
// 座標を保持する
struct Coord
{
    int y_;
    int x_;
    Coord(const int y = 0, const int x = 0) : y_(y), x_(x) {}
};

std::mt19937 mt_for_action(0);                // 行動選択用の乱数生成器を初期化
using ScoreType = int64_t;                    // ゲームの評価スコアの型を決めておく。
constexpr const ScoreType INF = 1000000000LL; // あり得ないぐらい大きなスコアの例を用意しておく

constexpr const int H = 3;  // 迷路の高さ
constexpr const int W = 4;  // 迷路の幅
constexpr int END_TURN = 4; // ゲーム終了ターン

// 一人ゲームの例
// 1ターンに上下左右四方向のいずれかに1マスずつ進む。
// 床にあるポイントを踏むと自身のスコアとなり、床のポイントが消える。
// END_TURNの時点のスコアを高くすることが目的
class MazeState
{
private:
    static constexpr const int dx[4] = {1, -1, 0, 0}; // 右、左、下、上への移動方向のx成分
    static constexpr const int dy[4] = {0, 0, 1, -1}; // 右、左、下、上への移動方向のy成分

    int points_[H][W] = {}; // 床のポイントを1~9で表現する
    int turn_ = 0;          // 現在のターン

public:
    Coord character_ = Coord();
    int game_score_ = 0;            // ゲーム上で実際に得たスコア
    ScoreType evaluated_score_ = 0; // 探索上で評価したスコア
    MazeState() {}

    // h*wの迷路を生成する。
    MazeState(const int seed)
    {
        auto mt_for_construct = std::mt19937(seed); // 盤面構築用の乱数生成器を初期化
        this->character_.y_ = mt_for_construct() % H;
        this->character_.x_ = mt_for_construct() % W;

        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
            {
                if (y == character_.y_ && x == character_.x_)
                {
                    continue;
                }
                this->points_[y][x] = mt_for_construct() % 10;
            }
    }

    // [どのゲームでも実装する] : ゲームの終了判定
    bool isDone() const
    {
        return this->turn_ == END_TURN;
    }
    // [どのゲームでも実装する] : 探索用の盤面評価をする
    void evaluateScore()
    {
        this->evaluated_score_ = this->game_score_; // 簡単のため、まずはゲームスコアをそのまま盤面の評価とする
    }
    // [どのゲームでも実装する] : 指定したactionでゲームを1ターン進める
    void advance(const int action)
    {
        this->character_.x_ += dx[action];
        this->character_.y_ += dy[action];
        auto &point = this->points_[this->character_.y_][this->character_.x_];
        if (point > 0)
        {
            this->game_score_ += point;
            point = 0;
        }
        this->turn_++;
    }

    // [どのゲームでも実装する] : 現在の状況でプレイヤーが可能な行動を全て取得する
    std::vector<int> legalActions() const
    {
        std::vector<int> actions;
        for (int action = 0; action < 4; action++)
        {
            int ty = this->character_.y_ + dy[action];
            int tx = this->character_.x_ + dx[action];
            if (ty >= 0 && ty < H && tx >= 0 && tx < W)
            {
                actions.emplace_back(action);
            }
        }
        return actions;
    }

    // [実装しなくてもよいが実装すると便利] : 現在のゲーム状況を文字列にする
    std::string toString() const
    {
        std::stringstream ss;
        ss << "turn:\t" << this->turn_ << "\n";
        ss << "score:\t" << this->game_score_ << "\n";
        for (int h = 0; h < H; h++)
        {
            for (int w = 0; w < W; w++)
            {
                if (this->character_.y_ == h && this->character_.x_ == w)
                {
                    ss << '@';
                }
                else if (this->points_[h][w] > 0)
                {
                    ss << points_[h][w];
                }
                else
                {
                    ss << '.';
                }
            }
            ss << '\n';
        }

        return ss.str();
    }
};

using State = MazeState;

// ランダムに行動を決定する
int randomAction(const State &state)
{
    auto legal_actions = state.legalActions();
    return legal_actions[mt_for_action() % (legal_actions.size())];
}

// 貪欲法で行動を決定する
int greedyAction(const State &state)
{
    auto legal_actions = state.legalActions();
    ScoreType best_score = -INF; // 絶対にありえない小さな値でベストスコアを初期化する
    int best_action = -1;        // ありえない行動で初期化する
    for (const auto action : legal_actions)
    {
        State now_state = state;
        now_state.advance(action);
        now_state.evaluateScore();
        if (now_state.evaluated_score_ > best_score)
        {
            best_score = now_state.evaluated_score_;
            best_action = action;
        }
    }
    return best_action;
}

// シードを指定してゲーム状況を表示しながらAIにプレイさせる。
void playGame(const int seed)
{
    using std::cout;
    using std::endl;

    auto state = State(seed);
    cout << state.toString() << endl;
    while (!state.isDone())
    {
        state.advance(greedyAction(state));
        cout << state.toString() << endl;
    }
}
int main()
{
    playGame(/*盤面初期化のシード*/ 121321);
    return 0;
}