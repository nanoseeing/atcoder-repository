import copy
import heapq
import math
import random
import time
from collections import defaultdict

INF = 10**18
IS_ONLINE_JUDGE = False

DEBUG: bool = True
MAX_TIME: float = 1.8  # sec
INIT_TEMP: float = 5000.0
MIN_TEMP: float = 1.0
MIN_L: int = 5_000
MAX_L: int = 500_000


# =================================
def serach_min_k(nums: list, k):
    """listからk番目に小さい数とそのインデックスを返す"""
    ind_nums = [(inds, num) for inds, num in enumerate(nums)]
    ind_nums.sort(key=lambda x: x[1])
    ind = ind_nums[k - 1][0]
    num = ind_nums[k - 1][1]
    return ind, num


def calc_dist(p1: tuple, p2: tuple) -> float:
    x_dff = p1[0] - p2[0]
    y_dff = p1[1] - p2[1]
    dist = (x_dff**2 + y_dff**2) ** 0.5
    return dist


def exponential_schedule(init: float, obj: float, elapsed_time: float, max_time: float) -> float:
    """
    時間に基いて指数関数的に減衰・増加
    """
    lambda_param = math.log(obj / init) / max_time
    return init * math.exp(lambda_param * elapsed_time)


def linear_schedule(init: float, obj: float, elapsed_time: float, max_time: float) -> float:
    """
    時間に基づく線形減衰・増加
    """
    return init + (obj - init) * (elapsed_time / max_time)


def prim(
    graph: dict[int, list[tuple[int, int]]],
) -> tuple[list[tuple[int, int]], list[int]]:
    """
    G: 隣接グラフ
    G := [ [(v_0, cost_0), (v_1,cost_1),..], [(v_2, cost_2)],...]

    返り値 ans :最小全域木の重みの総和
    """
    v_list = list(graph.keys())
    used = {v: False for v in v_list}

    init_v = v_list[0]  # 適当な点を選ぶ
    used[init_v] = True
    que = [(cost, init_v, v) for v, cost in graph[init_v]]
    heapq.heapify(que)

    ans_v: list[int] = [init_v]
    ans_edges: list[tuple[int, int]] = []
    while que:
        cost_v, from_v, to_v = heapq.heappop(que)
        if used[to_v]:
            continue
        used[to_v] = True
        ans_edges.append((min(from_v, to_v), max(from_v, to_v)))
        ans_v.append(to_v)
        for nxt, cost_nxt in graph[to_v]:
            if used[nxt]:
                continue
            heapq.heappush(que, (cost_nxt, to_v, nxt))
    return ans_edges, ans_v


def prim_k(graph: dict[int, list[tuple[int, int]]], init_v: int, k: int, used_v: set[int]):
    """
    init_vを含むk頂点の最小全域木を求める
    """
    used = copy.copy(used_v)
    used.add(init_v)
    que = [(cost, init_v, v) for v, cost in graph[init_v]]
    heapq.heapify(que)

    ans_v = [init_v]
    ans_edges: list[tuple[int, int]] = []
    ans_cost = 0
    while que:
        if len(ans_v) >= k:
            break
        cost_v, from_v, to_v = heapq.heappop(que)
        if to_v in used:
            continue
        used.add(to_v)
        ans_edges.append((min(from_v, to_v), max(from_v, to_v)))
        ans_v.append(to_v)
        ans_cost += cost_v
        for nxt, cost_nxt in graph[to_v]:
            if nxt in used:
                continue
            heapq.heappush(que, (cost_nxt, to_v, nxt))
    return ans_edges, ans_v, ans_cost


# =================================


