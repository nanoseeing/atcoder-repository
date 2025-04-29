import heapq


def dijkstra_with_path(graph, start):
    # graph: {node: [(neighbor, cost), ...]}
    dist = {}
    prev = {}
    heap = [(0, start)]

    while heap:
        cost, u = heapq.heappop(heap)
        if u in dist:
            continue
        dist[u] = cost
        for v, weight in graph.get(u, []):
            if v not in dist:
                heapq.heappush(heap, (cost + weight, v))
                if v not in prev or cost + weight < dist.get(v, float("inf")):
                    prev[v] = u

    return dist, prev


def restore_path(prev, start, goal):
    if goal not in prev and start != goal:
        return None  # 到達できない
    path = [goal]
    while path[-1] != start:
        path.append(prev[path[-1]])
    path.reverse()
    return path


# --- 使用例 ---
graph = {"A": [("B", 1), ("C", 4)], "B": [("C", 2), ("D", 5)], "C": [("D", 1)], "D": []}

start = "A"
goal = "D"

dist, prev = dijkstra_with_path(graph, start)
path = restore_path(prev, start, goal)

print(f"最短距離: {dist[goal]}")
print(f"最短経路: {path}")
