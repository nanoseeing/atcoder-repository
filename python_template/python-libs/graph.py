""" グラフ関連 """

from collections import deque
import heapq
INF = 10**18


def bfs(N, v0, edge):
    """ 最短経路（BFS）O(V+E)
    """

    d = [-1] * N
    d[v0] = 0
    q = deque()
    q.append(v0)

    while q:
        v = q.popleft()
        for nv in edge[v]:
            if d[nv] == -1:
                q.append(nv)
                d[nv] = d[v] + 1

    return d


def topological_sort(N, dag):
    """ トポロジカルソート O(V+E)
    """

    degree = [0] * N
    for i in range(N):
        for v in dag[i]:
            degree[v] += 1

    ret = [v for v in range(N) if degree[v] == 0]
    q = deque(ret)

    while q:
        v = q.popleft()
        for nv in dag[v]:
            degree[nv] -= 1
            if degree[nv] == 0:
                q.append(nv)
                ret.append(nv)

    return ret


def judge_tree(N, v0, edge):
    """ 閉路検出
    """

    search = [True] * N
    search[v0] = False
    q = deque()
    q.append((v0, -1))

    while q:
        v, pv = q.popleft()
        for nv in edge[v]:
            if search[nv]:
                q.append((nv, v))
                search[nv] = False
            elif pv != nv:
                return True

    return False


def dijkstra(N, s0, edge):
    """ 単一始点最短経路 O(NlogN)
    """

    d = [INF] * N
    used = [False] * N
    edgelist = [(0, s0)]
    heapq.heapify(edgelist)

    while len(edgelist):
        minedge = heapq.heappop(edgelist)
        if used[minedge[1]]:
            continue
        v = minedge[1]
        d[v] = minedge[0]
        used[v] = True
        for e in edge[v]:
            if d[e[1]] <= (e[0] + d[v]) or used[e[1]]:
                continue
            heapq.heappush(edgelist, [e[0] + d[v], e[1]])
    return d


def warshall_floyd(N, cost):
    """ 全点対間最短経路(ワーシャルフロイド) O(V^3)
    """

    cost = [[INF for _ in range(N)] for _ in range(N)]

    # 対角成分を0に
    for i in range(N):
        cost[i][i] = 0

    for k in range(N):
        for i in range(N):
            for j in range(N):
                if cost[i][k] < INF and cost[k][j] < INF:
                    cost[i][j] = min(cost[i][j], cost[i][k] + cost[k][j])


def prims_algorithm(N, cost):
    """ 最小全域木[プリム法] O(ElogV)
    """
    used = [False] * N
    used[0] = True
    que = [(c, w) for c, w in cost[0]]
    heapq.heapify(que)

    ret = 0
    while que:
        cv, v = heapq.heappop(que)
        if used[v]:
            continue
        used[v] = True
        ret += cv
        for c, w in cost[v]:
            if used[w]:
                continue
            heapq.heappush(que, (c, w))

    return ret


def kruskals_algorithm(edges, N):
    """ 最小全域木[クラスカル法] O(ElogV)
        edges : (c, a, b) 事前にソートしておくこと
    """
    uf = UnionFind(N)
    cost = 0
    for edge in edges:
        c, a, b = edge
        if not uf.same(a, b):
            cost += c
            uf.union(a, b)