class EnvOffline:
    def __init__(self, input_file_path: str, output_file_path: str):
        with open(input_file_path) as f:
            lines = f.readlines()
            N, M, Q, L, W = map(int, lines[0].split())
            G = list(map(int, lines[1].split()))
            rectangles = []
            for l in range(2, 2 + N):
                rectangles.append(list(map(int, lines[l].split())))
            coordinates = []
            for l in range(2 + N, 2 + N + N):
                coordinates.append(tuple(map(int, lines[l].split())))

        self.N, self.M, self.Q, self.L, self.W = N, M, Q, L, W
        self.G = G
        self.rectangles = rectangles
        self.coordinates = coordinates
        self.cneter_points = [[(l + r) / 2, (c + d) / 2] for l, r, c, d in rectangles]
        self._query_history: list[str] = []

        self.input_file_path = input_file_path
        self.output_file_path = output_file_path

    def query(self, c_list: list[int]) -> list[tuple[int, int]]:
        query_str = " ".join(["?", str(len(c_list)), *map(str, c_list)])
        self._query_history.append(query_str)

        now_graph: dict[int, list] = {v: [] for v in c_list}
        for c1 in c_list:
            for c2 in c_list:
                if c1 == c2:
                    continue
                x1, y1 = self.coordinates[c1]
                x2, y2 = self.coordinates[c2]
                dist = abs(x1 - x2) ** 2 + abs(y1 - y2) ** 2
                now_graph[c1].append((c2, dist))
                now_graph[c2].append((c1, dist))

        ans_edges, ans_v = prim(now_graph)
        return ans_edges

    def answer(
        self,
        groups: list[list[int]],
        edges: list[list[tuple[int, int]]],
    ):
        cost = 0.0
        ans_str = "!\n"
        for i in range(len(groups)):
            ans_str += " ".join(map(str, groups[i])) + "\n"
            for e in edges[i]:
                ans_str += " ".join(map(str, e)) + "\n"
                dist = calc_dist(self.coordinates[e[0]], self.coordinates[e[1]])
                cost += dist

        for query_str in self._query_history:
            ans_str += query_str + "\n"

        with open(self.output_file_path, "w") as f:
            f.write(ans_str)

        return cost


class EnvOnline:
    def __init__(self):
        N, M, Q, L, W = map(int, input().split())
        G = list(map(int, input().split()))
        rectangles = []
        for l in range(N):
            rectangles.append(list(map(int, input().split())))

        self.N, self.M, self.Q, self.L, self.W = N, M, Q, L, W
        self.G = G
        self.rectangles = rectangles
        self.cneter_points = [[(l + r) / 2, (c + d) / 2] for l, r, c, d in rectangles]

    def query(self, c_list: list[int]) -> list[tuple[int, int]]:
        query_str = " ".join(["?", str(len(c_list)), *map(str, c_list)])
        print(query_str)
        return [tuple(map(int, input().split())) for _ in range(len(c_list) - 1)]

    def answer(
        self,
        groups: list[list[int]],
        edges: list[list[tuple[int, int]]],
    ):
        ans_str = "!\n"
        for i in range(len(groups)):
            ans_str += " ".join(map(str, groups[i])) + "\n"
            for e in edges[i]:
                ans_str += " ".join(map(str, e)) + "\n"
        print(ans_str)


# =================================


def neighbor_function(x):
    return x


def calc_score(x):
    return 0.0


