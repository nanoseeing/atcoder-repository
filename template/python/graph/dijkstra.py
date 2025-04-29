import heapq


def _restore_path(prev: list[int], start: int, goal: int) -> list[int] | None:
    """最短経路復元"""
    path = [goal]
    while path[-1] != start:
        if prev[path[-1]] == -1:
            return None  # 到達不可
        path.append(prev[path[-1]])

    path.reverse()
    return path


def dijkstra(graph: list[list[tuple[float, int]]], start: int, goal: int) -> float:
    """dijkstra(O(V+E)logV)"""
    graph_N = len(graph)
    d: list[float] = [float("inf")] * graph_N
    # prev: list[int] = [-1] * graph_N
    edgelist: list[tuple[float, int]] = [(0, start)]
    heapq.heapify(edgelist)

    d[start] = 0
    while len(edgelist):
        now_cost, v = heapq.heappop(edgelist)
        if d[v] < now_cost:
            continue
        for cost, to in graph[v]:
            if d[to] > (cost + d[v]):
                d[to] = cost + d[v]
                # prev[to] = v
                heapq.heappush(edgelist, (cost + d[v], to))

    return d[goal]

    # path = _restore_path(prev, start, goal)
    # return path
