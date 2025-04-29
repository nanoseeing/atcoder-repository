def warshall_floyd(N, cost: list[list[int]]) -> list[list[int]]:
    """全点対間最短経路(ワーシャルフロイド) O(V^3)"""

    INF = 10**18
    cost = [[INF for _ in range(N)] for _ in range(N)]

    # 対角成分を0に
    for i in range(N):
        cost[i][i] = 0

    for k in range(N):
        for i in range(N):
            for j in range(N):
                if cost[i][k] < INF and cost[k][j] < INF:
                    cost[i][j] = min(cost[i][j], cost[i][k] + cost[k][j])

    return cost
