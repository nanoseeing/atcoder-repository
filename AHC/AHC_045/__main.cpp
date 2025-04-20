#ifndef AHC_BASE_CPP_
#define AHC_BASE_CPP_

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <numeric>
#include <ranges>
#include <set>
#include <unordered_set>
#include <vector>

using uint8 = uint8_t;
using int64 = int64_t;
using uint64 = uint64_t;

#define UNEXPECTED()                                                     \
    {                                                                    \
        std::cerr << "Unexpected@" << __FILE__ << ":" << __LINE__ << "!" \
                  << std::endl;                                          \
        exit(1);                                                         \
    }

// Override operator<< for ostream to handle vector.
template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &v) {
    os << "[";
    for(const int i : std::views::iota(0, int(v.size()))) {
        if(i > 0)
            os << ", ";
        os << v[i];
    }
    os << "]";
    return os;
}

double g_time_factor = 1.0;

class TimeMeasurement {
  public:
    TimeMeasurement()
        : start_(std::chrono::steady_clock::now()) {
    }

    int64 CurrentMs() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_)
                   .count() *
               g_time_factor;
    }

    int64 CurrentUs() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(now - start_)
                   .count() *
               g_time_factor;
    }

  private:
    std::chrono::time_point<std::chrono::steady_clock> start_;
};
TimeMeasurement g_time;

class Random {
  public:
    Random()
        : state_(3141592653589793238ULL) {
    }

    double RandomDouble(double a, double b) {
        Update();
        const double r = state_ * kScale_;
        return a + (b - a) * r;
    }

    int RandomInt(int a, int b) {
        return static_cast<int>(RandomDouble(a, b));
    }

    int RandomIntInclusive(int a, int b) {
        return RandomInt(a, b + 1);
    }

  private:
    uint64 state_;

    // 2^(-64)
    static constexpr double kScale_ = 1.0 / 18446744073709551616.0;

    void Update() {
        // xorshift
        state_ ^= state_ >> 13;
        state_ ^= state_ << 7;
        state_ ^= state_ >> 17;
    }
};
Random g_random;

#endif // AHC_BASE_CPP_

#ifndef OPTIMIZATION_BASE_CPP_
#define OPTIMIZATION_BASE_CPP_

#ifndef ALGORITHMS_CPP_
#define ALGORITHMS_CPP_

class MergeFindSet {
  public:
    MergeFindSet(const int n)
        : rs_(n), ps_(n), ss_(n, 1) {
        for(const int i : std::views::iota(0, n)) {
            ps_[i] = i;
        }
    }

    int Find(const int x) {
        if(x != ps_[x]) {
            ps_[x] = Find(ps_[x]);
        }
        return ps_[x];
    }

    void Merge(const int x, const int y) {
        const int xr = Find(x);
        const int yr = Find(y);
        if(rs_[xr] > rs_[yr]) {
            ps_[yr] = ps_[xr];
            ss_[xr] += ss_[yr];
        } else if(rs_[xr] < rs_[yr]) {
            ps_[xr] = ps_[yr];
            ss_[yr] += ss_[xr];
        } else if(ps_[xr] != ps_[yr]) {
            ps_[yr] = ps_[xr];
            rs_[xr]++;
            ss_[xr] += ss_[yr];
        }
    }

    int Size(int x) {
        return ss_[Find(x)];
    }

  private:
    std::vector<int> rs_, ps_, ss_;
};

class SubsetSumSolver {
  public:
    SubsetSumSolver() {
    }

    void Solve(const std::vector<int> &xs, const int max_sum) {
        xs_ = xs;
        dp_table_.clear();
        dp_table_.assign(max_sum + 1, 0);
        prev_index_table_.clear();
        prev_index_table_.assign(max_sum + 1, -1);

        dp_table_[0] = 1;
        for(const int i : std::views::iota(0, int(xs_.size()))) {
            const int x = xs_[i];
            // Iterate in descending order so each element is used at most once.
            for(int sum = max_sum; sum >= x; sum--) {
                if(!dp_table_[sum] && dp_table_[sum - x]) {
                    dp_table_[sum] = 1;
                    prev_index_table_[sum] = i;
                }
            }
        }

        is_solved_ = true;
    }

    // Returns whether there is a subset of xs_ that sums to target.
    bool SubsetExists(const int target) const {
        if(!is_solved_) {
            UNEXPECTED();
        }
        if(target >= dp_table_.size())
            return false;
        return dp_table_[target];
    }

    // Returns a subset of xs_ that sums to target as indices.
    // Assumption: SubsetExists(target) is true.
    std::vector<int> GetSubset(const int target) const {
        if(!is_solved_) {
            UNEXPECTED();
        }
        std::vector<int> subset_indices;
        int sum = target;
        while(sum > 0) {
            const int index = prev_index_table_[sum];
            subset_indices.push_back(index);
            sum -= xs_[index];
        }
        if(sum != 0) {
            UNEXPECTED();
        }
        return subset_indices;
    }

  private:
    std::vector<int> xs_;
    bool is_solved_ = false;
    std::vector<int> dp_table_;
    std::vector<int> prev_index_table_;
};

struct Edge {
    int s, t;
    Edge(const int s, const int t)
        : s(s), t(t) {
    }
    Edge()
        : s(-1), t(-1) {
    }
};

struct WEdge {
    int s, t;
    double w;
    WEdge(const int s, const int t, const double w)
        : s(s), t(t), w(w) {
    }
    WEdge()
        : s(-1), t(-1), w(0.0) {
    }
};

struct Graph {
    int vertex_count;
    std::vector<Edge> edges;

    // edges[v_begins[v], v_ends[v]) is the list of edges from the vertex v.
    std::vector<int> v_begins;
    std::vector<int> v_ends;
    Graph(const int in_vertex_count, const std::vector<Edge> &in_edges)
        : vertex_count(in_vertex_count), edges(in_edges.size()),
          v_begins(vertex_count), v_ends(vertex_count) {
        std::vector<int> counts(vertex_count);
        for(const auto &edge : in_edges) {
            counts[edge.s]++;
        }
        for(const int v : std::views::iota(1, vertex_count)) {
            v_begins[v] = v_begins[v - 1] + counts[v - 1];
            v_ends[v] = v_begins[v];
        }
        for(const auto &edge : in_edges) {
            edges[v_ends[edge.s]] = edge;
            v_ends[edge.s]++;
        }
    }

    Graph()
        : vertex_count(0) {
    }

    auto OutEdges(int v) -> std::ranges::subrange<std::vector<Edge>::iterator> {
        auto begin_it = edges.begin() + v_begins[v];
        auto end_it = edges.begin() + v_ends[v];
        return std::ranges::subrange(begin_it, end_it);
    }

    auto OutEdges(int v) const
        -> std::ranges::subrange<std::vector<Edge>::const_iterator> {
        auto begin_it = edges.begin() + v_begins[v];
        auto end_it = edges.begin() + v_ends[v];
        return std::ranges::subrange(begin_it, end_it);
    }

    void Reset(int new_vertex_count, const std::vector<Edge> &new_edges) {
        vertex_count = new_vertex_count;
        edges = new_edges;
        v_begins.clear();
        v_begins.resize(vertex_count);
        v_ends.clear();
        v_ends.resize(vertex_count);

        std::vector<int> counts(vertex_count);
        for(const auto &edge : new_edges) {
            counts[edge.s]++;
        }
        for(const int v : std::views::iota(1, vertex_count)) {
            v_begins[v] = v_begins[v - 1] + counts[v - 1];
            v_ends[v] = v_begins[v];
        }
        for(const auto &edge : new_edges) {
            edges[v_ends[edge.s]] = edge;
            v_ends[edge.s]++;
        }
    }
};

class IndexSet {
  public:
    IndexSet(int n)
        : positions_(n, -1) {
    }

    void Add(int v) {
        if(Contains(v)) {
            return;
        }
        positions_[v] = values_.size();
        values_.push_back(v);
    }

    void Remove(int v) {
        int p = positions_[v];
        int b = values_.back();
        values_[p] = b;
        values_.pop_back();
        positions_[b] = p;
        positions_[v] = -1;
    }

    bool Contains(int v) const {
        return positions_[v] != -1;
    }

    int Size() const {
        return values_.size();
    }

    const std::vector<int> &Values() const {
        return values_;
    }

    void Clear() {
        for(const int v : values_) {
            positions_[v] = -1;
        }
        values_.clear();
    }

  private:
    std::vector<int> values_;
    std::vector<int> positions_;
};

class DoubleMatrix {
  public:
    DoubleMatrix(int r, int c)
        : r_(r), c_(c), values_(r * c) {
    }

    double Get(int i, int j) const {
        return values_[i * c_ + j];
    }

    void Set(int i, int j, double value) {
        values_[i * c_ + j] = value;
    }

    void Increment(int i, int j, double value) {
        values_[i * c_ + j] += value;
    }

  private:
    int r_;
    int c_;
    std::vector<double> values_;
};

#endif // ALGORITHMS_CPP_

#ifndef MCM_CPP_
#define MCM_CPP_

#ifndef DEBUG_CPP_
#define DEBUG_CPP_

#ifndef PROBLEM_BASE_CPP_
#define PROBLEM_BASE_CPP_

constexpr int kVertexCount = 800;
constexpr int kMaxQueryCount = 400;
constexpr int kMaxQuerySize = 15;

constexpr int kQueryPhaseQuickModeTime = 1200;

bool g_is_local_mode = false;

struct Double2D {
    double x = 0.0;
    double y = 0.0;
    Double2D(double x, double y)
        : x(x), y(y) {
    }
    Double2D()
        : x(0.0), y(0.0) {
    }
};

// Returns the squared distance between two points.
double Distance2(const Double2D &p0, const Double2D &p1) {
    const double dx = p0.x - p1.x;
    const double dy = p0.y - p1.y;
    return dx * dx + dy * dy;
}

double Distance(const Double2D &p0, const Double2D &p1) {
    const double dx = p0.x - p1.x;
    const double dy = p0.y - p1.y;
    return std::sqrt(dx * dx + dy * dy);
}

struct Double2DRange {
    double x_min;
    double x_max;
    double y_min;
    double y_max;
    Double2DRange(const double x_min, const double x_max, const double y_min,
                  const double y_max)
        : x_min(x_min), x_max(x_max), y_min(y_min), y_max(y_max) {
    }
    Double2DRange()
        : x_min(0.0), x_max(0.0), y_min(0.0), y_max(0.0) {
    }
};

struct ProblemInput {
    int cluster_count = 0;
    int max_query_size = 0;
    int max_error_width = 0;
    std::vector<int> cluster_sizes;
    std::array<double, kVertexCount> x_mins;
    std::array<double, kVertexCount> x_maxs;
    std::array<double, kVertexCount> y_mins;
    std::array<double, kVertexCount> y_maxs;
};

std::array<Double2D, kVertexCount> g_actual_points{};

struct RangeConstraint {
    int i;
    double x_min;
    double x_max;
    double y_min;
    double y_max;
    RangeConstraint(int i, double x_min, double x_max, double y_min, double y_max)
        : i(i), x_min(x_min), x_max(x_max), y_min(y_min), y_max(y_max) {
    }
};

// A constraint that distance(P_i0, p_i1) >= distance(P_j0, p_j1).
struct DistanceConstraint {
    int i0, i1, j0, j1;
    DistanceConstraint(const int i0, const int i1, const int j0, const int j1)
        : i0(i0), i1(i1), j0(j0), j1(j1) {
    }
    DistanceConstraint()
        : i0(-1), i1(-1), j0(-1), j1(-1) {
    }

    // For std::set.
    bool operator==(const DistanceConstraint &other) const {
        return i0 == other.i0 && i1 == other.i1 && j0 == other.j0 && j1 == other.j1;
    }

    // For std::set.
    bool operator<(const DistanceConstraint &other) const {
        if(i0 != other.i0)
            return i0 < other.i0;
        if(i1 != other.i1)
            return i1 < other.i1;
        if(j0 != other.j0)
            return j0 < other.j0;
        return j1 < other.j1;
    }
};

enum class IndexedDistanceConstraintType { kUndefined,
                                           kI0,
                                           kJ0,
                                           kI0J0 };

struct IndexedDistanceConstraint {
    int index;
    IndexedDistanceConstraintType type;
    DistanceConstraint distance_constraint;
};

class ConstraintSet {
  public:
    std::vector<RangeConstraint> range_constraints;

    // For gradient descent.
    std::vector<DistanceConstraint> distance_constraints;
    std::vector<std::vector<int>> v_to_distance_constraint_indexes;

    // For Gibbs sampling.
    std::vector<std::vector<IndexedDistanceConstraint>> v_to_i0_constraints;
    std::vector<std::vector<IndexedDistanceConstraint>> v_to_j0_constraints;
    std::vector<std::vector<IndexedDistanceConstraint>> v_to_i0j0_constraints;

    ConstraintSet()
        : v_to_distance_constraint_indexes(kVertexCount),
          v_to_i0_constraints(kVertexCount), v_to_j0_constraints(kVertexCount),
          v_to_i0j0_constraints(kVertexCount) {
    }

    void AddRangeConstraint(const RangeConstraint &range_constraint) {
        range_constraints.push_back(range_constraint);
    }

