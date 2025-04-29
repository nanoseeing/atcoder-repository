from collections import deque


def bfs(N, v0, edge):
    """最短経路（BFS）O(V+E)"""

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


def judge_cicle(N, v0, edge):
    """閉路検出"""

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
