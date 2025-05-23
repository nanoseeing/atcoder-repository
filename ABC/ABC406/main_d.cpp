/*
# ================
# Main
# ================

# import itertools
# import heapq
# import bisect
# from collections import Counter
# from collections import defaultdict
# from collections import deque

# sys.setrecursionlimit(10**7)

MOD = 10**9 + 7
INF = 9 * 10**18

def main():
    H, W, N = in_nn()
    X = []
    Y = []
    for i in range(N):
        x, y = in_nn()
        X.append(x)
        Y.append(y)

    Xsort = sorted(set(X))
    Ysort = sorted(set(Y))

    XN = len(Xsort)
    YN = len(Ysort)

    x_id2num = {}
    x_num2id = {}
    y_id2num = {}
    y_num2id = {}
    for i in range(XN):
        x_id2num[i] = Xsort[i]
        x_num2id[Xsort[i]] = i
    for i in range(YN):
        y_id2num[i] = Ysort[i]
        y_num2id[Ysort[i]] = i

    new_X = []
    new_Y = []
    for i in range(N):
        new_X.append(x_num2id[X[i]])
        new_Y.append(y_num2id[Y[i]])

    Xhave = [set() for _ in range(XN)]
    Yhave = [set() for _ in range(YN)]
    for i in range(N):
        x = new_X[i]
        y = new_Y[i]
        Xhave[x].add(y)
        Yhave[y].add(x)

    is_x_query = [False] * XN
    is_y_query = [False] * YN

    ans = []
    Q = in_n()
    for q in range(Q):
        qtype, num = in_nn()
        if qtype == 1:
            x = num
            if x not in x_num2id:
                ans.append(0)
            else:
                new_X = x_num2id[x]
                if is_x_query[new_X]:
                    ans.append(0)
                else:
                    ans.append(len(Xhave[new_X]))
                    for y in Xhave[new_X]:
                        Yhave[y].discard(new_X)
                    is_x_query[new_X] = True
        else:
            y = num
            if y not in y_num2id:
                ans.append(0)
            else:
                new_Y = y_num2id[y]
                if is_y_query[new_Y]:
                    ans.append(0)
                else:
                    ans.append(len(Yhave[new_Y]))
                    for x in Yhave[new_Y]:
                        Xhave[x].discard(new_Y)
                    is_y_query[new_Y] = True

    print(*ans, sep="\n")
*/

#include <bits/stdc++.h>
using namespace std;

using ll = long long;
const ll INF = 9e18;
const int MOD = 1e9 + 7;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int H, W, N;
    cin >> H >> W >> N;
    vector<int> X(N), Y(N);
    for(int i = 0; i < N; ++i) {
        cin >> X[i] >> Y[i];
    }

    // Coordinate compression
    vector<int> Xsort = X, Ysort = Y;
    sort(Xsort.begin(), Xsort.end());
    Xsort.erase(unique(Xsort.begin(), Xsort.end()), Xsort.end());

    sort(Ysort.begin(), Ysort.end());
    Ysort.erase(unique(Ysort.begin(), Ysort.end()), Ysort.end());

    int XN = Xsort.size();
    int YN = Ysort.size();

    unordered_map<int, int> x_num2id, y_num2id;
    for(int i = 0; i < XN; ++i)
        x_num2id[Xsort[i]] = i;
    for(int i = 0; i < YN; ++i)
        y_num2id[Ysort[i]] = i;

    vector<int> new_X(N), new_Y(N);
    for(int i = 0; i < N; ++i) {
        new_X[i] = x_num2id[X[i]];
        new_Y[i] = y_num2id[Y[i]];
    }

    vector<unordered_set<int>> Xhave(XN), Yhave(YN);
    for(int i = 0; i < N; ++i) {
        int x = new_X[i];
        int y = new_Y[i];
        Xhave[x].insert(y);
        Yhave[y].insert(x);
    }

    vector<bool> is_x_query(XN, false), is_y_query(YN, false);
    vector<int> ans;

    int Q;
    cin >> Q;
    for(int q = 0; q < Q; ++q) {
        int qtype, num;
        cin >> qtype >> num;
        if(qtype == 1) {
            if(!x_num2id.count(num)) {
                ans.push_back(0);
            } else {
                int idx = x_num2id[num];
                if(is_x_query[idx]) {
                    ans.push_back(0);
                } else {
                    ans.push_back(Xhave[idx].size());
                    for(int y : Xhave[idx]) {
                        Yhave[y].erase(idx);
                    }
                    is_x_query[idx] = true;
                }
            }
        } else {
            if(!y_num2id.count(num)) {
                ans.push_back(0);
            } else {
                int idx = y_num2id[num];
                if(is_y_query[idx]) {
                    ans.push_back(0);
                } else {
                    ans.push_back(Yhave[idx].size());
                    for(int x : Yhave[idx]) {
                        Xhave[x].erase(idx);
                    }
                    is_y_query[idx] = true;
                }
            }
        }
    }

    for(int a : ans)
        cout << a << '\n';
    return 0;
}
