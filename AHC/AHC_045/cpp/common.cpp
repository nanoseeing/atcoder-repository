#include <bits/stdc++.h>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <algorithm>
#include <numeric>

using namespace std;

// Union-Find（Disjoint Set Union）
class UnionFind {
private:
    unordered_map<int, int> parent;

public:
    UnionFind(const vector<int>& vs) {
        for (int v : vs) {
            parent[v] = -1;
        }
    }

    int find(int x) {
        if (parent[x] < 0) return x;
        return parent[x] = find(parent[x]);
    }

    void unite(int x, int y) {
        x = find(x);
        y = find(y);
        if (x == y) return;

        if (parent[x] > parent[y]) swap(x, y);
        parent[x] += parent[y];
        parent[y] = x;
    }

    bool same(int x, int y) {
        return find(x) == find(y);
    }

    int size(int x) {
        return -parent[find(x)];
    }

    vector<int> members(int x) {
        int root = find(x);
        vector<int> res;
        for (const auto& [k, _] : parent) {
            if (find(k) == root) res.push_back(k);
        }
        return res;
    }

    vector<int> roots() {
        vector<int> res;
        for (const auto& [k, v] : parent) {
            if (v < 0) res.push_back(k);
        }
        return res;
    }

    int group_count() {
        return roots().size();
    }

    unordered_map<int, vector<int>> all_group_members() {
        unordered_map<int, vector<int>> res;
        for (const auto& [k, _] : parent) {
            int r = find(k);
            res[r].push_back(k);
        }
        return res;
    }
};

// 辞書順ソートされたエッジタプルの形式 (a, b)
pair<int, int> sort_pair(pair<int, int> ab) {
    return ab.first < ab.second ? ab : make_pair(ab.second, ab.first);
}

// クラスカル法による最小全域木
pair<vector<pair<int, int>>, double> kruskals_algorithm(vector<tuple<double, int, int>> edges, const vector<int>& vs) {
    UnionFind uf(vs);
    double cost = 0.0;
    vector<pair<int, int>> ans_edges;

    for (const auto& [c, a, b] : edges) {
        if (!uf.same(a, b)) {
            cost += c;
            uf.unite(a, b);
            ans_edges.push_back(sort_pair({a, b}));
        }
    }

    return {ans_edges, cost};
}


int main(void){
    vector<tuple<double, int, int>> edges = {
        {1.2, 0, 1},
        {2.5, 1, 2},
        {0.9, 0, 2},
    };

    vector<int> vs = {0, 1, 2};
    auto [mst_edges, total_cost] = kruskals_algorithm(edges, vs);
    for (const auto& edge : mst_edges) {
        cout << "Edge: " << edge.first << " - " << edge.second << endl;
    }

    return 0;
}