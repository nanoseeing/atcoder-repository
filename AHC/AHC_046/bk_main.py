import heapq
import sys
from collections import deque
from dataclasses import dataclass

STDERR = sys.stderr

INF = 10**9
N = 22
M = 40

COMMANDS = ["M", "S", "A"]
DIRECTIONS_STR = ["L", "R", "U", "D"]


@dataclass
class Coord:
    x: int
    y: int


@dataclass
class ProblemInput:
    start: Coord
    target_coords: list[Coord]


@dataclass
class Action:
    command: str
    direction: str


def dijkstra(N, s0, edge):
    """単一始点最短経路 O(NlogN)"""

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


def bfs(grid_map, start: Coord, goal: Coord):
    print(f"BFS: {start} -> {goal}", file=STDERR)
    sy, sx = start.y, start.x
    gy, gx = goal.y, goal.x

    dist = [[INF] * N for _ in range(N)]
    prev = [[None] * N for _ in range(N)]
    prev_act = [[None] * N for _ in range(N)]
    dist[sy][sx] = 0
    queue: deque[Coord] = deque()
    queue.append(Coord(sx, sy))

    directions = [(-1, 0), (1, 0), (0, -1), (0, 1)]
    while queue:
        coord = queue.popleft()
        y, x = coord.y, coord.x

        # 移動
        for di, (dx, dy) in enumerate(directions):
            ny, nx = y + dy, x + dx
            if grid_map[ny][nx] == "#":
                continue
            if dist[ny][nx] > dist[y][x] + 1:
                dist[ny][nx] = dist[y][x] + 1
                prev[ny][nx] = Coord(y, x)
                prev_act[ny][nx] = Action("M", DIRECTIONS_STR[di])
                queue.append(Coord(nx, ny))

        # 滑走
        for di, (dx, dy) in enumerate(directions):
            ny, nx = y, x
            while True:
                nx += dx
                ny += dy
                if grid_map[ny][nx] == "#":
                    nx -= dx
                    ny -= dy
                    if x == nx and y == ny:
                        break
                    if dist[ny][nx] > dist[y][x] + 1:
                        dist[ny][nx] = dist[y][x] + 1
                        prev[ny][nx] = Coord(y, x)
                        prev_act[ny][nx] = Action("S", DIRECTIONS_STR[di])
                        queue.append(Coord(nx, ny))
                    break

    assert dist[gy][gx] < INF

    # path = []
    actions = []
    y, x = gx, gy
    while (y, x) != (sx, sy):
        # path.append((y, x))
        prev_coord = prev[y][x]
        y, x = prev_coord.y, prev_coord.x
        action = prev_act[y][x]
        if action:
            actions.append(action)
    # path.append((sx, sy))
    # path.reverse()
    actions.reverse()

    assert len(actions) > 0

    return actions


def greedy(problem_input: ProblemInput) -> list[Action]:
    grid_map = [[""] * (N) for _ in range(N)]
    for i in range(N):
        for j in range(N):
            if i == 0 or i == N - 1 or j == 0 or j == N - 1:
                grid_map[i][j] = "#"
            else:
                grid_map[i][j] = "."

    # for i in range(N + 2):
    #     print("".join(grid_map[i]))

    actions = []
    start = problem_input.start

    for coord in problem_input.target_coords:
        bfs_path = bfs(grid_map, start, coord)
        assert len(bfs_path) > 0
        actions.extend(bfs_path)
        start = coord
        break
    return actions


def problem_input():
    input()  # N, M

    start = tuple(map(int, input().split()))
    start = Coord(start[0] + 1, start[1] + 1)

    target_coords = []
    for _ in range(M - 1):
        coord = tuple(map(int, input().split()))
        coord = Coord(coord[0] + 1, coord[1] + 1)
        target_coords.append(coord)
    return ProblemInput(start, target_coords)


def problem_output(action_list: list[Action]):
    act_str = ""
    for action in action_list:
        act_str += f"{action.command} {action.direction}\n"
    print(act_str)


def solve():
    problem_data = problem_input()
    action_list = greedy(problem_data)
    problem_output(action_list)


def main():
    solve()


if __name__ == "__main__":
    main()
