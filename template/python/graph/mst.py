import heapq

from ..data_structure.union_find import UnionFind


def prims_algorithm(N, cost: list[list[tuple[int, int]]]) -> int:
    """最小全域木[プリム法] O(ElogV)"""
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


def kruskals_algorithm(N, edges: list[tuple[int, int, int]]) -> int:
    """最小全域木[クラスカル法] O(ElogV)
    edges : (c, a, b) 事前にソートしておくこと
    """
    uf = UnionFind(N)
    cost = 0
    for edge in edges:
        c, a, b = edge
        if not uf.same(a, b):
            cost += c
            uf.union(a, b)
            cost += c
            uf.union(a, b)
    return cost
