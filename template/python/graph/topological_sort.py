from collections import deque


def topological_sort(N, dag):
    """トポロジカルソート O(V+E)"""

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
    return ret