class MSTAnnealing:
    def __init__(
        self,
        estimate_points: list[tuple[float, float]],
        groups: list[int],
    ):
        self.estimate_points = estimate_points
        self.groups = groups
        self.N = len(estimate_points)
        self.G = len(groups)

        graph: list[list[tuple[int, float]]] = [[] for _ in range(self.N)]
        dists: dict[tuple[int, int], float] = {}
        for i in range(self.N):
            for j in range(self.N):
                if i == j:
                    continue
                dist = calc_dist(self.estimate_points[i], self.estimate_points[j])
                graph[i].append((j, dist))
                graph[j].append((i, dist))
                dists[(i, j)] = dist
                dists[(j, i)] = dist
        self.graph = graph

    def greedy_solve(self):
        groups = [(i, g) for i, g in enumerate(self.G)]
        groups = sorted(groups, key=lambda x: x[1], reverse=True)

        ans_edges = [None for _ in range(len(groups))]
        ans_v = [None for _ in range(len(groups))]

        now_used: set = set()
        not_used: set = set(range(self.N))
        for group_n, group_size in groups:
            v = not_used.pop()
            prim_edges, prim_v, cost = prim_k(self.graph, v, group_size, now_used)

            ans_edges[group_n] = prim_edges
            ans_v[group_n] = prim_v
            now_used.update(prim_v)
            not_used.difference_update(prim_v)

        return ans_v, ans_edges

    def simulated_annealing(
        self,
        x0: tuple[list[int], list[int]],
        t0: float,
        tmin: float,
        simulate_time: float,
        display: bool,
        display_interval: int = 100,
        obj: str = "min",
    ) -> tuple[tuple[list[int], list[int]], float]:
        if obj == "min":
            score_pm = 1
        elif obj == "max":
            score_pm = -1
        else:
            raise ValueError("obj must be 'min' or 'max'")

        x = x0
        current_cost = score_pm * calc_score(x)  # 初期解のコストを計算
        best_x = x
        best_cost = current_cost

        start_time = time.time()  # 開始時刻を記録
        iteration = 0

        while True:
            elapsed_time = time.time() - start_time
            if elapsed_time >= simulate_time:
                break
            temp = exponential_schedule(t0, tmin, elapsed_time, simulate_time)
            if temp < tmin:
                break

            # 近傍解を生成
            new_x = neighbor_function(x)
            new_cost = score_pm * calc_score(x)
            delta_cost = new_cost - current_cost

            # 改善されるなら更新、悪化しても確率的に更新
            if delta_cost < 0 or random.random() < math.exp(-delta_cost / temp):
                x = new_x
                current_cost = new_cost

            # 最良解の更新
            if current_cost < best_cost:
                best_x = x
                best_cost = current_cost

            iteration += 1
            if display and iteration % display_interval == 0:
                print(f"Iteration: {iteration}, Best cost: {best_cost}, Current cost: {current_cost}")

        return best_x, best_cost


def solve(env: EnvOffline):
    target_points = env.cneter_points
    graph = defaultdict(list)
    for i in range(env.N):
        for j in range(env.N):
            if i == j:
                continue
            dist = calc_dist(target_points[i], target_points[j])
            graph[i].append((j, dist))
            graph[j].append((i, dist))

    groups = [(i, g) for i, g in enumerate(env.G)]
    groups = sorted(groups, key=lambda x: x[1], reverse=True)

    ans_edges = [None for _ in range(len(groups))]
    ans_v = [None for _ in range(len(groups))]

    now_used: set = set()
    not_used: set = set(range(env.N))
    for group_n, group_size in groups:
        now_points = [target_points[p] for p in not_used]
        v = not_used.pop()
        # v = search_min_max_k_dist(list(not_used), now_points, group_size)
        prim_edges, prim_v, cost = prim_k(graph, v, group_size, now_used)

        ans_edges[group_n] = prim_edges
        ans_v[group_n] = prim_v
        now_used.update(prim_v)
        not_used.difference_update(prim_v)

    return ans_v, ans_edges


def search_min_max_k_dist(point_inds: list[int], points: list[tuple[float, float]], k: int):
    """ある点からk近傍を計算し、最も距離の小さい点を返す"""

    if k <= 1:
        return point_inds[0]

    P = len(points)
    dists: list[list[float]] = [[] for _ in range(P)]
    for p1 in range(P):
        for p2 in range(P):
            if p1 == p2:
                continue
            dist = calc_dist(points[p1], points[p2])
            dists[p1].append(dist)
            dists[p2].append(dist)

    knns = [serach_min_k(dists[p], k)[1] for p in range(P)]
    min_knn_index = serach_min_k(knns, 1)[0]
    return point_inds[min_knn_index]


def main():
    if IS_ONLINE_JUDGE:
        env = EnvOnline()
        ans_v, ans_edges = solve(env)
        env.answer(ans_v, ans_edges)
    else:
        FILE_NUM = 100
        costs = []
        for file_num in range(FILE_NUM):
            input_file_path = f"../in/{file_num:04d}.txt"
            output_file_path = f"../out/{file_num:04d}.txt"
            env = EnvOffline(input_file_path, output_file_path)
            ans_v, ans_edges = solve(env)
            cost = env.answer(ans_v, ans_edges)
            print(f"[{file_num}/{FILE_NUM}]: {cost}")
            costs.append(cost)
        print(f"avg: {sum(costs) / len(costs)}")
        print(f"max: {max(costs)}")
        with open("../out/costs.txt", "w") as f:
            for cost in costs:
                f.write(f"{cost}\n")


if __name__ == "__main__":
    main()
