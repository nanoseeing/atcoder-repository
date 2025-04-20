// Copyright [2022] <Copyright Eita Aoki (Thunder) >
#include <string>
#include <sstream>
#include <random>
#include <iostream>
#include <queue>
#include <chrono>
#include <functional>
#include <unordered_set>
#include <bitset>
// 座標を保持する
struct Coord
{
    int y_;
    int x_;
    Coord(const int y = 0, const int x = 0) : y_(y), x_(x) {}
};

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

constexpr const int H = 7;   // 迷路の高さ
constexpr const int W = 7;   // 迷路の幅
constexpr int END_TURN = 49; // ゲーム終了ターン

namespace single_bitset
{
    std::bitset<H * W> initLeftMask()
    {
        std::bitset<H *W> mask = std::bitset<H * W>();
        for (int y = 0; y < H; ++y)
        {
            mask |= (std::bitset<H * W>(1) << (y * W));
        }
        mask = ~mask;
        return mask;
    }
    std::bitset<H * W> initRightMask()
    {
        std::bitset<H *W> mask = std::bitset<H * W>();
        for (int y = 0; y < H; ++y)
        {
            mask |= (std::bitset<H * W>(1) << (y * W + W - 1));
        }
        mask = ~mask;
        return mask;
    }

    std::bitset<H *W> left_mask = initLeftMask();
    std::bitset<H *W> right_mask = initRightMask();

    // 1個のbitsetでH*Wの二次元配列を表現する
    class Mat
    {
    private:
        std::bitset<H *W> bits_ = std::bitset<H * W>();

        Mat up_mat() const
        {
            Mat ret_mat = *this;
            ret_mat.bits_ >>= W;

            return ret_mat;
        }
        Mat down_mat() const
        {
            Mat ret_mat = *this;
            ret_mat.bits_ <<= W;

            return ret_mat;
        }
        Mat left_mat() const
        {
            Mat ret_mat = *this;
            ret_mat.bits_ |= (ret_mat.bits_ & left_mask) >> 1;

            return ret_mat;
        }
        Mat right_mat() const
        {
            Mat ret_mat = *this;
            ret_mat.bits_ |= (ret_mat.bits_ & right_mask) << 1;
            return ret_mat;
        }

    public:
        Mat(){};
        Mat(const std::bitset<H * W> &mat) : bits_(mat){};

        // 指定位置にbitがあるか
        bool get(int y, int x) const
        {
            return bits_[y * W + x];
        }

        // 指定位置にbitをたてる
        void set(int y, int x)
        {
            bits_.set(y * W + x);
        }
        // 指定位置のbitを消す
        void del(int y, int x)
        {
            bits_.reset(y * W + x);
        }
        // 全bitを4方向に広げる
        void expand()
        {
            Mat up = up_mat();
            Mat down = down_mat();
            Mat left = left_mat();
            Mat right = right_mat();
            this->bits_ |= up.bits_;
            this->bits_ |= down.bits_;
            this->bits_ |= left.bits_;
            this->bits_ |= right.bits_;
        }

        // *this&=~mat // not演算と分けると効率が悪いので1演算にする
        void andeq_not(const Mat &mat)
        {

            this->bits_ &= ~mat.bits_;
        }

        // *this == mat
        bool is_equal(const Mat &mat) const
        {
            return this->bits_ == mat.bits_;
        }
        // *thisとmatで重複して立っているbitが1つでもあるか
        bool is_any_equal(const Mat &mat) const
        {
            return (this->bits_ & mat.bits_).any();
        }
    };
}

namespace zobrist_hash
{
    std::mt19937 mt_init_hash(0);
    // 壁の場所は固定なのでhash管理しない
    uint64_t points[H][W][9 + 1] = {}; // 数字そのままで入れられるように1足す
    uint64_t character[H][W] = {};
    void init()
    {
        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
            {
                for (int p = 1; p < 9 + 1; p++)
                {
                    points[y][x][p] = mt_init_hash();
                }
                character[y][x] = mt_init_hash();
            }
    }

}

// 一人ゲームの例
// 1ターンに上下左右四方向のいずれかに壁のない場所に1マスずつ進む。
// 床にあるポイントを踏むと自身のスコアとなり、床のポイントが消える。
// END_TURNの時点のスコアを高くすることが目的
class WallMazeState
{
private:
    static constexpr const int dx[4] = {1, -1, 0, 0}; // 右、左、下、上への移動方向のx成分
    static constexpr const int dy[4] = {0, 0, 1, -1}; // 右、左、下、上への移動方向のy成分

