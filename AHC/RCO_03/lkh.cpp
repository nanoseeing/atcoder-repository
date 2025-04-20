#include <bits/stdc++.h>
using namespace std;
using ld = double;
constexpr int N = 200;
constexpr ld INF = 1e18;
constexpr ld TIME_LIMIT = 1.95;

int perm[N + 1], best[N + 1];
ld dist[N][N], avg_dist;
ld best_score = INF;
mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());

inline ld variance(ld sum, ld sq) {
    return sq / N - (sum / N) * (sum / N);
}

ld compute_score(const int *path) {
    ld sum = 0.0, sq = 0.0;
    for(int i = 0; i < N; ++i) {
        ld d = dist[path[i]][path[i + 1]];
        sum += d;
        sq += d * d;
    }
    return variance(sum, sq);
}

void init_greedy(const vector<pair<ld, ld>> &points) {
    iota(perm, perm + N, 0);
    vector<bool> used(N, false);
    used[0] = true;
    int cur = 0;
    for(int i = 1; i < N; ++i) {
        int nxt = -1;
        ld best = INF;
        for(int j = 0; j < N; ++j) {
            if(used[j])
                continue;
            ld d = fabs(dist[cur][j] - avg_dist);
            if(d < best) {
                best = d;
                nxt = j;
            }
        }
        perm[i] = nxt;
        used[nxt] = true;
        cur = nxt;
    }
    perm[N] = perm[0];
}

bool gain_based_kopt(int depth, int max_depth, vector<pair<int, int>> &removed, vector<pair<int, int>> &added, vector<bool> &visited, ld T) {
    if(depth >= max_depth)
        return false;
    int t1 = removed.back().second;
    for(int t3 = 0; t3 < N; ++t3) {
        if(visited[t3] || t3 == t1)
            continue;
        ld g = dist[t1][t3];
        if(g >= dist[removed.back().first][t1])
            continue;

        added.emplace_back(t1, t3);
        visited[t3] = true;

        for(int i = 0; i < N; ++i) {
            if(perm[i] == t3 && i + 1 < N) {
                int t4 = perm[i + 1];
                removed.emplace_back(t3, t4);

                if(t4 == removed[0].first) {
                    ld sum = 0.0, sq = 0.0;
                    vector<int> new_perm(perm, perm + N + 1);
                    int r1 = i + 1;
                    int r2 = N - 1;
                    if(r1 < r2)
                        reverse(new_perm.begin() + r1, new_perm.begin() + r2 + 1);
                    new_perm[N] = new_perm[0];
                    for(int j = 0; j < N; ++j) {
                        ld d = dist[new_perm[j]][new_perm[j + 1]];
                        sum += d;
                        sq += d * d;
                    }
                    ld new_score = variance(sum, sq);
                    if(new_score < best_score || exp((best_score - new_score) / T) > uniform_real_distribution<ld>(0.0, 1.0)(rng)) {
                        memcpy(perm, new_perm.data(), sizeof(perm));
                        perm[N] = perm[0];
                        return true;
                    }
                } else {
                    if(gain_based_kopt(depth + 1, max_depth, removed, added, visited, T))
                        return true;
                }

                removed.pop_back();
            }
        }

        added.pop_back();
        visited[t3] = false;
    }
    return false;
}

void try_gain_kopt(ld T) {
    for(int s = 0; s < N - 1; ++s) {
        int t1 = perm[s];
        int t2 = perm[s + 1];
        vector<pair<int, int>> removed = {{t1, t2}};
        vector<pair<int, int>> added;
        vector<bool> visited(N, false);
        visited[t1] = visited[t2] = true;
        gain_based_kopt(1, 4, removed, added, visited, T);
    }
    perm[N] = perm[0];
}