    void AddDistanceConstraint(const DistanceConstraint &distance_constraint) {
        if(distance_constraint_set_.contains(distance_constraint)) {
            return;
        }
        distance_constraint_set_.insert(distance_constraint);

        const int distance_constraint_index = distance_constraints.size();
        distance_constraints.push_back(distance_constraint);
        v_to_distance_constraint_indexes[distance_constraint.i0].push_back(
            distance_constraint_index);
        v_to_distance_constraint_indexes[distance_constraint.i1].push_back(
            distance_constraint_index);
        v_to_distance_constraint_indexes[distance_constraint.j0].push_back(
            distance_constraint_index);
        v_to_distance_constraint_indexes[distance_constraint.j1].push_back(
            distance_constraint_index);

        const auto [i0, i1, j0, j1] = distance_constraint;
        // Index i0
        if(i0 == j0) {
            IndexedDistanceConstraint constraint;
            constraint.index = i0;
            constraint.type = IndexedDistanceConstraintType::kI0J0;
            constraint.distance_constraint = DistanceConstraint(i0, i1, j0, j1);
            v_to_i0j0_constraints[i0].push_back(constraint);
        } else if(i0 == j1) {
            IndexedDistanceConstraint constraint;
            constraint.index = i0;
            constraint.type = IndexedDistanceConstraintType::kI0J0;
            constraint.distance_constraint = DistanceConstraint(i0, i1, j1, j0);
            v_to_i0j0_constraints[i0].push_back(constraint);
        } else {
            IndexedDistanceConstraint constraint;
            constraint.index = i0;
            constraint.type = IndexedDistanceConstraintType::kI0;
            constraint.distance_constraint = DistanceConstraint(i0, i1, j0, j1);
            v_to_i0_constraints[i0].push_back(constraint);
        }

        // Index i1
        if(i1 == j0) {
            IndexedDistanceConstraint constraint;
            constraint.index = i1;
            constraint.type = IndexedDistanceConstraintType::kI0J0;
            constraint.distance_constraint = DistanceConstraint(i1, i0, j0, j1);
            v_to_i0j0_constraints[i1].push_back(constraint);
        } else if(i1 == j1) {
            IndexedDistanceConstraint constraint;
            constraint.index = i1;
            constraint.type = IndexedDistanceConstraintType::kI0J0;
            constraint.distance_constraint = DistanceConstraint(i1, i0, j1, j0);
            v_to_i0j0_constraints[i1].push_back(constraint);
        } else {
            IndexedDistanceConstraint constraint;
            constraint.index = i1;
            constraint.type = IndexedDistanceConstraintType::kI0;
            constraint.distance_constraint = DistanceConstraint(i1, i0, j0, j1);
            v_to_i0_constraints[i1].push_back(constraint);
        }

        // Index j0
        if(j0 == i0 || j0 == i1) {
            // This case is already indexed to i0 or i1.
        } else {
            IndexedDistanceConstraint constraint;
            constraint.index = j0;
            constraint.type = IndexedDistanceConstraintType::kJ0;
            constraint.distance_constraint = DistanceConstraint(i0, i1, j0, j1);
            v_to_j0_constraints[j0].push_back(constraint);
        }

        // Index j1
        if(j1 == i0 || j1 == i1) {
            // This case is already indexed to i0 or i1.
        } else {
            IndexedDistanceConstraint constraint;
            constraint.index = j1;
            constraint.type = IndexedDistanceConstraintType::kJ0;
            constraint.distance_constraint = DistanceConstraint(i0, i1, j1, j0);
            v_to_j0_constraints[j1].push_back(constraint);
        }
    }

  private:
    // For deduplication.
    std::set<DistanceConstraint> distance_constraint_set_;
};

struct MCMCSample {
    std::array<Double2D, kVertexCount> points{};
};

struct MSTCluster {
    std::vector<int> vs;
    std::vector<Edge> mst_edges;
    MSTCluster(const std::vector<int> &vs, const std::vector<Edge> &mst_edges)
        : vs(vs), mst_edges(mst_edges) {
    }
};

#endif // PROBLEM_BASE_CPP_

void CheckConstraints(
    const ConstraintSet &constraint_set,
    const std::array<Double2D, kVertexCount> &points) {
    const double kMargin = 10.0;
    int range_constraint_error_count = 0;
    int distance_constraint_error_count = 0;
    for(const auto &constraint : constraint_set.range_constraints) {
        if(points[constraint.i].x < constraint.x_min - kMargin ||
           points[constraint.i].x > constraint.x_max + kMargin ||
           points[constraint.i].y < constraint.y_min - kMargin ||
           points[constraint.i].y > constraint.y_max + kMargin) {
            range_constraint_error_count++;
        }
    }
    for(const auto &constraint : constraint_set.distance_constraints) {
        const double d2i = Distance2(points[constraint.i0], points[constraint.i1]);
        const double di = std::sqrt(d2i);
        const double d2j = Distance2(points[constraint.j0], points[constraint.j1]);
        const double dj = std::sqrt(d2j);
        if(di - dj < -kMargin) {
            distance_constraint_error_count++;
        }
    }
    std::cerr << "# range constraint errors: " << range_constraint_error_count
              << std::endl;
    std::cerr << "# distance constraint errors: "
              << distance_constraint_error_count << std::endl;
}

#endif // DEBUG_CPP_

constexpr double kSamplingTimeLimit = 1700.0;

void RunGibbsSampling(
    const std::vector<int> &vs,
    const ConstraintSet &constraint_set,
    std::array<Double2D, kVertexCount> &points,
    std::vector<Double2DRange> &ranges,
    bool is_medium_mode = false) {
    for(const int v : vs) {
        // x
        double x_min =
            std::min(constraint_set.range_constraints[v].x_min, points[v].x);
        double x_max =
            std::max(constraint_set.range_constraints[v].x_max, points[v].x);
        double x = points[v].x;
        double y = points[v].y;

        for(const auto &indexed_istance_constraint :
            constraint_set.v_to_j0_constraints[v]) {
            const auto &distance_constraint =
                indexed_istance_constraint.distance_constraint;
            const auto [i0, i1, j0, j1] = distance_constraint;
            const double dxi = points[i1].x - points[i0].x;
            const double dyi = points[i1].y - points[i0].y;
            const double dxj = points[j1].x - x;
            const double dyj = points[j1].y - y;
            const double d2i = dxi * dxi + dyi * dyi;
            const double d2j = dxj * dxj + dyj * dyj;
            if(d2i >= d2j) {
                const double r2 = d2i - dyj * dyj; // r2 >= 0.0
                const double r = std::sqrt(std::max(r2, 0.0));
                x_min = std::max(x_min, points[j1].x - r);
                x_max = std::min(x_max, points[j1].x + r);
            } else {
                if(points[j0].x < points[j1].x) {
                    x_min = std::max(x_min, points[j0].x);
                    x_max = std::min(x_max, points[j1].x);
                } else {
                    x_min = std::max(x_min, points[j1].x);
                    x_max = std::min(x_max, points[j0].x);
                }
            }
        }

        for(const auto &indexed_istance_constraint :
            constraint_set.v_to_i0_constraints[v]) {
            const auto &distance_constraint =
                indexed_istance_constraint.distance_constraint;
            const auto [i0, i1, j0, j1] = distance_constraint;
            const double dxi = points[i1].x - x;
            const double dyi = points[i1].y - y;
            const double dxj = points[j1].x - points[j0].x;
            const double dyj = points[j1].y - points[j0].y;
            const double d2i = dxi * dxi + dyi * dyi;
            const double d2j = dxj * dxj + dyj * dyj;
            if(d2i >= d2j) {
                const double r2 = d2j - dyi * dyi;
                if(r2 >= 0.0) {
                    const double r = std::sqrt(r2);
                    if(points[i0].x < points[i1].x) {
                        x_max = std::min(x_max, points[i1].x - r);
                    } else {
                        x_min = std::max(x_min, points[i1].x + r);
                    }
                } else {
                    // Do nothing.
                }
            } else {
                if(points[i0].x < points[i1].x) {
                    x_max = std::min(x_max, points[i0].x);
                } else {
                    x_min = std::max(x_min, points[i0].x);
                }
            }
        }

        for(const auto &indexed_istance_constraint :
            constraint_set.v_to_i0j0_constraints[v]) {
            const auto &distance_constraint =
                indexed_istance_constraint.distance_constraint;
            const auto [i0, i1, j0, j1] = distance_constraint;
            const double dxi = points[i1].x - x;
            const double dyi = points[i1].y - y;
            const double dxj = points[j1].x - x;
            const double dyj = points[j1].y - y;
            const double d2i = dxi * dxi + dyi * dyi;
            const double d2j = dxj * dxj + dyj * dyj;
            if(d2i >= d2j) {
                const double y0 = y;
                const double x1 = points[i1].x;
                const double y1 = points[i1].y;
                const double x3 = points[j1].x;
                const double y3 = points[j1].y;
                if(x3 - x1 > 0.001) {
                    const double x =
                        (x3 * x3 - x1 * x1 + y3 * y3 - y1 * y1 + 2 * y0 * (y1 - y3)) /
                        (2.0 * (x3 - x1));
                    x_min = std::max(x_min, x);
                } else if(x3 - x1 < -0.001) {
                    const double x =
                        (x3 * x3 - x1 * x1 + y3 * y3 - y1 * y1 + 2 * y0 * (y1 - y3)) /
                        (2.0 * (x3 - x1));
                    x_max = std::min(x_max, x);
                } else {
                    // Do nothing.
                }
            } else {
                const double x1 = points[i1].x;
                const double x3 = points[j1].x;
                if(x3 - x1 > 0.001) {
                    x_min = std::max(x_min, points[i0].x);
                } else if(x3 - x1 < -0.001) {
                    x_max = std::min(x_max, points[i0].x);
                } else {
                    // Do nothing.
                }
            }
        }

        if(x_min > x_max) {
            // if (x_min - x_max > 10.0) {
            //   UNEXPECTED();
            // }
            points[v].x = (x_min + x_max) * 0.5;
            ranges[v].x_min = points[v].x;
            ranges[v].x_max = points[v].x;
        } else {
            if(is_medium_mode) {
                points[v].x = (x_min + x_max) * 0.5;
            } else {
                points[v].x = g_random.RandomDouble(x_min, x_max);
            }
            ranges[v].x_min = x_min;
            ranges[v].x_max = x_max;
        }

        x = points[v].x;

        // y
        double y_min =
            std::min(constraint_set.range_constraints[v].y_min, points[v].y);
        double y_max =
            std::max(constraint_set.range_constraints[v].y_max, points[v].y);

        for(const auto &indexed_istance_constraint :
            constraint_set.v_to_j0_constraints[v]) {
            const auto &distance_constraint =
                indexed_istance_constraint.distance_constraint;
            const auto [i0, i1, j0, j1] = distance_constraint;
            const double dxi = points[i1].x - points[i0].x;
            const double dyi = points[i1].y - points[i0].y;
            const double dxj = points[j1].x - x;
            const double dyj = points[j1].y - y;
            const double d2i = dxi * dxi + dyi * dyi;
            const double d2j = dxj * dxj + dyj * dyj;
            if(d2i >= d2j) {
                const double r2 = d2i - dxj * dxj; // r2 >= 0.0
                const double r = std::sqrt(std::fmax(r2, 0.0));
                y_min = std::max(y_min, points[j1].y - r);
                y_max = std::min(y_max, points[j1].y + r);
            } else {
                if(points[j0].y < points[j1].y) {
                    y_min = std::max(y_min, y);
                    y_max = std::min(y_max, points[j1].y);
                } else {
                    y_min = std::max(y_min, points[j1].y);
                    y_max = std::min(y_max, y);
                }
            }
        }

        for(const auto &indexed_istance_constraint :
            constraint_set.v_to_i0_constraints[v]) {
            const auto &distance_constraint =
                indexed_istance_constraint.distance_constraint;
            const auto [i0, i1, j0, j1] = distance_constraint;
            const double dxi = points[i1].x - x;
            const double dyi = points[i1].y - y;
            const double dxj = points[j1].x - points[j0].x;
            const double dyj = points[j1].y - points[j0].y;
            const double d2i = dxi * dxi + dyi * dyi;
            const double d2j = dxj * dxj + dyj * dyj;
            if(d2i >= d2j) {
                const double r2 = d2j - dxi * dxi;
                if(r2 > 0) {
                    const double r = std::sqrt(r2);
                    if(points[i0].y < points[i1].y) {
                        y_max = std::min(y_max, points[i1].y - r);
                    } else {
                        y_min = std::max(y_min, points[i1].y + r);
                    }
                }
            } else {
                if(points[i0].y < points[i1].y) {
                    y_max = std::min(y_max, y);
                } else {
                    y_min = std::max(y_min, y);
                }
            }
        }

        for(const auto &indexed_istance_constraint :
            constraint_set.v_to_i0j0_constraints[v]) {
            const auto &distance_constraint =
                indexed_istance_constraint.distance_constraint;
            const auto [i0, i1, j0, j1] = distance_constraint;
            const double dxi = points[i1].x - x;
            const double dyi = points[i1].y - y;
            const double dxj = points[j1].x - x;
            const double dyj = points[j1].y - y;
            const double d2i = dxi * dxi + dyi * dyi;
            const double d2j = dxj * dxj + dyj * dyj;
            if(d2i >= d2j) {
                const double x0 = x;
                const double x1 = points[i1].x;
                const double y1 = points[i1].y;
                const double x3 = points[j1].x;
                const double y3 = points[j1].y;
                if(y3 - y1 > 0.001) {
                    const double y =
                        (y3 * y3 - y1 * y1 + x3 * x3 - x1 * x1 + 2 * x0 * (x1 - x3)) /
                        (2.0 * (y3 - y1));
                    y_min = std::max(y_min, y);
                } else if(y3 - y1 < -0.001) {
                    const double y =
                        (y3 * y3 - y1 * y1 + x3 * x3 - x1 * x1 + 2 * x0 * (x1 - x3)) /
                        (2.0 * (y3 - y1));
                    y_max = std::min(y_max, y);
                } else {
                    // Do nothing.
                }
            } else {
                const double y1 = points[i1].y;
                const double y3 = points[j1].y;
                if(y3 - y1 > 0.001) {
                    y_min = std::max(y_min, y);
                } else if(y3 - y1 < -0.001) {
                    y_max = std::min(y_max, y);
                } else {
                    // Do nothing.
                }
            }
        }

        if(y_min > y_max) {
            // if (y_min - y_max > 10.0) {
            //   UNEXPECTED();
            // }
            points[v].y = (y_min + y_max) * 0.5;
            ranges[v].y_min = points[v].y;
            ranges[v].y_max = points[v].y;
        } else {
            if(is_medium_mode) {
                points[v].y = (y_min + y_max) * 0.5;
            } else {
                points[v].y = g_random.RandomDouble(y_min, y_max);
            }
            ranges[v].y_min = y_min;
            ranges[v].y_max = y_max;
        }
    }
}