    struct DistanceCoord
    {
        int y_;
        int x_;
        int distance_;
        DistanceCoord() : y_(0), x_(0), distance_(0) {}
        DistanceCoord(const int y, const int x, const int distance) : y_(y), x_(x), distance_(distance) {}
        DistanceCoord(const Coord &coord) : y_(coord.y_), x_(coord.x_), distance_(0) {}
    };

    // 幅優先探索により、最も近いポイントのある床までの距離を計算する。
    int getDistanceToNearestPoint()
    {
        auto que = std::deque<DistanceCoord>();
        que.emplace_back(this->character_);
        std::vector<std::vector<bool>> check(H, std::vector<bool>(W, false));
        while (!que.empty())
        {
            const auto &tmp_cod = que.front();
            que.pop_front();
            if (this->points_[tmp_cod.y_][tmp_cod.x_] > 0)
            {
                return tmp_cod.distance_;
            }
            check[tmp_cod.y_][tmp_cod.x_] = true;

            for (int action = 0; action < 4; action++)
            {
                int ty = tmp_cod.y_ + dy[action];
                int tx = tmp_cod.x_ + dx[action];

                if (ty >= 0 && ty < H && tx >= 0 && tx < W && !this->walls_[ty][tx] && !check[ty][tx])
                {
                    que.emplace_back(ty, tx, tmp_cod.distance_ + 1);
                }
            }
        }
        return H * W;
    }

    // zobrist_hash用
    void init_hash()
    {
        hash_ = 0;
        hash_ ^= zobrist_hash::character[character_.y_][character_.x_];
        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
            {
                auto point = points_[y][x];
                if (point > 0)
                {
                    hash_ ^= zobrist_hash::points[y][x][point];
                }
            }
    }

public:
    Coord character_ = Coord();
    int game_score_ = 0;            // ゲーム上で実際に得たスコア
    ScoreType evaluated_score_ = 0; // 探索上で評価したスコア
    int first_action_ = -1;         // 探索木のルートノードで最初に選択した行動
    uint64_t hash_ = 0;             // 同一盤面判定に使うハッシュ

    // MazeStateByBitSetから参照できるようにpublic にする
    int points_[H][W] = {}; // 床のポイントを1~9で表現する
    int turn_ = 0;          // 現在のターン
    int walls_[H][W] = {};

    WallMazeState() {}

    // h*wの迷路を生成する。
    WallMazeState(const int seed)
    {
        auto mt_for_construct = std::mt19937(seed); // 盤面構築用の乱数生成器を初期化

        this->character_.y_ = mt_for_construct() % H;
        this->character_.x_ = mt_for_construct() % W;

        // 棒倒し法で壁を生成する。
        for (int y = 1; y < H; y += 2)
            for (int x = 1; x < W; x += 2)
            {
                int ty = y;
                int tx = x;
                // ここで(ty,tx)は1マス置きの位置
                if (ty == character_.y_ && tx == character_.x_)
                {
                    continue;
                }
                this->walls_[ty][tx] = 1;
                int direction_size = 3; // (右、左、下)方向の隣接マスを壁方向にする。
                if (y == 1)
                {
                    direction_size = 4; // 最初だけ上方向の隣接マスも壁候補にする。
                }
                int direction = mt_for_construct() % direction_size;
                ty += dy[direction];
                tx += dx[direction];
                // ここで(ty,tx)は1マス置きの位置からランダムに隣接する位置
                if (ty == character_.y_ && tx == character_.x_)
                {
                    continue;
                }
                this->walls_[ty][tx] = 1;
            }

        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
            {
                if (y == character_.y_ && x == character_.x_)
                {
                    continue;
                }

                this->points_[y][x] = mt_for_construct() % 10;
            }
        this->init_hash();
    }

