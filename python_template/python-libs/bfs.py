""" 幅優先探索 """

from collections import deque


# グラフ探索（基本形）
def bfs(N, v0, edge):

    search = [True] * N
    search[v0] = False
    q = deque()
    q.append(v0)

    while q:
        v = q.popleft()
        for nv in edge[v]:
            if search[nv]:
                q.append(nv)
                search[nv] = False


# グリッド移動
def bfs_grid(H, W, sx, sy, grid):

    q = deque()
    q.append((sx, sy))
    d = [[-1 for _ in range(W)] for _ in range(H)]
    d[sy][sx] = 0

    px = [-1, 1, 0, 0]
    py = [0, 0, -1, 1]
    # px = [-1, 0, 1, -1, 1, -1, 0, 1]
    # py = [1, 1, 1, 0, 0, -1, -1, -1]

    while q:
        now = q.popleft()
        x, y = now
        for i in range(4):
            nx, ny = x + px[i], y + py[i]
            if 0 <= nx < W and 0 <= ny < H and grid[ny][nx] == '.':
                if d[ny][nx] == -1:
                    d[ny][nx] = d[y][x] + 1
                    q.append((nx, ny))