void RunMCMCPhase(const std::array<Double2D, kVertexCount> &points,
                  const ConstraintSet &constraint_set,
                  std::vector<MCMCSample> &samples,
                  DoubleMatrix &distance_matrix) {
    samples.clear();
    for(const int i : std::views::iota(0, kVertexCount)) {
        for(const int j : std::views::iota(0, kVertexCount)) {
            distance_matrix.Set(i, j, 0.0);
        }
    }

    std::vector<int> vs;
    for(const int v : std::views::iota(0, kVertexCount)) {
        vs.push_back(v);
    }

    std::array<Double2D, kVertexCount> adjusted_points = points;
    std::vector<Double2DRange> ranges(kVertexCount);
    RunGibbsSampling(vs, constraint_set, adjusted_points, ranges, true);
    samples.push_back(MCMCSample(adjusted_points));
    for(const int i : std::views::iota(0, kVertexCount)) {
        for(const int j : std::views::iota(i + 1, kVertexCount)) {
            distance_matrix.Increment(
                i, j, Distance(samples[0].points[i], samples[0].points[j]));
        }
    }
    const int adjusted_sample_timie_limit =
        std::min(kSamplingTimeLimit, (g_time.CurrentMs() + 1900.0) * 0.5);
    while(samples.size() < 1000) {
        if(g_time.CurrentMs() > adjusted_sample_timie_limit) {
            std::cerr << "Emergency exit from sampling (" << samples.size()
                      << " samples)." << std::endl;
            break;
        }
        MCMCSample sample = samples.back();
        RunGibbsSampling(vs, constraint_set, sample.points, ranges);
        samples.push_back(sample);
        for(const int i : std::views::iota(0, kVertexCount)) {
            for(const int j : std::views::iota(i + 1, kVertexCount)) {
                distance_matrix.Increment(
                    i, j, Distance(sample.points[i], sample.points[j]));
            }
        }
    }

    const double factor = 1.0 / samples.size();
    for(const int i : std::views::iota(0, kVertexCount)) {
        for(const int j : std::views::iota(i + 1, kVertexCount)) {
            distance_matrix.Set(i, j, distance_matrix.Get(i, j) * factor);
            distance_matrix.Set(j, i, distance_matrix.Get(i, j));
        }
    }
}

#endif // MCM_CPP_

bool RunGradientDescent(
    const ConstraintSet &constraint_set,
    const double alpha,
    std::array<Double2D, kVertexCount> &points,
    IndexSet &updated_vertex_set,
    IndexSet &added_constraint_index_set) {
    static std::vector<int> current_updated_vs;
    current_updated_vs = updated_vertex_set.Values();

    added_constraint_index_set.Clear();

    auto expand_v = [&](const int v, int min_index) {
        for(const auto constraint_index :
            constraint_set.v_to_distance_constraint_indexes[v]) {
            if(constraint_index < min_index) {
                continue;
            }
            if(added_constraint_index_set.Contains(constraint_index)) {
                continue;
            }
            added_constraint_index_set.Add(constraint_index);
        }
    };

    for(const int v : updated_vertex_set.Values()) {
        expand_v(v, -1);
    }

    updated_vertex_set.Clear();

    for(const int constraint_index : added_constraint_index_set.Values()) {
        const auto &distance_constraint =
            constraint_set.distance_constraints[constraint_index];
        const auto &[i0, i1, j0, j1] = distance_constraint;
        const double dxi = points[i1].x - points[i0].x;
        const double dyi = points[i1].y - points[i0].y;
        const double dxj = points[j1].x - points[j0].x;
        const double dyj = points[j1].y - points[j0].y;
        const double d2i = dxi * dxi + dyi * dyi;
        const double d2j = dxj * dxj + dyj * dyj;
        if(d2i < d2j) {
            const double di = std::sqrt(d2i + 0.001);
            const double dj = std::sqrt(d2j + 0.001);
            const double ndxi = dxi / di;
            const double ndyi = dyi / di;
            const double ndxj = dxj / dj;
            const double ndyj = dyj / dj;
            points[i0].x += -alpha * ndxi;
            points[i0].y += -alpha * ndyi;
            points[i1].x += alpha * ndxi;
            points[i1].y += alpha * ndyi;
            points[j0].x += alpha * ndxj;
            points[j0].y += alpha * ndyj;
            points[j1].x += -alpha * ndxj;
            points[j1].y += -alpha * ndyj;
            updated_vertex_set.Add(i0);
            updated_vertex_set.Add(i1);
            updated_vertex_set.Add(j0);
            updated_vertex_set.Add(j1);
        }
    }

    for(const int i : current_updated_vs) {
        const auto &[unused_i, x_min, x_max, y_min, y_max] =
            constraint_set.range_constraints[i];
        const double x_center = (x_min + x_max) / 2.0;
        const double y_center = (y_min + y_max) / 2.0;
        if(points[i].x < x_min) {
            points[i].x += alpha;
            updated_vertex_set.Add(i);
        } else if(points[i].x > x_max) {
            points[i].x += -alpha;
            updated_vertex_set.Add(i);
        }
        if(points[i].y < y_min) {
            points[i].y += alpha;
            updated_vertex_set.Add(i);
        } else if(points[i].y > y_max) {
            points[i].y += -alpha;
            updated_vertex_set.Add(i);
        }
    }

    return updated_vertex_set.Size() > 0;
}

#endif // OPTIMIZATION_BASE_CPP_

#ifndef SOLVER_CPP_
#define SOLVER_CPP_

#ifndef CLUSTERING_CPP_
#define CLUSTERING_CPP_

#ifndef MST_CPP_
#define MST_CPP_

std::vector<Edge> FindMST(const int vertex_count,
                          const DoubleMatrix &distance_matrix) {
    std::vector<Edge> result;
    result.reserve(vertex_count - 1);

    std::vector<uint8> visited(vertex_count, false);
    std::vector<double> min_weights(vertex_count,
                                    std::numeric_limits<double>::max());
    std::vector<int> parents(vertex_count, -1);
    visited[0] = true;
    min_weights[0] = 0.0;

    int min_weight_v = 0;
    for(int count = 0; count < vertex_count - 1; ++count) {
        for(int v = 0; v < vertex_count; ++v) {
            if(!visited[v]) {
                double edge_weight = distance_matrix.Get(min_weight_v, v);
                if(edge_weight < min_weights[v]) {
                    min_weights[v] = edge_weight;
                    parents[v] = min_weight_v;
                }
            }
        }

        double min_weight = std::numeric_limits<double>::max();
        for(int v = 0; v < vertex_count; ++v) {
            if(!visited[v] && min_weights[v] < min_weight) {
                min_weight = min_weights[v];
                min_weight_v = v;
            }
        }

        visited[min_weight_v] = true;
        result.emplace_back(parents[min_weight_v], min_weight_v);
    }
    return result;
}

// Assumption: vertex_count <= N
template <std::size_t N>
void FindMSTOptimized(const int vertex_count,
                      const DoubleMatrix &distance_matrix,
                      std::vector<Edge> &result) {
    static std::array<uint8, N> visited = {};
    static std::array<double, N> min_weights = {};
    static std::array<int, N> parents = {};

    for(const int v : std::views::iota(0, vertex_count)) {
        visited[v] = false;
        min_weights[v] = std::numeric_limits<double>::max();
        parents[v] = -1;
    }

    visited[0] = true;
    min_weights[0] = 0.0;

    int min_weight_v = 0;
    for(int count = 0; count < vertex_count - 1; ++count) {
        for(int v = 0; v < vertex_count; ++v) {
            if(!visited[v]) {
                double edge_weight = distance_matrix.Get(min_weight_v, v);
                if(edge_weight < min_weights[v]) {
                    min_weights[v] = edge_weight;
                    parents[v] = min_weight_v;
                }
            }
        }

        double min_weight = std::numeric_limits<double>::max();
        for(int v = 0; v < vertex_count; ++v) {
            if(!visited[v] && min_weights[v] < min_weight) {
                min_weight = min_weights[v];
                min_weight_v = v;
            }
        }

        visited[min_weight_v] = true;
        result.emplace_back(parents[min_weight_v], min_weight_v);
    }
}

// Find MST for a specified set of vertices.
std::vector<Edge> FindMST(const std::vector<int> &vs,
                          const DoubleMatrix &distance_matrix) {
    std::vector<Edge> result;
    result.reserve(vs.size() - 1);

    std::vector<uint8> visited(vs.size(), false);
    std::vector<double> min_weights(vs.size(),
                                    std::numeric_limits<double>::max());
    std::vector<int> parents(vs.size(), -1);
    visited[0] = true;
    min_weights[0] = 0.0;

    int min_weight_i = 0;
    for(int count = 0; count < vs.size() - 1; ++count) {
        for(const int i : std::views::iota(0, int(vs.size()))) {
            if(!visited[i]) {
                double edge_weight = distance_matrix.Get(vs[min_weight_i], vs[i]);
                if(edge_weight < min_weights[i]) {
                    min_weights[i] = edge_weight;
                    parents[i] = min_weight_i;
                }
            }
        }

        double min_weight = std::numeric_limits<double>::max();
        for(const int i : std::views::iota(0, int(vs.size()))) {
            if(!visited[i] && min_weights[i] < min_weight) {
                min_weight = min_weights[i];
                min_weight_i = i;
            }
        }

        visited[min_weight_i] = true;
        result.emplace_back(vs[parents[min_weight_i]], vs[min_weight_i]);
    }
    return result;
}

#endif

// Returns whether two edges are equal (undirected).
bool UndirectedEdgeEqual(const Edge &e0, const Edge &e1) {
    return (e0.s == e1.s && e0.t == e1.t) || (e0.s == e1.t && e0.t == e1.s);
}

// ---------------------
// Extract MST edges that have both endpoints in the given component
// ---------------------
void ExtractMstEdges(const std::vector<int> &component,
                     const std::vector<Edge> &mst_edges,
                     std::vector<Edge> &component_mst_edges) {
    for(const auto &edge : mst_edges) {
        if(find(component.begin(), component.end(), edge.s) != component.end() &&
           find(component.begin(), component.end(), edge.t) != component.end())
            component_mst_edges.push_back(edge);
    }
}

// ---------------------
// Recursive clustering function.
// vs: The current set of vertices.
// mst_edges: MST edges for vs
// cluster_ids: Indices (into target_sizes) for clusters that must be assigned
//              to vs. target_sizes remains constant globally.
// points: Coordinates for all vertices.
// assignment: Output assignment for vertices; assignment[i] is the cluster id
//             for vertex i.
bool RunClustering(const ProblemInput &input, const std::vector<int> &vs,
                   const std::vector<Edge> &mst_edges,
                   const std::vector<int> &cluster_ids,
                   const std::array<Double2D, kVertexCount> &points,
                   const DoubleMatrix &distance_matrix,
                   std::vector<int> &assignment) {
    // Base case: only one cluster remains.
    if(cluster_ids.size() == 1) {
        if((int)vs.size() != input.cluster_sizes[cluster_ids[0]]) {
            UNEXPECTED();
        }
        for(int v : vs) {
            assignment[v] = cluster_ids[0];
        }
        return true;
    }

    // Build the current target sizes vector (for the clusters in cluster_ids)
    std::vector<int> current_target_sizes;
    for(int cluster_id : cluster_ids) {
        current_target_sizes.push_back(input.cluster_sizes[cluster_id]);
    }

    int total = std::accumulate(current_target_sizes.begin(),
                                current_target_sizes.end(), 0);

    SubsetSumSolver subset_sum_solver;
    subset_sum_solver.Solve(current_target_sizes, int(vs.size()));

    std::vector<Edge> undirected_mst_edges;
    for(const auto &edge : mst_edges) {
        undirected_mst_edges.push_back(Edge(edge.s, edge.t));
        undirected_mst_edges.push_back(Edge(edge.t, edge.s));
    }
    Graph graph(kVertexCount, undirected_mst_edges);

    std::vector<int> parents(kVertexCount, -1);
    std::vector<int> sizes(kVertexCount, -1);

    auto dfs = [&](auto &&dfs, const int v, const int p) -> void {
        sizes[v] = 1;
        for(const Edge &edge : graph.OutEdges(v)) {
            if(edge.t == p)
                continue;
            parents[edge.t] = v;
            dfs(dfs, edge.t, v);
            sizes[v] += sizes[edge.t];
        }
    };
    dfs(dfs, vs[0], -1);

    // Sort MST edges in descending order of weight (using squared Euclidean
    // distance).
    std::vector<Edge> sorted_edges = mst_edges;
    auto edge_weight = [&](const Edge &e) -> double {
        return distance_matrix.Get(e.s, e.t);
    };
    sort(sorted_edges.begin(), sorted_edges.end(),
         [&](const Edge &a, const Edge &b) {
             return edge_weight(a) > edge_weight(b);
         });

    // Try removing heavy edges and check if the resulting split fits a valid
    // partition.
    for(const auto &target_edge : sorted_edges) {
        int v0 = target_edge.s;
        int v1 = target_edge.t;
        if(parents[v1] == v0) {
            // Do nothing.
        } else if(parents[v0] == v1) {
            std::swap(v0, v1);
        } else {
            UNEXPECTED();
        }

        const int size_0 = int(vs.size()) - sizes[v1];
        const int size_1 = sizes[v1];
        if(!subset_sum_solver.SubsetExists(size_0)) {
            continue;
        }

        // Retrieve subset indices (in current_target_sizes) that sum to size0.
        std::vector<int> cluster_indexes_0 =
            subset_sum_solver.GetSubset(size_0);
        std::vector<int> cluster_ids_0;
        for(const int cluster_index : cluster_indexes_0) {
            cluster_ids_0.push_back(cluster_ids[cluster_index]);
        }

        // Partition cluster_ids into two groups: one for comp0 and one for comp1.
        std::vector<int> in_cluster_ids_0(input.cluster_count, 0);
        for(const int cluster_id : cluster_ids_0) {
            in_cluster_ids_0[cluster_id] = 1;
        }
        std::vector<int> cluster_ids_1;
        for(const int cluster_id : cluster_ids) {
            if(!in_cluster_ids_0[cluster_id]) {
                cluster_ids_1.push_back(cluster_id);
            }
        }

        // Get vertices in each component using DFS
        std::vector<int> comp0, comp1;
        auto get_subtree_vertices =
            [&](auto &&get_subtree_vertices, const int v, const int p,
                std::vector<int> &subtree_vertices) -> void {
            subtree_vertices.push_back(v);
            for(const auto &edge : graph.OutEdges(v)) {
                if(edge.t != p) {
                    get_subtree_vertices(get_subtree_vertices, edge.t, v,
                                         subtree_vertices);
                }
            }
        };
        get_subtree_vertices(get_subtree_vertices, v1, v0, comp1);

        // Get component 0 vertices as complement of component 1
        std::vector<int> in_comp1(kVertexCount);
        for(const int v : comp1) {
            in_comp1[v] = 1;
        }
        for(const int v : vs) {
            if(!in_comp1[v]) {
                comp0.push_back(v);
            }
        }

        // Extract MST edges corresponding to each connected component.
        std::vector<Edge> mst_edges0, mst_edges1;
        ExtractMstEdges(comp0, mst_edges, mst_edges0);
        ExtractMstEdges(comp1, mst_edges, mst_edges1);

        // Recurse on both components.
        if(RunClustering(input, comp0, mst_edges0, cluster_ids_0, points,
                         distance_matrix, assignment) &&
           RunClustering(input, comp1, mst_edges1, cluster_ids_1, points,
                         distance_matrix, assignment)) {
            return true;
        } else {
            UNEXPECTED();
        }
    }

    // No valid split found.
    int target_size = 0;
    for(int size = int(vs.size()) / 2; size >= 1; size--) {
        if(subset_sum_solver.SubsetExists(size)) {
            target_size = size;
            break;
        }
    }
    if(target_size == 0) {
        UNEXPECTED();
    }

    std::vector<int> cluster_indexes_0 =
        subset_sum_solver.GetSubset(target_size);
    std::vector<int> cluster_ids_0;
    for(const int cluster_index : cluster_indexes_0) {
        cluster_ids_0.push_back(cluster_ids[cluster_index]);
    }

    // Partition cluster_ids into two groups: one for comp0 and one for comp1.
    std::vector<int> in_cluster_ids_0(input.cluster_count, 0);
    for(const int cluster_id : cluster_ids_0) {
        in_cluster_ids_0[cluster_id] = 1;
    }
    std::vector<int> cluster_ids_1;
    for(const int cluster_id : cluster_ids) {
        if(!in_cluster_ids_0[cluster_id]) {
            cluster_ids_1.push_back(cluster_id);
        }
    }

    // for (const int i : std::views::iota(0, int(cluster_ids.size()) / 2)) {
    //   target_size += input.cluster_sizes[cluster_ids[i]];
    //   cluster_ids_0.push_back(cluster_ids[i]);
    // }
    // for (const int i :
    //      std::views::iota(int(cluster_ids.size()) / 2, int(cluster_ids.size()))) {
    //   cluster_ids_1.push_back(cluster_ids[i]);
    // }

    std::vector<int> sorted_vertices = vs;
    std::sort(sorted_vertices.begin(), sorted_vertices.end(),
              [&points](int v0, int v1) {
                  if(points[v0].x != points[v1].x) {
                      return points[v0].x < points[v1].x;
                  }
                  return points[v0].y < points[v1].y;
              });

    std::vector<int> vs_0(sorted_vertices.begin(),
                          sorted_vertices.begin() + target_size);
    std::vector<int> vs_1(sorted_vertices.begin() + target_size,
                          sorted_vertices.end());

    // Do K-means.
    for(const int iter : std::views::iota(0, 10)) {
        // Calculate the mean (kernel) of vs_0 and vs_1
        Double2D kernel_0 = {0.0, 0.0};
        Double2D kernel_1 = {0.0, 0.0};

        // Calculate kernel_0 (mean of vs_0)
        for(const int v : vs_0) {
            kernel_0.x += points[v].x;
            kernel_0.y += points[v].y;
        }
        if(!vs_0.empty()) {
            kernel_0.x /= vs_0.size();
            kernel_0.y /= vs_0.size();
        }

        // Calculate kernel_1 (mean of vs_1)
        for(const int v : vs_1) {
            kernel_1.x += points[v].x;
            kernel_1.y += points[v].y;
        }
        if(!vs_1.empty()) {
            kernel_1.x /= vs_1.size();
            kernel_1.y /= vs_1.size();
        }

        // Sort vertices by the difference in distance to the two kernels
        std::vector<std::pair<double, int>> distance_diffs;
        for(const int v : sorted_vertices) {
            double dist_to_kernel_0 = Distance2(points[v], kernel_0);
            double dist_to_kernel_1 = Distance2(points[v], kernel_1);
            double diff = dist_to_kernel_0 - dist_to_kernel_1;
            distance_diffs.push_back({diff, v});
        }

        // Sort by difference in distance (vertices closer to kernel_0 will come
        // first)
        std::sort(distance_diffs.begin(), distance_diffs.end());

        // Update vs_0 and vs_1 based on the sorted differences
        vs_0.clear();
        vs_1.clear();

        // Take the first target_size vertices for vs_0
        for(int i = 0; i < target_size; ++i) {
            vs_0.push_back(distance_diffs[i].second);
        }

        // Take the remaining vertices for vs_1
        for(size_t i = target_size; i < distance_diffs.size(); ++i) {
            vs_1.push_back(distance_diffs[i].second);
        }
    }
    std::vector<Edge> mst_0 = FindMST(vs_0, distance_matrix);
    std::vector<Edge> mst_1 = FindMST(vs_1, distance_matrix);
    if(RunClustering(input, vs_0, mst_0, cluster_ids_0, points, distance_matrix,
                     assignment) &&
       RunClustering(input, vs_1, mst_1, cluster_ids_1, points, distance_matrix,
                     assignment)) {
        return true;
    }

    UNEXPECTED();
    return false;
}