    // [どのゲームでも実装する] : ゲームの終了判定
    bool isDone() const
    {
        return this->turn_ == END_TURN;
    }
    // [どのゲームでも実装する] : 探索用の盤面評価をする
    void evaluateScore()
    {
        this->evaluated_score_ = this->game_score_ * H * W - getDistanceToNearestPoint(); // 評価に距離情報を加える
    }
    // [どのゲームでも実装する] : 指定したactionでゲームを1ターン進める
    void advance(const int action)
    {
        hash_ ^= zobrist_hash::character[character_.y_][character_.x_]; // 現在のキャラクター位置情報を削除
        this->character_.x_ += dx[action];
        this->character_.y_ += dy[action];
        auto &point = this->points_[this->character_.y_][this->character_.x_];
        hash_ ^= zobrist_hash::character[character_.y_][character_.x_]; // 移動先のキャラクター位置情報を追加
        if (point > 0)
        {
            hash_ ^= zobrist_hash::points[character_.y_][character_.x_][point]; // ポイントがなくなったことをハッシュに反映
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
            if (ty >= 0 && ty < H && tx >= 0 && tx < W && this->walls_[ty][tx] == 0)
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
                if (this->walls_[h][w] == 1)
                {
                    ss << '#';
                }
                else if (this->character_.y_ == h && this->character_.x_ == w)
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

// [どのゲームでも実装する] : 探索時のソート用に評価を比較する
bool operator<(const WallMazeState &maze_1, const WallMazeState &maze_2)
{
    return maze_1.evaluated_score_ < maze_2.evaluated_score_;
}

using single_bitset::Mat;
class MazeStateByBitSet
{
private:
    static constexpr const int dx[4] = {1, -1, 0, 0}; // 右、左、下、上への移動方向のx成分
    static constexpr const int dy[4] = {0, 0, 1, -1}; // 右、左、下、上への移動方向のy成分

    int points_[H][W] = {};       // 床のポイントを1~9で表現する
    Mat whole_point_mat_ = Mat(); // 床のポイントがある場所を1で表現する
    int turn_ = 0;                // 現在のターン
    Mat walls_ = Mat();           // 壁がある場所を1で表現する
    Coord character_ = Coord();

    int ref_count_ = 0; // 参照カウンタ

    // bit演算により、最も近いポイントのある床までの距離を計算する。
    int getDistanceToNearestPoint()
    {

        auto now = Mat();
        now.set(this->character_.y_, this->character_.x_);
        for (int depth = 0;; ++depth)
        {
            if (now.is_any_equal(this->whole_point_mat_))
            {
                return depth;
            }

            auto next = now;
            next.expand();
            next.andeq_not(this->walls_);
            if (next.is_equal(now))
            {
                break;
            }
            now = next;
        }

        return H * W;
    }

    // zobrist_hash用
    void init_hash()
    {
        hash_ = 0;
        hash_ ^= zobrist_hash::character[character_.y_][character_.x_];
        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
            {
                auto point = points_[y][x];
                if (point > 0)
                {
                    hash_ ^= zobrist_hash::points[y][x][point];
                }
            }
    }

public:
    int game_score_;                // ゲーム上で実際に得たスコア
    ScoreType evaluated_score_ = 0; // 探索上で評価したスコア
    int first_action_ = -1;         // 探索木のルートノードで最初に選択した行動
    uint64_t hash_ = 0;             // 同一盤面判定に使うハッシュ
    MazeStateByBitSet() {}

    // h*wの迷路を生成する。
    MazeStateByBitSet(const WallMazeState &state) : turn_(state.turn_), character_(state.character_.y_, state.character_.x_), game_score_(state.game_score_)
    {
        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
            {
                if (state.walls_[y][x])
                {
                    this->walls_.set(y, x);
                }
                if (state.points_[y][x])
                {
                    this->points_[y][x] = state.points_[y][x];
                    this->whole_point_mat_.set(y, x);
                }
            }
        init_hash();
    }

    // [どのゲームでも実装する] : 探索用の盤面評価をする // todo 評価関数を配布するコードではここだけ変えたファイルをコピーしたい。 本文ではここだけ抜き出したい
    void
    evaluateScore()
    {
        this->evaluated_score_ = this->game_score_ * H * W - getDistanceToNearestPoint();
    }

    // [どのゲームでも実装する] : ゲームの終了判定
    bool isDone() const
    {
        return this->turn_ == END_TURN;
    }

    // [どのゲームでも実装する] : 指定したactionでゲームを1ターン進める
    void advance(const int action)
    {
        hash_ ^= zobrist_hash::character[character_.y_][character_.x_]; // 現在のキャラクター位置情報を削除
        this->character_.x_ += dx[action];
        this->character_.y_ += dy[action];
        auto &point = this->points_[this->character_.y_][this->character_.x_];
        hash_ ^= zobrist_hash::character[character_.y_][character_.x_]; // 移動先のキャラクター位置情報を追加
        if (point > 0)
        {
            hash_ ^= zobrist_hash::points[character_.y_][character_.x_][point]; // ポイントがなくなったことをハッシュに反映
            this->game_score_ += point;
            whole_point_mat_.del(this->character_.y_, this->character_.x_);
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
            if (ty >= 0 && ty < H && tx >= 0 && tx < W && !this->walls_.get(ty, tx))
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
                if (this->walls_.get(h, w) == 1)
                {
                    ss << '#';
                }
                else if (this->character_.y_ == h && this->character_.x_ == w)
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

    // 参照カウントを初期化する
    void init()
    {
        ref_count_ = 1;
    }
    // 参照カウントをインクリメントする
    void addRef()
    {
        ++ref_count_;
    }
    // 参照カウントをデクリメントする。参照がなくなったら破棄する
    void release()
    {
        if (ref_count_ == 1)
        {
            delete (this);
        }
        else
        {
            --ref_count_;
        }
    }
};

// [どのゲームでも実装する] : 探索時のソート用に評価を比較する
bool operator<(const MazeStateByBitSet &maze_1, const MazeStateByBitSet &maze_2)
{
    return maze_1.evaluated_score_ < maze_2.evaluated_score_;
}

using State = MazeStateByBitSet;

class StatePtr
{
public:
    State *state;
    StatePtr(const State &state)
    {
        this->state = new State();
        *this->state = state;
        this->state->init();
    }
    StatePtr(const StatePtr &state_ptr)
    {
        this->state = state_ptr.state;
        this->state->addRef();
    }
    void operator=(const StatePtr &state_ptr)
    {
        this->state->release();
        this->state = state_ptr.state;
        this->state->addRef();
    }
    ~StatePtr()
    {
        this->state->release();
    }
};
bool operator<(const StatePtr &state_ptr1, const StatePtr &state_ptr2)
{
    return (*state_ptr1.state) < (*state_ptr2.state);
}

// ビーム幅と深さを指定してビームサーチで行動を決定する
int beamSearchAction(const State &state, const int beam_width, const int beam_depth)
{
    std::priority_queue<StatePtr> now_beam;
    State best_state;

    now_beam.emplace(state);
    auto hash_check = std::unordered_set<uint64_t>();

    for (int t = 0; t < beam_depth; t++)
    {
        std::priority_queue<StatePtr> next_beam;
        for (int i = 0; i < beam_width; i++)
        {
            if (now_beam.empty())
                break;
            State now_state = *now_beam.top().state;
            now_beam.pop();
            auto legal_actions = now_state.legalActions();
            for (const auto &action : legal_actions)
            {
                State next_state = now_state;
                next_state.advance(action);
                if (t >= 1 && hash_check.count(next_state.hash_) > 0)
                {
                    continue;
                }
                hash_check.emplace(next_state.hash_);
                next_state.evaluateScore();
                if (t == 0)
                    next_state.first_action_ = action;
                next_beam.emplace(next_state);
            }
        }

        now_beam = next_beam;
        best_state = *now_beam.top().state;

        if (best_state.isDone())
        {
            break;
        }
    }
    return best_state.first_action_;
}

using AIFunction = std::function<int(const State &)>;
using StringAIPair = std::pair<std::string, AIFunction>;

// ゲームをgame_number回プレイして平均スコアを表示する
void testAiScore(const StringAIPair &ai, const int game_number)
{
    using std::cout;
    using std::endl;
    std::mt19937 mt_for_construct(0);
    double score_mean = 0;
    for (int i = 0; i < game_number; i++)
    {
        auto state = State(mt_for_construct());

        while (!state.isDone())
        {
            state.advance(ai.second(MazeStateByBitSet(state)));
        }
        auto score = state.game_score_;
        score_mean += score;
    }
    score_mean /= (double)game_number;
    cout << "Score of " << ai.first << ":\t" << score_mean << endl;
}

// game_number種類のゲーム盤面についてper_game_number回の処理を行うのに必要な平均時間(ms)を計測して表示する。
void testAiSpeed(const StringAIPair &ai, const int game_number, const int per_game_number)
{
    using std::cout;
    using std::endl;
    std::mt19937 mt_for_construct(0);
    std::chrono::high_resolution_clock::time_point diff_sum;
    for (int i = 0; i < game_number; i++)
    {
        auto state = State(mt_for_construct());
        auto state_bit = MazeStateByBitSet(state);
        auto start_time = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < per_game_number; j++)
        {
            ai.second(state_bit);
        }
        auto diff = std::chrono::high_resolution_clock::now() - start_time;
        diff_sum += diff;
    }
    double time_mean = std::chrono::duration_cast<std::chrono::milliseconds>(diff_sum.time_since_epoch()).count() / (double)(game_number);
    cout << "Time of " << ai.first << ":\t" << time_mean << "ms" << endl;
}

int main()
{
    zobrist_hash::init(); // 必ず最初に呼び出す。
    int beamwidth = 100;
    int beamdepth = END_TURN;
    const auto &ai = StringAIPair("beamSearchAction", [&](const State &state)
                                  { return beamSearchAction(state, beamwidth, beamdepth); });
    testAiScore(ai, /*ゲーム回数*/ 100);
    testAiSpeed(ai, /*ゲーム回数*/ 100, 10);
    return 0;
}