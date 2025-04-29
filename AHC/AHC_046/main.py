import heapq
import random
import sys
import time
from dataclasses import dataclass

random.seed(0)
STDERR = sys.stderr

INF = 10**9
N = 20
M = 40

COMMANDS = ["M", "S", "A"]
DIRECTIONS_STR = ["L", "R", "U", "D"]
DIRECTIONS_LIST = [[-1, 0], [1, 0], [0, -1], [0, 1]]

MAX_TIME_SEC = 1.80


@dataclass
class Coord:
    x: int
    y: int

    def __hash__(self):
        return hash((self.x, self.y))


@dataclass
class ProblemInput:
    start: Coord
    target_coords: list[Coord]


@dataclass
class Action:
    command: str
    direction: str


def problem_input():
    input()  # N, M

    start = tuple(map(int, input().split()))
    start = Coord(start[1], start[0])

    target_coords = []
    for _ in range(M - 1):
        coord = tuple(map(int, input().split()))
        coord = Coord(coord[1], coord[0])
        target_coords.append(coord)
    return ProblemInput(start, target_coords)


def problem_output(action_list: list[Action]):
    act_str = ""
    for action in action_list:
        act_str += f"{action.command} {action.direction}\n"
    print(act_str)


def _restore_path(prev, start, goal):
    assert start != goal
    path = [goal]
    while path[-1] != start:
        if prev[path[-1]] == -1:
            return None
        path.append(prev[path[-1]])

    path.reverse()
    return path


def dijkstra(graph, start, goal) -> list[Action] | None:
    graph_N = len(graph)
    d = [INF] * graph_N
    prev = [-1] * graph_N
    edgelist = [(0, start)]
    heapq.heapify(edgelist)

    d[start] = 0
    while len(edgelist):
        now_cost, v = heapq.heappop(edgelist)
        if d[v] < now_cost:
            continue
        for cost, to in graph[v]:
            if d[to] > (cost + d[v]):
                d[to] = cost + d[v]
                prev[to] = v
                heapq.heappush(edgelist, (cost + d[v], to))

    path = _restore_path(prev, start, goal)
    if path is None:
        return None

    path_n = len(path)
    actions = []
    for i in range(path_n - 1):
        prev_v = path[i]
        next_v = path[i + 1]
        x = prev_v % N
        y = prev_v // N
        dx = next_v % N
        dy = next_v // N
        if x == dx:
            if y < dy:
                direction = "D"
            else:
                direction = "U"
        else:
            if x < dx:
                direction = "R"
            else:
                direction = "L"
        diff = abs(x - dx) + abs(y - dy)
        if diff == 1:
            command = "M"
        else:
            command = "S"
        actions.append(Action(command, direction))

    return actions


def construct_graph(n: int) -> list[set[tuple[int, int]]]:
    graph: list[set[tuple[int, int]]] = [set() for _ in range(n * n)]
    for x in range(n):
        for y in range(n):
            v = y * n + x
            for dx, dy in DIRECTIONS_LIST:
                nx = x + dx
                ny = y + dy
                if nx < 0 or nx >= n or ny < 0 or ny >= n:
                    continue
                graph[v].add((1, ny * n + nx))

            if x != 0:
                nx, ny = 0, y
                graph[v].add((1, ny * n + nx))
            if x != n - 1:
                nx, ny = n - 1, y
                graph[v].add((1, ny * n + nx))
            if y != 0:
                nx, ny = x, 0
                graph[v].add((1, ny * n + nx))
            if y != n - 1:
                nx, ny = x, n - 1
                graph[v].add((1, ny * n + nx))
    return graph