void RunClusteringPhase(const ProblemInput &input,
                        const std::vector<int> &reserved_isolated_vertices,
                        const std::array<Double2D, kVertexCount> &points,
                        const DoubleMatrix &distance_matrix,
                        std::vector<int> &assignment) {
    std::array<int, kVertexCount> v_to_reserved{};
    for(const int v : reserved_isolated_vertices) {
        v_to_reserved[v] = 1;
    }

    int reserved_cluster_count = 0;
    std::vector<int> cluster_to_reserved(input.cluster_count);
    if(reserved_isolated_vertices.size() > 0) {
        for(const int cluster_id : std::views::iota(0, input.cluster_count)) {
            if(input.cluster_sizes[cluster_id] == 1) {
                cluster_to_reserved[cluster_id] = 1;
                assignment[reserved_isolated_vertices[reserved_cluster_count]] =
                    cluster_id;
                reserved_cluster_count++;
                if(reserved_cluster_count == reserved_isolated_vertices.size()) {
                    break;
                }
            }
        }
    }

    std::vector<int> vs;
    vs.reserve(kVertexCount);
    for(const int i : std::views::iota(0, kVertexCount)) {
        if(v_to_reserved[i]) {
            continue;
        }
        vs.push_back(i);
    }
    std::vector<Edge> mst_edges = FindMST(vs, distance_matrix);
    std::vector<int> cluster_ids;
    cluster_ids.reserve(input.cluster_count);
    for(const int i : std::views::iota(0, input.cluster_count)) {
        if(cluster_to_reserved[i]) {
            continue;
        }
        cluster_ids.push_back(i);
    }

    if(!RunClustering(input, vs, mst_edges, cluster_ids, points, distance_matrix,
                      assignment)) {
        UNEXPECTED();
    }
}

#endif // CLUSTERING_CPP_

#ifndef CLUSTERING_ITERATION_CPP
#define CLUSTERING_ITERATION_CPP

#include <cmath>

constexpr double kClusteringIterationTimeLimit = 1900.0;

// Assumption: score is to maximize.
bool AcceptAnnealing(const double score_delta, const double temperature) {
    if(temperature == 0.0) {
        return score_delta >= 0.0;
    }
    if(score_delta > 0)
        return true;
    if(score_delta < -3.0 * temperature)
        return false;
    const double probability = std::exp(score_delta / temperature);
    return g_random.RandomDouble(0.0, 1.0) < probability;
}

double ExpInterpolation(const double x0, const double y0, const double x1,
                        const double y1, const double x) {
    const double log_y0 = std::log(y0);
    const double log_y1 = std::log(y1);
    const double log_y = log_y0 + (x - x0) * (log_y1 - log_y0) / (x1 - x0);
    return std::exp(log_y);
}

struct ClusteringIterationConfig {
    std::array<Double2D, kVertexCount> points;
    DoubleMatrix distance_matrix;
    ClusteringIterationConfig(const std::array<Double2D, kVertexCount> &points,
                              const DoubleMatrix &distance_matrix)
        : points(points), distance_matrix(distance_matrix) {
    }
};

struct ClusteringIterationState {
    std::vector<int> assignment;
    std::vector<std::vector<int>> cluster_to_vs;
    std::vector<std::vector<Edge>> cluster_to_mst;
    std::vector<double> cluster_to_mst_weight_sum;
    std::vector<double> cluster_to_mst_weight_max;
    std::vector<std::vector<Edge>> v_to_edges;
    double score = 0.0;
    ClusteringIterationState(const int cluster_count)
        : assignment(kVertexCount),
          cluster_to_vs(cluster_count),
          cluster_to_mst(cluster_count),
          cluster_to_mst_weight_sum(cluster_count),
          cluster_to_mst_weight_max(cluster_count),
          v_to_edges(kVertexCount) {
    }
};

void SetClusterToState(const ClusteringIterationConfig &config,
                       const int cluster_id, const std::vector<int> &vs,
                       const std::vector<Edge> &mst,
                       ClusteringIterationState &state) {
    state.score -= state.cluster_to_mst_weight_sum[cluster_id];

    for(const int v : vs) {
        state.assignment[v] = cluster_id;
    }
    state.cluster_to_vs[cluster_id] = vs;
    state.cluster_to_mst[cluster_id] = mst;
    for(const int v : vs) {
        state.v_to_edges[v].clear();
    }
    double weight_sum = 0.0;
    double weight_max = 0.0;
    for(const Edge &e : mst) {
        state.v_to_edges[e.s].push_back(e);
        state.v_to_edges[e.t].push_back(Edge(e.t, e.s));
        weight_sum += config.distance_matrix.Get(e.s, e.t);
        weight_max = std::max(weight_max, config.distance_matrix.Get(e.s, e.t));
    }
    state.cluster_to_mst_weight_sum[cluster_id] = weight_sum;
    state.cluster_to_mst_weight_max[cluster_id] = weight_max;

    state.score += weight_sum;
}

// The result of clustering into 2 clusters.
struct Clustering2Result {
    std::vector<int> vs_0;
    std::vector<int> vs_1;
    std::vector<Edge> mst_0;
    std::vector<Edge> mst_1;
    // The sum of weights of mst_0 and mst_1.
    double score;
};

std::vector<std::pair<int, int>> GetClosePairs(
    const DoubleMatrix &distance_matrix) {
    std::vector<std::pair<int, int>> close_pairs;
    // For each vertex, find the 10 closest vertices.
    for(int v = 0; v < kVertexCount; v++) {
        // Create a vector of pairs (distance, vertex_id) for sorting
        std::vector<std::pair<double, int>> distance_v_pairs;
        for(int u = 0; u < kVertexCount; u++) {
            if(v == u)
                continue; // Skip self
            distance_v_pairs.push_back({distance_matrix.Get(v, u), u});
        }

        // Sort by distance (ascending)
        std::sort(distance_v_pairs.begin(), distance_v_pairs.end());

        // Take the top 10 closest vertices.
        for(const int i : std::views::iota(0, 10)) {
            int u = distance_v_pairs[i].second;
            // Store the vertex pair (smaller index first for consistency)
            if(v < u) {
                close_pairs.push_back({v, u});
            } else {
                close_pairs.push_back({u, v});
            }
        }
    }
    // Remove duplicate pairs
    std::sort(close_pairs.begin(), close_pairs.end());
    close_pairs.erase(std::unique(close_pairs.begin(), close_pairs.end()),
                      close_pairs.end());
    return close_pairs;
}

double EdgesWeight(const ClusteringIterationConfig &config,
                   const std::vector<Edge> &edges) {
    double w = 0.0;
    for(const Edge &e : edges) {
        w += config.distance_matrix.Get(e.s, e.t);
    }
    return w;
}

bool TryVertexSwap(const ClusteringIterationConfig &config,
                   const double temperature, ClusteringIterationState &state,
                   const int v0, const int c0, const int v1, const int c1) {
    if(state.v_to_edges[v0].size() != 1 || state.v_to_edges[v1].size() != 1) {
        return false;
    }

    int p0 = state.v_to_edges[v0][0].t;
    int p1 = state.v_to_edges[v1][0].t;

    const double d00 = config.distance_matrix.Get(v0, p0);
    const double d01 = config.distance_matrix.Get(v0, p1);
    const double d10 = config.distance_matrix.Get(v1, p0);
    const double d11 = config.distance_matrix.Get(v1, p1);
    const double score_delta = (d01 + d10) - (d00 + d11);
    if(!AcceptAnnealing(-score_delta, temperature)) {
        return false;
    }

    std::vector<int> vs_0 = state.cluster_to_vs[c0];
    std::vector<int> vs_1 = state.cluster_to_vs[c1];
    vs_0.erase(std::find(vs_0.begin(), vs_0.end(), v0));
    vs_0.push_back(v1);
    vs_1.erase(std::find(vs_1.begin(), vs_1.end(), v1));
    vs_1.push_back(v0);

    const auto mst0 = FindMST(vs_0, config.distance_matrix);
    const auto mst1 = FindMST(vs_1, config.distance_matrix);
    SetClusterToState(config, c0, vs_0, mst0, state);
    SetClusterToState(config, c1, vs_1, mst1, state);
    return true;
}

std::optional<Clustering2Result> MaybeDoClustering2ByMergedMST(
    const ClusteringIterationConfig &config, const std::vector<int> &vs,
    const int target_size_0, const int target_size_1) {
    const auto mst = FindMST(vs, config.distance_matrix);
    const double merged_mst_w = EdgesWeight(config, mst);

    static std::vector<int> parents(kVertexCount, -1);
    static std::vector<int> sizes(kVertexCount, -1);
    static std::vector<std::vector<Edge>> v_to_merged_mst_edges(kVertexCount);
    // Reset static variables before use.
    for(const int v : vs) {
        parents[v] = -1;
        sizes[v] = -1;
        v_to_merged_mst_edges[v].clear();
    }
    for(const Edge &e : mst) {
        v_to_merged_mst_edges[e.s].push_back(e);
        v_to_merged_mst_edges[e.t].push_back(Edge(e.t, e.s));
    }

    auto dfs = [&](auto &&dfs, const int v, const int p) -> void {
        sizes[v] = 1;
        for(const Edge &edge : v_to_merged_mst_edges[v]) {
            if(edge.t == p)
                continue;
            parents[edge.t] = v;
            dfs(dfs, edge.t, v);
            sizes[v] += sizes[edge.t];
        }
    };
    dfs(dfs, vs[0], -1);

    // Sort MST edges in descending order of weight (using squared Euclidean
    // distance).
    std::vector<Edge> sorted_edges = mst;
    auto edge_weight = [&](const Edge &e) -> double {
        return config.distance_matrix.Get(e.s, e.t);
    };
    sort(sorted_edges.begin(), sorted_edges.end(),
         [&](const Edge &a, const Edge &b) {
             return edge_weight(a) > edge_weight(b);
         });

    // Try removing heavy edges and check if the resulting split fits a valid
    // partition.
    for(const auto &target_edge : sorted_edges) {
        int v0 = target_edge.s;
        int v1 = target_edge.t;
        if(parents[v1] == v0) {
            // Do nothing.
        } else if(parents[v0] == v1) {
            std::swap(v0, v1);
        } else {
            UNEXPECTED();
        }

        const int size_0 = int(vs.size()) - sizes[v1];
        const int size_1 = sizes[v1];
        if(!((size_0 == target_size_0 && size_1 == target_size_1) ||
             (size_0 == target_size_1 && size_1 == target_size_0))) {
            continue;
        }

        std::vector<int> component_0, component_1;
        auto get_subtree_vertices =
            [&](auto &&get_subtree_vertices, const int v, const int p,
                std::vector<int> &subtree_vertices) -> void {
            subtree_vertices.push_back(v);
            for(const auto &edge : v_to_merged_mst_edges[v]) {
                if(edge.t != p) {
                    get_subtree_vertices(get_subtree_vertices, edge.t, v,
                                         subtree_vertices);
                }
            }
        };
        get_subtree_vertices(get_subtree_vertices, v1, v0, component_1);

        // Get component 0 vertices as complement of component 1
        static std::vector<int> in_comp1(kVertexCount);
        // Reset before use.
        for(const int v : vs) {
            in_comp1[v] = 0;
        }
        for(const int v : component_1) {
            in_comp1[v] = 1;
        }
        for(const int v : vs) {
            if(!in_comp1[v]) {
                component_0.push_back(v);
            }
        }

        if(size_0 != target_size_0) {
            std::swap(component_0, component_1);
        }

        std::vector<Edge> mst0 = FindMST(component_0, config.distance_matrix);
        std::vector<Edge> mst1 = FindMST(component_1, config.distance_matrix);
        return Clustering2Result{
            component_0, component_1, mst0, mst1,
            EdgesWeight(config, mst0) + EdgesWeight(config, mst1)};
    }
    return std::nullopt;
}