void try_2opt(ld T) {
    for(int iter = 0; iter < 2000; ++iter) {
        int i = uniform_int_distribution<int>(0, N - 2)(rng);
        int j = uniform_int_distribution<int>(i + 2, N - (i == 0 ? 2 : 1))(rng);
        if(j == N)
            continue;

        int a = perm[i], b = perm[i + 1];
        int c = perm[j], d = perm[j + 1];

        ld before = dist[a][b] + dist[c][d];
        ld after = dist[a][c] + dist[b][d];

        ld delta = after * after + dist[b][d] * dist[b][d] - (dist[a][b] * dist[a][b] + dist[c][d] * dist[c][d]);

        if(after < before || exp(-(after - before) / T) > uniform_real_distribution<ld>(0.0, 1.0)(rng)) {
            reverse(perm + i + 1, perm + j + 1);
        }
    }
    perm[N] = perm[0];
}

void try_3opt(ld T) {
    for(int iter = 0; iter < 1000; ++iter) {
        int a = uniform_int_distribution<int>(0, N - 4)(rng);
        int b = uniform_int_distribution<int>(a + 2, N - 2)(rng);
        int c = uniform_int_distribution<int>(b + 2, N - 1)(rng);

        int A = perm[a], B = perm[a + 1];
        int C = perm[b], D = perm[b + 1];
        int E = perm[c], F = perm[(c + 1) % (N + 1)];

        ld d0 = dist[A][B] + dist[C][D] + dist[E][F];

        reverse(perm + a + 1, perm + b + 1);
        reverse(perm + b + 1, perm + c + 1);

        ld d1 = dist[perm[a]][perm[a + 1]] + dist[perm[b]][perm[b + 1]] + dist[perm[c]][perm[(c + 1) % (N + 1)]];

        ld delta = d1 * d1 - d0 * d0;

        if(d1 > d0 && exp(-(d1 - d0) / T) <= uniform_real_distribution<ld>(0.0, 1.0)(rng)) {
            reverse(perm + b + 1, perm + c + 1);
            reverse(perm + a + 1, perm + b + 1);
        }
    }
    perm[N] = perm[0];
}

void double_bridge() {
    int a = uniform_int_distribution<int>(1, N / 4)(rng);
    int b = uniform_int_distribution<int>(N / 4, N / 2)(rng);
    int c = uniform_int_distribution<int>(N / 2, 3 * N / 4)(rng);
    int d = uniform_int_distribution<int>(3 * N / 4, N - 1)(rng);

    rotate(perm + a, perm + b, perm + c);
    rotate(perm + b, perm + c, perm + d);
    perm[N] = perm[0];
}

int main() {
    int n;
    cin >> n;
    assert(n == N);
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<pair<ld, ld>> points(N);
    for(int i = 0; i < N; ++i) {
        cin >> points[i].first >> points[i].second;
    }

    for(int i = 0; i < N; ++i)
        for(int j = 0; j < N; ++j)
            dist[i][j] = hypot(points[i].first - points[j].first, points[i].second - points[j].second);

    ld total = 0.0;
    for(int i = 0; i < N; ++i)
        for(int j = i + 1; j < N; ++j)
            total += dist[i][j];
    avg_dist = total * 2 / (N * (N - 1));

    init_greedy(points);
    best_score = compute_score(perm);
    memcpy(best, perm, sizeof(perm));

    auto start = chrono::steady_clock::now();
    ld T = 0.5, T_min = 1e-4, alpha = 0.9995;

    while(chrono::duration<double>(chrono::steady_clock::now() - start).count() < TIME_LIMIT) {
        try_2opt(T);
        try_3opt(T);
        try_gain_kopt(T);
        ld score = compute_score(perm);
        if(score < best_score) {
            best_score = score;
            memcpy(best, perm, sizeof(perm));
        } else {
            memcpy(perm, best, sizeof(perm));
            double_bridge();
        }
        T = max(T * alpha, T_min);
    }

    for(int i = 0; i < N; ++i)
        cout << best[i] << '\n';

    return 0;
}