def update_edge(graph: list[set[tuple[int, int]]], add_wall_coord: Coord):
    wall_y, wall_x = add_wall_coord.y, add_wall_coord.x
    v = wall_y * N + wall_x

    graph[v] = set()  # 壁からの移動を禁止

    for from_x in range(N):
        from_v = wall_y * N + from_x
        new_set = set()
        for c, to_v in graph[from_v]:
            to_x, to_y = to_v % N, to_v // N
            if from_x == wall_x:
                continue
            if from_x < wall_x <= to_x:
                new_to_x = wall_x - 1
                if from_x != new_to_x:
                    new_set.add((c, to_y * N + new_to_x))
            elif to_x <= wall_x < from_x:
                new_to_x = wall_x + 1
                if from_x != new_to_x:
                    new_set.add((c, to_y * N + new_to_x))
            else:
                new_set.add((c, to_v))
        graph[from_v] = new_set

    for from_y in range(N):
        from_v = from_y * N + wall_x
        new_set = set()
        for c, to_v in graph[from_v]:
            to_x, to_y = to_v % N, to_v // N
            if from_y == wall_y:
                continue
            if from_y < wall_y <= to_y:
                new_to_y = wall_y - 1
                if from_y != new_to_y:
                    new_set.add((c, new_to_y * N + wall_x))
            elif to_y <= wall_y < from_y:
                new_to_y = wall_y + 1
                if from_y != new_to_y:
                    new_set.add((c, new_to_y * N + wall_x))
            else:
                new_set.add((c, to_v))
        graph[from_v] = new_set


def walking(problem_data: ProblemInput, graph: list[set[tuple[int, int]]], walk_inds: list[Coord]):
    actions = []
    start = problem_data.start

    walk_inds_set = set(walk_inds)
    for coord in problem_data.target_coords:
        # print(f"start: {start}, goal: {coord}", file=STDERR)
        sy, sx = start.y, start.x
        for i, (dx, dy) in enumerate(DIRECTIONS_LIST):
            nx, ny = sx + dx, sy + dy
            if Coord(nx, ny) not in walk_inds_set:
                continue
            update_edge(graph, Coord(nx, ny))
            walk_inds_set.remove(Coord(nx, ny))
            actions.append(Action("A", DIRECTIONS_STR[i]))
            # print(f"add wall: {nx}, {ny}", file=STDERR)
        gy, gx = coord.y, coord.x
        sv = sy * N + sx
        gv = gy * N + gx
        bfs_path = dijkstra(graph, sv, gv)
        if bfs_path is None:
            return None
        actions.extend(bfs_path)
        start = coord
    return actions


def greedy(problem_input: ProblemInput) -> list[Action]:
    graph = construct_graph(N)
    ret = walking(problem_input, graph, [])
    assert ret is not None
    return ret


def random_walls(problem_data: ProblemInput):
    target_wall_inds = set()
    for coord in problem_data.target_coords:
        y, x = coord.y, coord.x
        for dx, dy in DIRECTIONS_LIST:
            nx = x + dx
            ny = y + dy
            if nx < 0 or nx >= N or ny < 0 or ny >= N:
                continue
            target_wall_inds.add(Coord(nx, ny))
    target_wall_list = list(target_wall_inds)

    best_action = greedy(problem_data)
    best_cost = len(best_action)
    best_wall_delete = 0

    simulate_cnt = 0

    start_time = time.perf_counter()
    while True:
        if time.perf_counter() - start_time > MAX_TIME_SEC:
            break
        target_coords = random.choices(target_wall_list, k=random.randint(1, 50))
        graph = construct_graph(N)
        actions = walking(problem_data, graph, target_coords)
        if actions is None:
            continue
        if len(actions) < best_cost:
            best_cost = len(actions)
            best_action = actions
            best_wall_delete = len(target_coords)
        simulate_cnt += 1
        print(f"{simulate_cnt}/{100} best_cost: {best_cost} ({best_wall_delete} del)", file=STDERR)

    return best_action


def solve():
    problem_data = problem_input()
    # best_action = greedy(problem_data)
    best_action = random_walls(problem_data)

    problem_output(best_action)


def main():
    solve()


if __name__ == "__main__":
    main()