bool TryUpdateTwoClustersFromMergedMST(const ClusteringIterationConfig &config,
                                       const double temperature, const int c0,
                                       const int c1,
                                       ClusteringIterationState &state) {
    const int size_0 = state.cluster_to_vs[c0].size();
    const int size_1 = state.cluster_to_vs[c1].size();

    const double base_w =
        state.cluster_to_mst_weight_sum[c0] + state.cluster_to_mst_weight_sum[c1];

    std::vector<int> vs = state.cluster_to_vs[c0];
    vs.insert(vs.end(), state.cluster_to_vs[c1].begin(),
              state.cluster_to_vs[c1].end());

    std::optional<Clustering2Result> result =
        MaybeDoClustering2ByMergedMST(config, vs, size_0, size_1);
    if(!result) {
        return false;
    }

    const double new_w = result->score;
    const double score_delta = new_w - base_w;

    // Likely no change.
    // TODO: Consider proper implementation.
    if(std::abs(score_delta) < 0.001) {
        return false;
    }

    if(!AcceptAnnealing(-score_delta, temperature)) {
        return false;
    }

    SetClusterToState(config, c0, result->vs_0, result->mst_0, state);
    SetClusterToState(config, c1, result->vs_1, result->mst_1, state);
    return true;
}

Clustering2Result DoClustering2ByKMeans(const ClusteringIterationConfig &config,
                                        const std::vector<int> &vs,
                                        const int size_0, const int size_1) {
    std::vector<int> shuffled_vs = vs;
    const double theta = g_random.RandomDouble(0.0, 2.0 * M_PI);
    // Sort vertices by projecting them onto a random line with angle theta
    std::sort(shuffled_vs.begin(), shuffled_vs.end(), [&](int a, int b) {
        const double z_a = config.points[a].x * std::cos(theta) +
                           config.points[a].y * std::sin(theta);
        const double z_b = config.points[b].x * std::cos(theta) +
                           config.points[b].y * std::sin(theta);
        return z_a < z_b;
    });
    // for (const int i : std::views::iota(0, int(vs.size()) - 1)) {
    //   int j = g_random.RandomInt(i, vs.size());
    //   std::swap(shuffled_vs[i], shuffled_vs[j]);
    // }

    std::vector<int> vs_0;
    std::vector<int> vs_1;
    for(const int i : std::views::iota(0, size_0)) {
        vs_0.push_back(shuffled_vs[i]);
    }
    for(const int i : std::views::iota(size_0, size_0 + size_1)) {
        vs_1.push_back(shuffled_vs[i]);
    }

    for(const int iter : std::views::iota(0, 10)) {
        // Calculate the mean (kernel) of vs_0 and vs_1
        Double2D kernel_0 = {0.0, 0.0};
        Double2D kernel_1 = {0.0, 0.0};

        // Calculate kernel_0 (mean of vs_0)
        for(const int v : vs_0) {
            kernel_0.x += config.points[v].x;
            kernel_0.y += config.points[v].y;
        }
        if(!vs_0.empty()) {
            kernel_0.x /= vs_0.size();
            kernel_0.y /= vs_0.size();
        }

        // Calculate kernel_1 (mean of vs_1)
        for(const int v : vs_1) {
            kernel_1.x += config.points[v].x;
            kernel_1.y += config.points[v].y;
        }
        if(!vs_1.empty()) {
            kernel_1.x /= vs_1.size();
            kernel_1.y /= vs_1.size();
        }
        // Sort vertices by the difference in distance to the two kernels
        std::vector<std::pair<double, int>> distance_diffs;
        for(const int v : vs) {
            double dist_to_kernel_0 = Distance2(config.points[v], kernel_0);
            double dist_to_kernel_1 = Distance2(config.points[v], kernel_1);
            double diff = dist_to_kernel_0 - dist_to_kernel_1;
            distance_diffs.push_back({diff, v});
        }

        // Sort by difference in distance (vertices closer to kernel_0 will come
        // first)
        std::sort(distance_diffs.begin(), distance_diffs.end());

        // Update vs_0 and vs_1 based on the sorted differences
        vs_0.clear();
        vs_1.clear();

        // Take the first target_size vertices for vs_0
        for(int i = 0; i < size_0; ++i) {
            vs_0.push_back(distance_diffs[i].second);
        }

        // Take the remaining vertices for vs_1
        for(size_t i = size_0; i < distance_diffs.size(); ++i) {
            vs_1.push_back(distance_diffs[i].second);
        }
    }

    const auto mst0 = FindMST(vs_0, config.distance_matrix);
    const auto mst1 = FindMST(vs_1, config.distance_matrix);
    double new_w = EdgesWeight(config, mst0) + EdgesWeight(config, mst1);
    return {vs_0, vs_1, mst0, mst1, new_w};
}

bool TryUpdateTwoClustersFromKMeans(const ClusteringIterationConfig &config,
                                    const double temperature, const int c0,
                                    const int c1,
                                    ClusteringIterationState &state) {
    std::vector<int> vs = state.cluster_to_vs[c0];
    vs.insert(vs.end(), state.cluster_to_vs[c1].begin(),
              state.cluster_to_vs[c1].end());
    const Clustering2Result result =
        DoClustering2ByKMeans(config, vs, state.cluster_to_vs[c0].size(),
                              state.cluster_to_vs[c1].size());

    const double base_w = EdgesWeight(config, state.cluster_to_mst[c0]) +
                          EdgesWeight(config, state.cluster_to_mst[c1]);
    double new_w = result.score;
    const double score_delta = new_w - base_w;

    // Likely no change.
    // TODO: Consider proper implementation.
    if(std::abs(score_delta) < 0.001) {
        return false;
    }

    if(!AcceptAnnealing(-score_delta, temperature)) {
        return false;
    }

    SetClusterToState(config, c0, result.vs_0, result.mst_0, state);
    SetClusterToState(config, c1, result.vs_1, result.mst_1, state);
    return true;
}

// Assumption: size(c0) + size(c1) == size(c2)
bool TryUpdateByOneTwoClusterSwap(const ClusteringIterationConfig &config,
                                  const double temperature, const int c0,
                                  const int c1, const int c2,
                                  ClusteringIterationState &state) {
    const double base_w = state.cluster_to_mst_weight_sum[c0] +
                          state.cluster_to_mst_weight_sum[c1] +
                          state.cluster_to_mst_weight_sum[c2];

    std::vector<int> vs_2 = state.cluster_to_vs[c0];
    vs_2.insert(vs_2.end(), state.cluster_to_vs[c1].begin(),
                state.cluster_to_vs[c1].end());
    std::vector<Edge> mst_2 = FindMST(vs_2, config.distance_matrix);

    std::vector<int> vs = state.cluster_to_vs[c2];

    std::optional<Clustering2Result> opt_merged_mst_result =
        MaybeDoClustering2ByMergedMST(config, vs, state.cluster_to_vs[c0].size(),
                                      state.cluster_to_vs[c1].size());
    if(opt_merged_mst_result) {
        const double new_w =
            opt_merged_mst_result->score + EdgesWeight(config, mst_2);
        const double score_delta = new_w - base_w;
        if(AcceptAnnealing(-score_delta, temperature)) {
            SetClusterToState(config, c0, opt_merged_mst_result->vs_0,
                              opt_merged_mst_result->mst_0, state);
            SetClusterToState(config, c1, opt_merged_mst_result->vs_1,
                              opt_merged_mst_result->mst_1, state);
            SetClusterToState(config, c2, vs_2, mst_2, state);
            return true;
        }
    }

    Clustering2Result k_means_result =
        DoClustering2ByKMeans(config, vs, state.cluster_to_vs[c0].size(),
                              state.cluster_to_vs[c1].size());
    const double new_w = k_means_result.score + EdgesWeight(config, mst_2);
    const double score_delta = new_w - base_w;
    if(AcceptAnnealing(-score_delta, temperature)) {
        SetClusterToState(config, c0, k_means_result.vs_0, k_means_result.mst_0,
                          state);
        SetClusterToState(config, c1, k_means_result.vs_1, k_means_result.mst_1,
                          state);
        SetClusterToState(config, c2, vs_2, mst_2, state);
        return true;
    }

    return false;
}

void ExtractMstEdges2(const std::vector<int> &component,
                      const std::vector<Edge> &mst_edges,
                      std::vector<Edge> &component_mst_edges) {
    for(const auto &edge : mst_edges) {
        if(find(component.begin(), component.end(), edge.s) != component.end() &&
           find(component.begin(), component.end(), edge.t) != component.end())
            component_mst_edges.push_back(edge);
    }
}

// target_clusters: The target clusters to compute in this multi-clustering.
// cluster_indexes: The indexes of the clusters to handle in this specific
// call. The index points to a element in target_clusters.
void DoMultiClustering(
    const ProblemInput &input,
    const ClusteringIterationConfig &config,
    const std::vector<int> &target_clusters,
    const std::vector<int> &vs,
    const std::vector<int> &cluster_indexes,
    std::vector<std::vector<int>> &cluster_index_to_vs,
    std::vector<std::vector<Edge>> &cluster_index_to_mst) {
    if(cluster_indexes.size() == 1) {
        const int cluster_index = cluster_indexes[0];
        const std::vector<Edge> mst = FindMST(vs, config.distance_matrix);
        cluster_index_to_vs[cluster_index] = vs;
        cluster_index_to_mst[cluster_index] = mst;
        return;
    }

    if(cluster_indexes.size() == 2) {
        const int size_0 = input.cluster_sizes[target_clusters[cluster_indexes[0]]];
        const int size_1 = input.cluster_sizes[target_clusters[cluster_indexes[1]]];
        std::optional<Clustering2Result> opt_merged_mst_result =
            MaybeDoClustering2ByMergedMST(config, vs, size_0, size_1);
        if(opt_merged_mst_result) {
            cluster_index_to_vs[cluster_indexes[0]] = opt_merged_mst_result->vs_0;
            cluster_index_to_vs[cluster_indexes[1]] = opt_merged_mst_result->vs_1;
            cluster_index_to_mst[cluster_indexes[0]] = opt_merged_mst_result->mst_0;
            cluster_index_to_mst[cluster_indexes[1]] = opt_merged_mst_result->mst_1;
            return;
        }
        // if (vs.size() != size_0 + size_1) {
        //   std::cerr << "vs.size(): " << vs.size() << std::endl;
        //   std::cerr << "size_0: " << size_0 << std::endl;
        //   std::cerr << "size_1: " << size_1 << std::endl;
        //   UNEXPECTED();
        // }
        Clustering2Result k_means_result =
            DoClustering2ByKMeans(config, vs, size_0, size_1);
        cluster_index_to_vs[cluster_indexes[0]] = k_means_result.vs_0;
        cluster_index_to_vs[cluster_indexes[1]] = k_means_result.vs_1;
        cluster_index_to_mst[cluster_indexes[0]] = k_means_result.mst_0;
        cluster_index_to_mst[cluster_indexes[1]] = k_means_result.mst_1;
        return;
    }

    // More than 3 clusters.
    // std::vector<int> cluster_ids;
    // for (const int cluster_index : cluster_indexes) {
    //   cluster_ids.push_back(target_clusters[cluster_index]);
    // }

    std::vector<int> current_target_sizes;
    for(const int cluster_index : cluster_indexes) {
        current_target_sizes.push_back(input.cluster_sizes[target_clusters[cluster_index]]);
    }
    // std::cerr << "vs.size(): " << vs.size() << std::endl;
    // std::cerr << "current_target_sizes: " << current_target_sizes << std::endl;

    int total = std::accumulate(current_target_sizes.begin(),
                                current_target_sizes.end(), 0);

    SubsetSumSolver subset_sum_solver;
    subset_sum_solver.Solve(current_target_sizes, int(vs.size()));

    // TODO: Add Merged MST approach here.
    {
        const auto mst_edges = FindMST(vs, config.distance_matrix);
        std::vector<Edge> undirected_mst_edges;
        for(const auto &edge : mst_edges) {
            undirected_mst_edges.push_back(Edge(edge.s, edge.t));
            undirected_mst_edges.push_back(Edge(edge.t, edge.s));
        }
        Graph graph(kVertexCount, undirected_mst_edges);

        std::vector<int> parents(kVertexCount, -1);
        std::vector<int> sizes(kVertexCount, -1);

        auto dfs = [&](auto &&dfs, const int v, const int p) -> void {
            sizes[v] = 1;
            for(const Edge &edge : graph.OutEdges(v)) {
                if(edge.t == p)
                    continue;
                parents[edge.t] = v;
                dfs(dfs, edge.t, v);
                sizes[v] += sizes[edge.t];
            }
        };
        dfs(dfs, vs[0], -1);
        // Sort MST edges in descending order of weight (using squared Euclidean
        // distance).
        std::vector<Edge> sorted_edges = mst_edges;
        auto edge_weight = [&](const Edge &e) -> double {
            return config.distance_matrix.Get(e.s, e.t);
        };
        sort(sorted_edges.begin(), sorted_edges.end(),
             [&](const Edge &a, const Edge &b) {
                 return edge_weight(a) > edge_weight(b);
             });

        for(const auto &target_edge : sorted_edges) {
            int v0 = target_edge.s;
            int v1 = target_edge.t;
            if(parents[v1] == v0) {
                // Do nothing.
            } else if(parents[v0] == v1) {
                std::swap(v0, v1);
            } else {
                UNEXPECTED();
            }

            const int size_0 = int(vs.size()) - sizes[v1];
            const int size_1 = sizes[v1];
            if(!subset_sum_solver.SubsetExists(size_0)) {
                continue;
            }

            std::vector<int> cluster_local_indexes_0 =
                subset_sum_solver.GetSubset(size_0);

            // Partition cluster_ids into two groups: one for comp0 and one for
            // comp1.
            std::vector<int> in_cluster_local_indexes_0(target_clusters.size(), 0);
            for(const int cluster_local_index : cluster_local_indexes_0) {
                in_cluster_local_indexes_0[cluster_local_index] = 1;
            }
            std::vector<int> cluster_local_indexes_1;
            for(const int cluster_local_index :
                std::views::iota(0, int(cluster_indexes.size()))) {
                if(!in_cluster_local_indexes_0[cluster_local_index]) {
                    cluster_local_indexes_1.push_back(cluster_local_index);
                }
            }

            // Get vertices in each component using DFS
            std::vector<int> comp0, comp1;
            auto get_subtree_vertices =
                [&](auto &&get_subtree_vertices, const int v, const int p,
                    std::vector<int> &subtree_vertices) -> void {
                subtree_vertices.push_back(v);
                for(const auto &edge : graph.OutEdges(v)) {
                    if(edge.t != p) {
                        get_subtree_vertices(get_subtree_vertices, edge.t, v,
                                             subtree_vertices);
                    }
                }
            };
            get_subtree_vertices(get_subtree_vertices, v1, v0, comp1);

            // Get component 0 vertices as complement of component 1
            std::vector<int> in_comp1(kVertexCount);
            for(const int v : comp1) {
                in_comp1[v] = 1;
            }
            for(const int v : vs) {
                if(!in_comp1[v]) {
                    comp0.push_back(v);
                }
            }

            std::vector<int> cluster_indexes_0;
            for(const int cluster_local_index : cluster_local_indexes_0) {
                cluster_indexes_0.push_back(cluster_indexes[cluster_local_index]);
            }
            std::vector<int> cluster_indexes_1;
            for(const int cluster_local_index : cluster_local_indexes_1) {
                cluster_indexes_1.push_back(cluster_indexes[cluster_local_index]);
            }

            DoMultiClustering(input, config, target_clusters, comp0,
                              cluster_indexes_0, cluster_index_to_vs,
                              cluster_index_to_mst);
            DoMultiClustering(input, config, target_clusters, comp1,
                              cluster_indexes_1, cluster_index_to_vs,
                              cluster_index_to_mst);
            return;
        }
    }

    int target_size = 0;
    for(int size = int(vs.size()) / 2; size >= 1; size--) {
        if(subset_sum_solver.SubsetExists(size)) {
            target_size = size;
            break;
        }
    }
    if(target_size == 0) {
        UNEXPECTED();
    }

    std::vector<int> cluster_local_indexes_0 =
        subset_sum_solver.GetSubset(target_size);

    // Partition cluster_ids into two groups: one for comp0 and one for comp1.
    std::vector<int> in_cluster_local_indexes_0(target_clusters.size(), 0);
    for(const int cluster_local_index : cluster_local_indexes_0) {
        in_cluster_local_indexes_0[cluster_local_index] = 1;
    }
    std::vector<int> cluster_local_indexes_1;
    for(const int cluster_local_index :
        std::views::iota(0, int(cluster_indexes.size()))) {
        if(!in_cluster_local_indexes_0[cluster_local_index]) {
            cluster_local_indexes_1.push_back(cluster_local_index);
        }
    }

    const double theta = g_random.RandomDouble(0.0, 2.0 * M_PI);
    std::vector<int> sorted_vertices = vs;
    std::sort(sorted_vertices.begin(), sorted_vertices.end(),
              [&](int v0, int v1) {
                  const double z_0 = config.points[v0].x * std::cos(theta) +
                                     config.points[v0].y * std::sin(theta);
                  const double z_1 = config.points[v1].x * std::cos(theta) +
                                     config.points[v1].y * std::sin(theta);
                  return z_0 < z_1;
              });

    std::vector<int> vs_0(sorted_vertices.begin(),
                          sorted_vertices.begin() + target_size);
    std::vector<int> vs_1(sorted_vertices.begin() + target_size,
                          sorted_vertices.end());

    // Do K-means.
    for(const int iter : std::views::iota(0, 10)) {
        // Calculate the mean (kernel) of vs_0 and vs_1
        Double2D kernel_0 = {0.0, 0.0};
        Double2D kernel_1 = {0.0, 0.0};

        // Calculate kernel_0 (mean of vs_0)
        for(const int v : vs_0) {
            kernel_0.x += config.points[v].x;
            kernel_0.y += config.points[v].y;
        }
        if(!vs_0.empty()) {
            kernel_0.x /= vs_0.size();
            kernel_0.y /= vs_0.size();
        }

        // Calculate kernel_1 (mean of vs_1)
        for(const int v : vs_1) {
            kernel_1.x += config.points[v].x;
            kernel_1.y += config.points[v].y;
        }
        if(!vs_1.empty()) {
            kernel_1.x /= vs_1.size();
            kernel_1.y /= vs_1.size();
        }

        // Sort vertices by the difference in distance to the two kernels
        std::vector<std::pair<double, int>> distance_diffs;
        for(const int v : sorted_vertices) {
            double dist_to_kernel_0 = Distance2(config.points[v], kernel_0);
            double dist_to_kernel_1 = Distance2(config.points[v], kernel_1);
            double diff = dist_to_kernel_0 - dist_to_kernel_1;
            distance_diffs.push_back({diff, v});
        }

        // Sort by difference in distance (vertices closer to kernel_0 will come
        // first)
        std::sort(distance_diffs.begin(), distance_diffs.end());

        // Update vs_0 and vs_1 based on the sorted differences
        vs_0.clear();
        vs_1.clear();

        // Take the first target_size vertices for vs_0
        for(int i = 0; i < target_size; ++i) {
            vs_0.push_back(distance_diffs[i].second);
        }

        // Take the remaining vertices for vs_1
        for(size_t i = target_size; i < distance_diffs.size(); ++i) {
            vs_1.push_back(distance_diffs[i].second);
        }
    }

    std::vector<int> cluster_indexes_0;
    for(const int cluster_local_index : cluster_local_indexes_0) {
        cluster_indexes_0.push_back(cluster_indexes[cluster_local_index]);
    }
    std::vector<int> cluster_indexes_1;
    for(const int cluster_local_index : cluster_local_indexes_1) {
        cluster_indexes_1.push_back(cluster_indexes[cluster_local_index]);
    }

    DoMultiClustering(input, config, target_clusters, vs_0, cluster_indexes_0,
                      cluster_index_to_vs, cluster_index_to_mst);
    DoMultiClustering(input, config, target_clusters, vs_1, cluster_indexes_1,
                      cluster_index_to_vs, cluster_index_to_mst);
}

bool TryUpdateByMultiClustering(const ProblemInput &input,
                                const ClusteringIterationConfig &config,
                                const std::vector<int> &target_clusters,
                                const std::vector<int> &vs,
                                ClusteringIterationState &state) {
    double base_w = 0.0;
    for(const int cluster_id : target_clusters) {
        base_w += state.cluster_to_mst_weight_sum[cluster_id];
    }

    std::vector<int> cluster_indexes;
    for(const int index : std::views::iota(0, int(target_clusters.size()))) {
        cluster_indexes.push_back(index);
    }

    std::vector<std::vector<int>> cluster_index_to_vs(target_clusters.size());
    std::vector<std::vector<Edge>> cluster_index_to_mst(target_clusters.size());
    DoMultiClustering(input, config, target_clusters, vs, cluster_indexes,
                      cluster_index_to_vs, cluster_index_to_mst);

    double new_w = 0.0;
    for(const int cluster_index : std::views::iota(0, int(target_clusters.size()))) {
        new_w += EdgesWeight(config, cluster_index_to_mst[cluster_index]);
    }
    const double score_delta = new_w - base_w;
    if(AcceptAnnealing(-score_delta, 0.0)) {
        for(const int cluster_index :
            std::views::iota(0, int(target_clusters.size()))) {
            if(state.cluster_to_mst[target_clusters[cluster_index]].size() !=
               cluster_index_to_mst[cluster_index].size()) {
                std::cerr << "state.cluster_to_mst[target_clusters[cluster_index]].size(): " << state.cluster_to_mst[target_clusters[cluster_index]].size() << std::endl;
                std::cerr << "cluster_index_to_mst[cluster_index].size(): " << cluster_index_to_mst[cluster_index].size() << std::endl;
                UNEXPECTED();
            }
            const int cluster_id = target_clusters[cluster_index];
            std::vector<int> vs = cluster_index_to_vs[cluster_index];
            std::vector<Edge> mst = cluster_index_to_mst[cluster_index];
            SetClusterToState(config, cluster_id, vs, mst, state);
        }
        return true;
    }
    return false;
}

void RunClusteringIterationPhase(
    const ProblemInput &input, const std::array<Double2D, kVertexCount> &points,
    const DoubleMatrix &distance_matrix, std::vector<int> &assignment,
    std::vector<std::vector<Edge>> &cluster_to_mst) {
    std::vector<std::pair<int, int>> close_pairs = GetClosePairs(distance_matrix);
    ClusteringIterationConfig config(points, distance_matrix);

    // Initalize state.
    ClusteringIterationState state(input.cluster_count);
    std::vector<std::vector<int>> cluster_to_vs(input.cluster_count);
    for(const int v : std::views::iota(0, kVertexCount)) {
        cluster_to_vs[assignment[v]].push_back(v);
    }
    for(const int cluster_id : std::views::iota(0, input.cluster_count)) {
        SetClusterToState(config, cluster_id, cluster_to_vs[cluster_id],
                          FindMST(cluster_to_vs[cluster_id], distance_matrix),
                          state);
    }

    std::cerr << "initial_score: " << state.score << std::endl;

    // First, do hill climbing by vertex swaps.
    while(g_time.CurrentMs() < kClusteringIterationTimeLimit) {
        bool updated = false;
        for(const auto [v0, v1] : close_pairs) {
            const int c0 = state.assignment[v0];
            const int c1 = state.assignment[v1];
            if(c0 == c1) {
                continue;
            }
            if(TryVertexSwap(config, 0.0, state, v0, c0, v1, c1)) {
                updated = true;
                if(g_time.CurrentMs() > kClusteringIterationTimeLimit) {
                    break;
                }
            }
        }
        if(!updated) {
            break;
        }
    }

    std::cerr << "hill_climbing_score: " << state.score << std::endl;

    const int time_0 = g_time.CurrentMs();
    constexpr double kTemperature0 = 100.0;
    constexpr double kTemperature1 = 10.0;

    ClusteringIterationState best_state = state;
    double best_score = state.score;

    int annealing_transition_count = 0;
    int annealing_update_best_count = 0;
    auto update_best_state = [&]() {
        annealing_transition_count++;
        if(state.score < best_score) {
            best_score = state.score;
            best_state = state;
            annealing_update_best_count++;
        }
    };

    // Preparation for annealing.
    std::vector<std::vector<int>> cluster_size_to_cluster_ids(kVertexCount + 1);
    for(const int cluster_id : std::views::iota(0, input.cluster_count)) {
        cluster_size_to_cluster_ids[input.cluster_sizes[cluster_id]].push_back(
            cluster_id);
    }

    int swap_transition_count = 0;
    int merged_mst_transition_count = 0;
    int kmeans_transition_count = 0;
    int swap_one_two_cluster_transition_count = 0;
    int multi_update_2_count = 0;
    int multi_update_3_count = 0;
    int multi_update_4plus_count = 0;
    int try_update_count = 0;
    int try_update_true_count = 0;

    // The main function for annealing. Returns true if expensive operation is
    // performed.
    auto try_update = [&](int64 time_ms) -> bool {
        try_update_count++;
        const auto [v0, v1] =
            close_pairs[g_random.RandomInt(0, close_pairs.size())];
        const int c0 = state.assignment[v0];
        const int c1 = state.assignment[v1];
        if(c0 == c1) {
            return false;
        }

        const double temperature =
            ExpInterpolation(time_0, kTemperature0, kClusteringIterationTimeLimit,
                             kTemperature1, time_ms);

        if(TryVertexSwap(config, temperature, state, v0, c0, v1, c1)) {
            update_best_state();
            swap_transition_count++;
            try_update_true_count++;
            return true;
        }

        const int size_0 = input.cluster_sizes[c0];
        const int size_1 = input.cluster_sizes[c1];
        if(cluster_size_to_cluster_ids[size_0 + size_1].size() >= 1 &&
           g_random.RandomDouble(0.0, 1.0) < 0.5) {
            const std::vector<int> &c2_candidates =
                cluster_size_to_cluster_ids[size_0 + size_1];
            const int c2 = c2_candidates[g_random.RandomInt(0, c2_candidates.size())];
            const double d = config.distance_matrix.Get(v0, v1);
            if(d < state.cluster_to_mst_weight_max[c2] + temperature) {
                if(TryUpdateByOneTwoClusterSwap(config, temperature, c0, c1, c2,
                                                state)) {
                    update_best_state();
                    swap_one_two_cluster_transition_count++;
                    try_update_true_count++;
                    return true;
                }
                return true;
            }
        }

        // Do quick check to see if merging two clusters is promising. If not,
        // return without executing the expensive operation.
        if(const double d = config.distance_matrix.Get(v0, v1);
           state.cluster_to_mst_weight_max[c0] + temperature < d &&
           state.cluster_to_mst_weight_max[c1] + temperature < d) {
            return false;
        }

        const double r = g_random.RandomDouble(0.0, 1.0);
        if(r < 1.0 / 3.0) {
            if(TryUpdateTwoClustersFromMergedMST(config, temperature, c0, c1,
                                                 state)) {
                update_best_state();
                merged_mst_transition_count++;
            }
        } else {
            if(TryUpdateTwoClustersFromKMeans(config, temperature, c0, c1, state)) {
                update_best_state();
                kmeans_transition_count++;
            }
        }
        try_update_true_count++;
        return true;
    };

    auto try_multi_updte = [&]() -> void {
        const double center_x = g_random.RandomDouble(0.0, 10000.0);
        const double center_y = g_random.RandomDouble(0.0, 10000.0);
        const double radius = g_random.RandomDouble(500.0, 1000.0);
        const double min_x = center_x - radius;
        const double max_x = center_x + radius;
        const double min_y = center_y - radius;
        const double max_y = center_y + radius;
        std::vector<int> clusters;
        for(const int v : std::views::iota(0, kVertexCount)) {
            const double x = config.points[v].x;
            const double y = config.points[v].y;
            if(x >= min_x && x <= max_x && y >= min_y && y <= max_y) {
                clusters.push_back(state.assignment[v]);
            }
        }
        // Sort clusters and remove duplicates
        std::sort(clusters.begin(), clusters.end());
        clusters.erase(std::unique(clusters.begin(), clusters.end()), clusters.end());

        // Skip if we don't have enough clusters to work with
        if(clusters.size() < 3) {
            return;
        }

        // Collect vertices within the radius of the random center
        std::vector<int> vs;
        for(const int cluster_id : clusters) {
            for(const int v : state.cluster_to_vs[cluster_id]) {
                vs.push_back(v);
            }
        }

        if(TryUpdateByMultiClustering(input, config, clusters, vs, state)) {
            if(clusters.size() == 2) {
                multi_update_2_count++;
            } else if(clusters.size() == 3) {
                multi_update_3_count++;
            } else {
                multi_update_4plus_count++;
            }
        }
    };

    // Do annealing with more complex transitions.
    while(true) {
        const int64 time_ms = g_time.CurrentMs();
        if(time_ms > kClusteringIterationTimeLimit) {
            break;
        }

        if(g_random.RandomDouble(0.0, 1.0) < 0.1) {
            try_multi_updte();
            continue;
        }

        constexpr int kLoopCount = 100;
        for(const int i : std::views::iota(0, kLoopCount)) {
            if(try_update(time_ms)) {
                break;
            }
        }
    }

    std::cerr << "iteration_final_score: " << best_score << std::endl;
    std::cerr << std::endl;
    std::cerr << "annealing_transition_count: " << annealing_transition_count
              << std::endl;
    std::cerr << "annealing_update_best_count: " << annealing_update_best_count
              << std::endl;
    std::cerr << std::endl;
    std::cerr << "try_update_count: " << try_update_count << std::endl;
    std::cerr << "try_update_true_count: " << try_update_true_count << std::endl;
    std::cerr << "swap_transition_count: " << swap_transition_count << std::endl;
    std::cerr << "merged_mst_transition_count: " << merged_mst_transition_count
              << std::endl;
    std::cerr << "kmeans_transition_count: " << kmeans_transition_count
              << std::endl;
    std::cerr << "swap_one_two_cluster_transition_count: "
              << swap_one_two_cluster_transition_count << std::endl;
    std::cerr << "multi_update_2_count: " << multi_update_2_count << std::endl;
    std::cerr << "multi_update_3_count: " << multi_update_3_count << std::endl;
    std::cerr << "multi_update_4plus_count: " << multi_update_4plus_count
              << std::endl;
    std::cerr << std::endl;

    assignment = best_state.assignment;
    cluster_to_mst = best_state.cluster_to_mst;
}

#endif // CLUSTERING_ITERATION_CPP

#ifndef FINALIZER_CPP_
#define FINALIZER_CPP_

std::vector<MSTCluster>
FinalizeSolution(const ProblemInput &input, const std::vector<int> &assignment,
                 const std::vector<std::vector<Edge>> &cluster_to_mst) {
    std::vector<MSTCluster> clusters;
    for(const int cluster_id : std::views::iota(0, input.cluster_count)) {
        std::vector<int> vs;
        for(const int v : std::views::iota(0, kVertexCount)) {
            if(assignment[v] == cluster_id) {
                vs.push_back(v);
            }
        }

        std::vector<Edge> mst_edges = cluster_to_mst[cluster_id];
        clusters.emplace_back(vs, mst_edges);
    }
    return clusters;
}

#endif // FINALIZER_CPP_

#ifndef QUERY_PHASE_CPP_
#define QUERY_PHASE_CPP_

#include <set>

#ifndef QUERY_SELECTOR_CPP_
#define QUERY_SELECTOR_CPP_

double ScoreQuery(const int vertex_count, const std::vector<int> &vs,
                  const int sample_count,
                  const std::vector<DoubleMatrix> &distance_matrices,
                  const std::vector<double> &radiuses) {
    DoubleMatrix local_distance_matrix(vs.size(), vs.size());
    DoubleMatrix count_matrix(vs.size(), vs.size());
    std::vector<Edge> mst;
    mst.reserve(vs.size() - 1);
    for(const int sample_id : std::views::iota(0, sample_count)) {
        for(const int i : std::views::iota(0, int(vs.size()))) {
            for(const int j : std::views::iota(i + 1, int(vs.size()))) {
                const double d = distance_matrices[sample_id].Get(vs[i], vs[j]);
                local_distance_matrix.Set(i, j, d);
                local_distance_matrix.Set(j, i, d);
            }
        }
        mst.clear();
        FindMSTOptimized<kMaxQuerySize>(vs.size(), local_distance_matrix, mst);
        // mst = FindMST(vs.size(), local_distance_matrix);
        for(const auto &edge : mst) {
            count_matrix.Increment(edge.s, edge.t, 1);
            count_matrix.Increment(edge.t, edge.s, 1);
        }
    }

    double score = 0.0;
    for(const int i : std::views::iota(0, int(vs.size()))) {
        for(const int j : std::views::iota(i + 1, int(vs.size()))) {
            const double count = count_matrix.Get(i, j);
            // std::cerr << "i: " << i << ", j: " << j << ", count: " << count
            //           << std::endl;
            if(0.0 < count && count < sample_count) {
                const double p = count / sample_count;
                double information = -p * std::log(p) - (1.0 - p) * std::log(1.0 - p);
                score += information * (radiuses[vs[i]] + radiuses[vs[j]]);
            }
        }
    }
    // std::cerr << "score: " << score << std::endl;
    return score;
}

std::vector<int> SelectQueryFromCandidates(
    const std::array<Double2D, kVertexCount> &points,
    const std::vector<Double2DRange> &ranges,
    const std::array<int, kVertexCount> &v_to_reserved,
    const std::vector<int> &candidates, const int query_size) {
    constexpr double kBudget = 200000.0;
    const int sample_count =
        std::floor(kBudget / (std::max(query_size * query_size, 16) * query_size *
                              (candidates.size() - query_size)));
    // std::cerr << "sample_count: " << sample_count << std::endl;
    std::vector<DoubleMatrix> distance_matrices(
        sample_count, DoubleMatrix(candidates.size(), candidates.size()));
    for(const int sample_id : std::views::iota(0, sample_count)) {
        std::vector<Double2D> sample_points(candidates.size());
        for(const int i : std::views::iota(0, int(candidates.size()))) {
            const int v = candidates[i];
            const double r = (ranges[v].x_max - ranges[v].x_min) * 0.25 +
                             (ranges[v].y_max - ranges[v].y_min) * 0.25;
            const double sd = r * 0.5;
            const double u = g_random.RandomDouble(0.1, 1.0);
            const double magnitude = std::sqrt(-2.0 * std::log(u)) * sd;
            const double theta = g_random.RandomDouble(0.0, 2.0 * M_PI);
            sample_points[i].x = points[v].x + magnitude * std::cos(theta);
            sample_points[i].y = points[v].y + magnitude * std::sin(theta);
        }
        for(const int i : std::views::iota(0, int(candidates.size()))) {
            for(const int j : std::views::iota(0, i)) {
                const double d = Distance(sample_points[i], sample_points[j]);
                distance_matrices[sample_id].Set(i, j, d);
                distance_matrices[sample_id].Set(j, i, d);
            }
        }
    }
    std::vector<double> radiuses(candidates.size());
    for(const int i : std::views::iota(0, int(candidates.size()))) {
        const int v = candidates[i];
        radiuses[i] = (ranges[v].x_max - ranges[v].x_min) * 0.25 +
                      (ranges[v].y_max - ranges[v].y_min) * 0.25;
    }

    std::vector<int> current_indices;
    for(const int i : std::views::iota(0, query_size)) {
        current_indices.push_back(i);
    }
    double current_score = ScoreQuery(candidates.size(), current_indices,
                                      sample_count, distance_matrices, radiuses);

    if(query_size == 3) {
        for(const int i1 : std::views::iota(1, int(candidates.size()))) {
            for(const int i2 : std::views::iota(i1 + 1, int(candidates.size()))) {
                std::vector<int> indices = {0, i1, i2};
                const double score = ScoreQuery(candidates.size(), indices,
                                                sample_count, distance_matrices,
                                                radiuses);
                if(score > current_score) {
                    current_indices = indices;
                    current_score = score;
                }
            }
        }
    } else {
        for(const int target_index :
            std::views::iota(query_size, int(candidates.size()))) {
            int max_index = -1;
            double max_score = current_score;
            for(const int i : std::views::iota(1, query_size)) {
                std::vector<int> indices = current_indices;
                indices[i] = target_index;
                const double score =
                    ScoreQuery(candidates.size(), indices, sample_count,
                               distance_matrices, radiuses);
                if(score > max_score) {
                    max_score = score;
                    max_index = i;
                    // break;
                }
            }
            if(max_index >= 0) {
                current_indices[max_index] = target_index;
                current_score = max_score;
            }
        }
    }

    std::vector<int> max_vs(query_size);
    for(const int i : std::views::iota(0, query_size)) {
        max_vs[i] = candidates[current_indices[i]];
    }
    // std::cerr << "indices: " << current_indices << std::endl;
    // std::cerr << "query_score: " << current_score << std::endl;
    return max_vs;
}

// Select a query based on the root vertex (v0).
std::vector<int> SelectQuery(const std::array<Double2D, kVertexCount> &points,
                             const std::vector<Double2DRange> &ranges,
                             const std::array<int, kVertexCount> &v_to_reserved,
                             const int v0, const int query_size) {
    const double v_r = (ranges[v0].x_max - ranges[v0].x_min) * 0.25 +
                       (ranges[v0].y_max - ranges[v0].y_min) * 0.25;

    std::vector<std::pair<double, int>> scored_vs;
    scored_vs.reserve(kVertexCount);
    for(const int u : std::views::iota(0, kVertexCount)) {
        if(v_to_reserved[u]) {
            continue;
        }
        const double distance = Distance2(points[v0], points[u]);
        const double u_r = (ranges[u].x_max - ranges[u].x_min) * 0.25 +
                           (ranges[u].y_max - ranges[u].y_min) * 0.25;
        const double adjuster = v_r * v_r + u_r * u_r + 10000.0;
        const double score = distance / adjuster;
        scored_vs.emplace_back(score, u);
    }
    std::sort(scored_vs.begin(), scored_vs.end());
    std::vector<int> result;
    result.reserve(query_size);

    if(g_time.CurrentMs() > kQueryPhaseQuickModeTime) {
        for(const int i : std::views::iota(0, query_size)) {
            result.push_back(scored_vs[i].second);
        }
        return result;
    }

    std::vector<int> candidates;
    const int candidate_count = std::max(20, query_size * 2);
    for(const int i : std::views::iota(0, candidate_count)) {
        candidates.push_back(scored_vs[i].second);
    }

    // static int count = 0;
    // static int64 t_sum = 0;
    // int64 t0 = g_time.CurrentUs();
    result = SelectQueryFromCandidates(points, ranges, v_to_reserved, candidates,
                                       query_size);
    // int64 t1 = g_time.CurrentUs();
    // t_sum += t1 - t0;
    // count++;
    // std::cerr << "count: " << count << ", t_sum: " << t_sum << std::endl;

    return result;
}

#endif // QUERY_SELECTOR_CPP_

constexpr double kQueryPhaseTimeLimit = 1500.0;

// For local testing.
std::vector<Edge> MakeMSTFromActualPoints(const std::vector<int> &vs) {
    MergeFindSet mfs(kVertexCount);
    std::vector<WEdge> edges;
    edges.reserve(int(vs.size()) * (int(vs.size()) - 1) / 2);
    for(const int i : std::views::iota(0, int(vs.size()))) {
        for(const int j : std::views::iota(i + 1, int(vs.size()))) {
            edges.emplace_back(
                vs[i], vs[j],
                Distance2(g_actual_points[vs[i]], g_actual_points[vs[j]]));
        }
    }
    std::sort(edges.begin(), edges.end(),
              [](const WEdge &e0, const WEdge &e1) { return e0.w < e1.w; });

    std::vector<Edge> result;
    for(const auto &edge : edges) {
        if(mfs.Find(edge.s) != mfs.Find(edge.t)) {
            mfs.Merge(edge.s, edge.t);
            result.emplace_back(edge.s, edge.t);
            if(result.size() == int(vs.size()) - 1) {
                break;
            }
        }
    }
    return result;
}

std::vector<Edge> QueryMST(const std::vector<int> &vs) {
    if(g_is_local_mode) {
        return MakeMSTFromActualPoints(vs);
    }

    std::cout << "?";
    std::cout << " " << vs.size();
    for(const int v : vs) {
        std::cout << " " << v;
    }
    std::cout << std::endl;
    std::cout.flush();

    std::vector<Edge> result;
    for(const int i : std::views::iota(0, int(vs.size()) - 1)) {
        int s, t;
        std::cin >> s >> t;
        result.emplace_back(s, t);
    }
    return result;
}

void AddDistanceConstraints(const std::vector<int> &vs,
                            const std::vector<Edge> &mst_edges,
                            const std::vector<MCMCSample> &reference_samples,
                            ConstraintSet &distance_contraint_index) {
    static std::array<std::vector<Edge>, kVertexCount> v_to_edges;
    for(const int v : vs) {
        v_to_edges[v].clear();
    }

    for(const auto &edge : mst_edges) {
        v_to_edges[edge.s].push_back(edge);
        v_to_edges[edge.t].push_back(Edge(edge.t, edge.s));
    }

    static std::vector<Edge> current_path;
    auto dfs = [&](auto &&dfs, const int v, const int p) -> void {
        for(const auto &edge : v_to_edges[v]) {
            if(edge.t == p)
                continue;
            current_path.push_back(edge);
            if(current_path.size() >= 2) {
                const int i0 = current_path.front().s;
                const int i1 = current_path.back().t;
                if(i0 < i1) {
                    for(const auto &edge : current_path) {
                        auto [j0, j1] = edge;
                        if(j0 > j1)
                            std::swap(j0, j1);

                        bool required = false;
                        for(const auto &sample : reference_samples) {
                            const double d2i =
                                Distance2(sample.points[i0], sample.points[i1]);
                            const double d2j =
                                Distance2(sample.points[j0], sample.points[j1]);
                            // d(i0, i1) / sqrt(2) < d(j0, j1)
                            if(d2i * 0.5 < d2j) {
                                required = true;
                                break;
                            }
                        }

                        if(!required) {
                            continue;
                        }

                        const DistanceConstraint distance_constraint(i0, i1, j0, j1);
                        distance_contraint_index.AddDistanceConstraint(distance_constraint);
                    }
                }
            }
            dfs(dfs, edge.t, v);
            current_path.pop_back();
        }
    };
    for(const int v : vs) {
        dfs(dfs, v, -1);
    }
}

std::vector<int> FindNeighbors(const std::vector<int> &query,
                               std::array<Double2D, kVertexCount> &points) {
    // First, calculate the average point from the query vertices
    Double2D average_point = {0.0, 0.0};
    for(const int v : query) {
        average_point.x += points[v].x;
        average_point.y += points[v].y;
    }
    average_point.x /= query.size();
    average_point.y /= query.size();

    // Create a vector of pairs (distance to average, vertex_id)
    std::vector<std::pair<double, int>> distance_vertex_pairs;
    for(int v = 0; v < kVertexCount; v++) {
        // Skip vertices that are already in the query
        if(std::find(query.begin(), query.end(), v) != query.end()) {
            continue;
        }

        // Calculate distance from this vertex to the average point
        double dist = Distance(points[v], average_point);
        distance_vertex_pairs.push_back({dist, v});
    }

    // Sort by distance (ascending)
    std::sort(distance_vertex_pairs.begin(), distance_vertex_pairs.end());

    // Take the 50 closest vertices (or fewer if there aren't 50 available)
    std::vector<int> neighbors = query;
    constexpr int kNeighborSize = 50;
    for(const auto &[distance, v] : distance_vertex_pairs) {
        neighbors.push_back(v);
        if(neighbors.size() >= kNeighborSize) {
            break;
        }
    }

    return neighbors;
}

double GetRadiusAverage(const std::vector<Double2DRange> &ranges) {
    double r_sum = 0.0;
    for(const auto &range : ranges) {
        r_sum +=
            (range.x_max - range.x_min) * 0.25 + (range.y_max - range.y_min) * 0.25;
    }
    return r_sum / kVertexCount;
}

void RunSingleQueryStep(const ProblemInput &input, const int query_index,
                        const int v,
                        const std::array<int, kVertexCount> &v_to_reserved,
                        std::array<Double2D, kVertexCount> &points,
                        const std::vector<MCMCSample> &reference_samples,
                        ConstraintSet &distance_contraint_index,
                        std::vector<Double2DRange> &ranges) {
    const std::vector<int> query =
        SelectQuery(points, ranges, v_to_reserved, v, input.max_query_size);

    std::vector<Edge> mst_edges = QueryMST(query);

    AddDistanceConstraints(query, mst_edges, reference_samples,
                           distance_contraint_index);

    const double radius_average = GetRadiusAverage(ranges);
    // std::cerr << "radius_average: " << radius_average << std::endl;

    IndexSet updated_vertex_set(kVertexCount);
    IndexSet added_constraint_index_set(
        distance_contraint_index.distance_constraints.size());
    IndexSet merged_updated_vertex_set(kVertexCount);
    for(const int v : query) {
        updated_vertex_set.Add(v);
        merged_updated_vertex_set.Add(v);
    }

    double alpha = radius_average * 3.0;
    int iteration_count = 30;
    if(g_time.CurrentMs() > kQueryPhaseQuickModeTime) {
        alpha = radius_average * 1.5;
        iteration_count = 15;
    }
    for(const int i : std::views::iota(0, iteration_count)) {
        if(RunGradientDescent(distance_contraint_index, alpha, points,
                              updated_vertex_set, added_constraint_index_set)) {
            alpha *= 0.9;
            for(const int v : updated_vertex_set.Values()) {
                merged_updated_vertex_set.Add(v);
            }
        } else {
            break;
        }
    }

    std::vector<int> vs;
    if(g_time.CurrentMs() < kQueryPhaseQuickModeTime || query_index % 10 == 0) {
        for(const int v : std::views::iota(0, kVertexCount)) {
            vs.push_back(v);
        }
    } else {
        for(const int v : merged_updated_vertex_set.Values()) {
            vs.push_back(v);
        }
    }

    RunGibbsSampling(vs, distance_contraint_index, points, ranges, true);
}

void RunQueryPhase(const ProblemInput &input,
                   const std::vector<int> &reserved_isolated_vertices,
                   std::array<Double2D, kVertexCount> &points,
                   ConstraintSet &distance_contraint_index) {
    std::array<int, kVertexCount> v_to_reserved{};
    for(const int v : reserved_isolated_vertices) {
        v_to_reserved[v] = 1;
    }

    std::vector<std::pair<double, int>> scored_vertices;
    scored_vertices.reserve(kVertexCount);
    for(const int i : std::views::iota(0, kVertexCount)) {
        if(v_to_reserved[i]) {
            continue;
        }
        const double score = (input.x_maxs[i] - input.x_mins[i]) +
                             (input.y_maxs[i] - input.y_mins[i]);
        scored_vertices.emplace_back(score, i);
    }
    std::sort(scored_vertices.begin(), scored_vertices.end(), std::greater<>());

    std::vector<Double2DRange> ranges(kVertexCount);
    for(const int v : std::views::iota(0, kVertexCount)) {
        ranges[v] = Double2DRange(input.x_mins[v], input.x_maxs[v], input.y_mins[v],
                                  input.y_maxs[v]);
    }

    std::vector<MCMCSample> samples;

    for(const int query_index : std::views::iota(0, kMaxQueryCount)) {
        if(query_index == 0 || query_index == 100) {
            samples.clear();
            samples.push_back(MCMCSample(points));

            std::vector<int> vs;
            for(const int v : std::views::iota(0, kVertexCount)) {
                vs.push_back(v);
            }
            while(samples.size() < 100) {
                MCMCSample sample = samples.back();
                RunGibbsSampling(vs, distance_contraint_index, sample.points, ranges);
                samples.push_back(sample);
            }
        }

        if(g_time.CurrentMs() > kQueryPhaseTimeLimit) {
            std::cerr << "Reached time limit in query phase (" << query_index
                      << " queries)." << std::endl;
            break;
        }
        const int v = scored_vertices[query_index].second;
        RunSingleQueryStep(input, query_index, v, v_to_reserved, points, samples,
                           distance_contraint_index, ranges);
    }
}

#endif // QUERY_PHASE_CPP_

void InitializePoints(const ProblemInput &problem_input,
                      std::array<Double2D, kVertexCount> &points) {
    for(const int i : std::views::iota(0, kVertexCount)) {
        points[i].x = (problem_input.x_mins[i] + problem_input.x_maxs[i]) * 0.5;
        points[i].y = (problem_input.y_mins[i] + problem_input.y_maxs[i]) * 0.5;
    }
}

std::vector<RangeConstraint> MakeRangeConstraints(const ProblemInput &input) {
    std::vector<RangeConstraint> range_constraints;
    range_constraints.reserve(kVertexCount);
    for(const int i : std::views::iota(0, kVertexCount)) {
        range_constraints.emplace_back(i, input.x_mins[i], input.x_maxs[i],
                                       input.y_mins[i], input.y_maxs[i]);
    }
    return range_constraints;
}

std::vector<int> ReserveIsolatedVertices(const ProblemInput &input) {
    int isolated_cluster_count = 0;
    // return {};
    for(const int cluster_id : std::views::iota(0, input.cluster_count)) {
        if(input.cluster_sizes[cluster_id] == 1) {
            isolated_cluster_count++;
        }
    }
    std::cerr << "# isolated clusters: " << isolated_cluster_count << std::endl;

    int reserve_count = isolated_cluster_count / 2;
    if(reserve_count == 0) {
        return {};
    }

    if(input.max_query_size > 5 || input.max_error_width < 1000) {
        std::cerr << "isolated vertices reservation disabled" << std::endl;
        return {};
    }
    std::cerr << "isolated vertices reservation enabled" << std::endl;

    std::vector<int> reserved_certices;
    std::vector<std::pair<double, int>> scored_vertices;
    scored_vertices.reserve(kVertexCount);
    for(const int i : std::views::iota(0, kVertexCount)) {
        const double score = (input.x_maxs[i] - input.x_mins[i]) +
                             (input.y_maxs[i] - input.y_mins[i]);
        scored_vertices.emplace_back(score, i);
    }
    std::sort(scored_vertices.begin(), scored_vertices.end(), std::greater<>());

    std::vector<int> reserved_vertices;
    reserved_vertices.reserve(reserve_count);
    for(int i = 0; i < reserve_count; i++) {
        // std::cerr << "reserved_vertices: " << scored_vertices[i].second << " "
        //           << scored_vertices[i].first << std::endl;
        reserved_vertices.push_back(scored_vertices[i].second);
    }

    return reserved_vertices;
}

std::array<Double2D, kVertexCount>
MakeAveragePoints(const std::vector<MCMCSample> &samples) {
    std::array<Double2D, kVertexCount> points{};
    for(const auto &sample : samples) {
        for(const int i : std::views::iota(0, kVertexCount)) {
            points[i].x += sample.points[i].x / samples.size();
            points[i].y += sample.points[i].y / samples.size();
        }
    }
    return points;
}

std::vector<MSTCluster> Solve(const ProblemInput &problem_input) {
    std::array<Double2D, kVertexCount> points;
    InitializePoints(problem_input, points);
    const std::vector<RangeConstraint> range_constraints =
        MakeRangeConstraints(problem_input);
    ConstraintSet distance_contraint_index;
    for(const auto &range_constraint : range_constraints) {
        distance_contraint_index.AddRangeConstraint(range_constraint);
    }
    std::vector<int> reserved_isolated_vertices =
        ReserveIsolatedVertices(problem_input);

    // Phase 1: Query phase.
    RunQueryPhase(problem_input, reserved_isolated_vertices, points,
                  distance_contraint_index);

    // Output information for the query phase.
    std::cerr << "Query phase completed." << std::endl;

    std::cerr << "time: " << g_time.CurrentMs() << "ms" << std::endl;
    std::cerr << "# distance constraints: "
              << distance_contraint_index.distance_constraints.size()
              << std::endl;
    CheckConstraints(distance_contraint_index, points);
    std::cerr << std::endl;

    // Phase 2: Sampling phase.
    std::vector<MCMCSample> samples;
    DoubleMatrix distance_matrix(kVertexCount, kVertexCount);
    RunMCMCPhase(points, distance_contraint_index, samples, distance_matrix);

    std::cerr << "Sampling phase completed." << std::endl;
    std::cerr << "# time: " << g_time.CurrentMs() << "ms" << std::endl;
    CheckConstraints(distance_contraint_index, samples.back().points);
    std::cerr << std::endl;

    // Phase 3: Initial clustering phase.
    points = MakeAveragePoints(samples);

    std::vector<int> assignment(kVertexCount, -1);
    RunClusteringPhase(problem_input, reserved_isolated_vertices, points,
                       distance_matrix, assignment);
    std::cerr << "Initial clustering phase completed." << std::endl;
    std::cerr << "# time: " << g_time.CurrentMs() << "ms" << std::endl;
    std::cerr << std::endl;

    // Phase 4: Clustering iteration phase.
    std::vector<std::vector<Edge>> cluster_to_mst;
    RunClusteringIterationPhase(problem_input, points, distance_matrix,
                                assignment, cluster_to_mst);
    std::cerr << "Clustering iteration phase completed." << std::endl;
    std::cerr << "# time: " << g_time.CurrentMs() << "ms" << std::endl;
    std::cerr << std::endl;

    // Phase 5: Finalize phase.
    const std::vector<MSTCluster> clusters =
        FinalizeSolution(problem_input, assignment, cluster_to_mst);
    std::cerr << "Finalize phase completed." << std::endl;
    std::cerr << "# time: " << g_time.CurrentMs() << "ms" << std::endl;
    std::cerr << std::endl;
    return clusters;
}

#endif // SOLVER_CPP_

ProblemInput ReadProblemInput() {
    ProblemInput problem_input;
    int unused_vertex_count;
    std::cin >> unused_vertex_count;
    std::cin >> problem_input.cluster_count;
    int unused_max_query_count;
    std::cin >> unused_max_query_count;
    std::cin >> problem_input.max_query_size;
    std::cin >> problem_input.max_error_width;

    problem_input.cluster_sizes.resize(problem_input.cluster_count);
    for(const int i : std::views::iota(0, problem_input.cluster_count)) {
        std::cin >> problem_input.cluster_sizes[i];
    }

    for(const int i : std::views::iota(0, kVertexCount)) {
        std::cin >> problem_input.x_mins[i];
        std::cin >> problem_input.x_maxs[i];
        std::cin >> problem_input.y_mins[i];
        std::cin >> problem_input.y_maxs[i];
    }
    return problem_input;
}

void ReadActualPoints() {
    for(const int i : std::views::iota(0, kVertexCount)) {
        std::cin >> g_actual_points[i].x;
        std::cin >> g_actual_points[i].y;
    }
}

void WriteSolution(const std::vector<MSTCluster> &clusters) {
    std::cout << "!" << std::endl;
    for(const auto &cluster : clusters) {
        for(const int i : std::views::iota(0, int(cluster.vs.size()))) {
            if(i > 0) {
                std::cout << " ";
            }
            std::cout << cluster.vs[i];
        }
        std::cout << std::endl;
        for(const auto &edge : cluster.mst_edges) {
            std::cout << edge.s << " " << edge.t << std::endl;
        }
    }
}

void Run() {
    const ProblemInput problem_input = ReadProblemInput();
    if(g_is_local_mode) {
        ReadActualPoints();
    }
    const std::vector<MSTCluster> solution = Solve(problem_input);
    WriteSolution(solution);
}

int main(int argc, char *argv[]) {
    for(int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if(arg == "l") {
            g_is_local_mode = true;
        }
        if(arg.starts_with("tf=")) {
            g_time_factor = std::stof(arg.substr(3));
        }
    }
    Run();
}
