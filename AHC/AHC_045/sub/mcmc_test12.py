#!/usr/bin/env python
# coding: utf-8

# =================================
# 設定
# =================================
IS_ONLINE_JUDGE = False

LIMIT_QUERY_CNT = None

DEBUG = False
DEBUG_QUERY = None
DEBUG_L = None

MAX_TIME = 0.1
FILE_NUM = 100

# =================================
# 初期化
# =================================
import random

random.seed(1111)

if IS_ONLINE_JUDGE:
    DEBUG = False


# =================================
# Import
# =================================
import heapq
import math
import time
from abc import ABC, abstractmethod
from collections import defaultdict, deque


# =================================
# 環境
# =================================
class Env(ABC):
    N: int
    M: int
    Q: int
    L: int
    W: int
    G: list[int]
    rectangles: list[list[int]]
    cneter_points: list[tuple[int, int]]
    _query_history: list[str]

    @abstractmethod
    def query(self, c_list: list[int]) -> list[tuple[int, int]]:
        pass

    @abstractmethod
    def answer(
        self,
        groups: list[list[int]],
        edges: list[list[tuple[int, int]]],
    ):
        pass

    def read_first_line(self, line: str):
        N, M, Q, L, W = map(int, line.split())
        self.N = N
        self.M = M
        self.Q = Q
        self.L = L
        self.W = W

    def read_second_line(self, lines: list[str]):
        self.G = list(map(int, lines[0].split()))

        rectangles = []
        for line in lines[1:]:
            rectangles.append(list(map(int, line.split())))
        self.rectangles = rectangles

        self.cneter_points = [((l + r) // 2, (c + d) // 2) for l, r, c, d in rectangles]


class EnvOnline(Env):
    def __init__(self):
        self.read_first_line(input())
        self.read_second_line([input() for _ in range(self.N + 1)])

        self._query_history: list[str] = []

    def query(self, c_list: list[int]):
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


class EnvOffline(Env):
    def __init__(
        self,
        input_file_path: str,
        output_file_path: str,
        DEBUG: bool = False,
        DEBUG_L: int | None = None,
        DEBUG_QUERY: int | None = None,
    ):
        with open(input_file_path) as f:
            lines = f.readlines()
            self.read_first_line(lines[0])
            self.read_second_line(lines[1 : 2 + self.N])

            coordinates: list[tuple[int, int]] = []
            for l in range(2 + self.N, 2 + self.N * 2):
                coordinates.append(tuple(map(int, lines[l].split())))
            self.coordinates: list[tuple[int, int]] = coordinates

        self.input_file_path = input_file_path
        self.output_file_path = output_file_path
        self._query_history: list[str] = []

        if DEBUG:
            self.L = DEBUG_L if DEBUG_L is not None else self.L
            self.Q = DEBUG_QUERY if DEBUG_QUERY is not None else self.Q

    def query(self, c_list: list[int]) -> list[tuple[int, int]]:
        query_str = " ".join(["?", str(len(c_list)), *map(str, c_list)])
        self._query_history.append(query_str)
        ans_edges, _ = kruskals_algorithm(construct_sorted_edges({v: self.coordinates[v] for v in c_list}), c_list)
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

        if len(self._query_history) > self.Q:
            print(f"!!!query limit exceeded {len(self._query_history)}/{self.Q}")
        else:
            print(f"query: {len(self._query_history)}")

        return cost


# =================================
# utils
# =================================
def sort_pair(t: tuple) -> tuple:
    """ペアをソートする"""
    a, b = t
    if a > b:
        return b, a
    return (a, b)


# =================================
# 幾何計算
# =================================
def calc_dist(p1: tuple, p2: tuple) -> int:
    """問題専用距離計算"""
    return int(math.dist(p1, p2))


# =================================
# グラフアルゴリズムアルゴリズム
# =================================
def cut_graph(
    graph: dict[int, list[int]],  # 隣接リスト
    cut_edge: tuple[int, int],  # 切断する辺
):
    def _bfs(
        graph: dict[int, list[int]],  # 隣接リスト
        init_v,  # 初期頂点,
        not_visit_v: int,  # 訪問しない頂点
    ):
        visited = set([init_v, not_visit_v])
        q = deque([init_v])
        while q:
            node = q.pop()
            for nv in graph[node]:
                if nv not in visited:
                    visited.add(nv)
                    q.append(nv)
        return visited - {not_visit_v}

    v1, v2 = cut_edge
    group1 = _bfs(graph, v1, v2)
    group2 = _bfs(graph, v2, v1)
    return group1, group2


def construct_dist_matrix(points: list[tuple[int, int]]):
    """点の距離行列を構築する"""
    N = len(points)
    dist_matrix = [[0.0] * N for _ in range(N)]
    for i in range(N):
        for j in range(i + 1, N):
            dist = calc_dist(points[i], points[j])
            dist_matrix[i][j] = dist
            dist_matrix[j][i] = dist
    return dist_matrix


def construct_graph(points: dict[int, tuple[int, int]]):
    """点の隣接リストを構築する"""
    graph: dict[int, list[tuple[int, int]]] = {v: [] for v in points.keys()}
    for v1 in points:
        for v2 in points:
            if v1 == v2:
                continue
            dist = calc_dist(points[v1], points[v2])
            graph[v1].append((dist, v2))
    for v in points:
        graph[v].sort()

    return graph


def construct_graph_from_edges(
    edges: list[tuple[int, int]],  # 辺のリスト
    vs: list[int],  # 頂点のリスト
):
    """隣接リストを構築する"""
    graph: dict[int, list[int]] = {v: [] for v in vs}
    for e in edges:
        v1, v2 = e
        graph[v1].append((v2))
        graph[v2].append((v1))
    return graph


def prim_k(
    graph: list[list[float]],  # 全体の隣接行列
    init_v: int,
    k: int,
    used_v: set[int],
    N: int,
):
    """init_vを含むk頂点の最小全域木を求める"""
    used = used_v | {init_v}
    used.add(init_v)
    que = [(graph[init_v][v], init_v, v) for v in range(N)]
    heapq.heapify(que)

    ans_v = [init_v]
    ans_edges: list[tuple[int, int]] = []
    ans_cost = 0.0
    while que and len(ans_v) < k:
        cost_v, from_v, to_v = heapq.heappop(que)
        if to_v in used:
            continue
        used.add(to_v)
        ans_edges.append(sort_pair((from_v, to_v)))
        ans_v.append(to_v)
        ans_cost += cost_v
        for nxt in range(N):
            if nxt in used:
                continue
            heapq.heappush(que, (graph[to_v][nxt], to_v, nxt))
    return ans_edges, ans_v, ans_cost


def construct_sorted_edges(
    coords: dict[int, tuple[int, int]],  # 点のリスト
):
    """ソート済みの辺リストを構築する"""
    edges = []
    for v1 in coords:
        for v2 in coords:
            if v1 == v2:
                continue
            dist = calc_dist(coords[v1], coords[v2])
            edges.append((dist, v1, v2))
    return sorted(edges)


def kruskals_algorithm(edges: list[tuple[float, int, int]], vs: list[int]):
    """最小全域木[クラスカル法] O(ElogV)
    edges : (c, a, b) 事前にソートしておくこと
    """
    uf = UnionFind(list(vs))
    cost = 0.0

    ans_edges = []
    for edge in edges:
        c, a, b = edge
        if not uf.same(a, b):
            cost += c
            uf.union(a, b)
            ans_edges.append(sort_pair((a, b)))
    return ans_edges, cost


class UnionFind:
    def __init__(self, vs: list[int]):
        self.vs = vs
        self.parents = {v: -1 for v in vs}

    def find(self, x):
        if self.parents[x] < 0:
            return x
        else:
            self.parents[x] = self.find(self.parents[x])
            return self.parents[x]

    def union(self, x, y):
        x = self.find(x)
        y = self.find(y)

        if x == y:
            return

        if self.parents[x] > self.parents[y]:
            x, y = y, x

        self.parents[x] += self.parents[y]
        self.parents[y] = x

    def size(self, x):
        return -self.parents[self.find(x)]

    def same(self, x, y):
        return self.find(x) == self.find(y)

    def members(self, x):
        root = self.find(x)
        return [v for v in self.vs if self.find(v) == root]

    def roots(self):
        return [v for v, x in self.parents.items() if x < 0]

    def group_count(self):
        return len(self.roots())

    def all_group_members(self):
        ret = dict()
        for v in self.vs:
            r = self.find(v)
            if r in ret:
                ret[r].append(v)
            else:
                ret[r] = [v]
        return ret

    def __str__(self):
        return "\n".join("{}: {}".format(r, self.members(r)) for r in self.roots())


# =================================
# 焼きなまし
# =================================
def exponential_schedule(init: float, obj: float, elapsed_time: float, max_time: float) -> float:
    """時間に基いて指数関数的に上昇するスケジュール"""
    lambda_param = math.log(obj / init) / max_time
    return init * math.exp(lambda_param * elapsed_time)


def linear_schedule(init: float, obj: float, elapsed_time: float, max_time: float) -> float:
    """時間に基づく線形減衰・増加"""
    return init + (obj - init) * (elapsed_time / max_time)


# =================================
# 汎用（main.py専用）
# =================================
def debug_print(*args, **kwargs):
    if IS_ONLINE_JUDGE:
        return
    print(*args, **kwargs)


def cost_formula(d1: int, d2: int) -> int:
    if d1 > d2:
        return d1 - d2
    else:
        return 0


def calc_init_cost(
    constrains: list[tuple[tuple[int, int], tuple[int, int]]],
    x: list[tuple[int, int]],
) -> tuple[list[int], int]:
    cost_list: list[int] = [0 for _ in range(len(constrains))]
    for ci in range(len(constrains)):
        cons = constrains[ci]
        (p1, p2), (q1, q2) = cons
        dist1 = calc_dist(x[p1], x[p2])
        dist2 = calc_dist(x[q1], x[q2])
        cost_list[ci] = cost_formula(dist1, dist2)
    total_cost = sum(cost_list)
    return cost_list, total_cost


def calc_cost(
    constrains: list[tuple[tuple[int, int], tuple[int, int]]],
    x_now: list[tuple[int, int]],
    x_update: dict[int, tuple[int, int]],
    cost_list: list[int],
    cost_now: int,
    vid_to_constid: dict[int, set[int]],
) -> tuple[dict[int, int], int]:
    new_cost = cost_now
    cost_update = dict()
    for v in x_update:
        for consid in vid_to_constid[v]:
            (p1, p2), (q1, q2) = constrains[consid]
            coord1 = x_update[p1] if p1 in x_update else x_now[p1]
            coord2 = x_update[p2] if p2 in x_update else x_now[p2]
            coord3 = x_update[q1] if q1 in x_update else x_now[q1]
            coord4 = x_update[q2] if q2 in x_update else x_now[q2]
            dist1 = calc_dist(coord1, coord2)
            dist2 = calc_dist(coord3, coord4)
            pre_cost = cost_list[consid]
            now_cost = cost_formula(dist1, dist2)
            new_cost += now_cost - pre_cost
            cost_update[consid] = now_cost
    return cost_update, new_cost


def neighbor(
    x: list[tuple[int, int]],
    target_v: list[int],
    rects: list[list[int]],
    now_mul_x: float,
    k: int = 1,
):
    vs: list[int] = random.sample(target_v, k=k)
    x_update: dict[int, tuple[int, int]] = dict()

    for v in vs:
        rnd = random.random()
        rand_coord = tuple()
        if rnd < 0.99:
            x1, x2, y1, y2 = rects[v]
            nx, ny = x[v]
            dx = x2 - x1
            dy = y2 - y1
            max_dx = max(int(dx * now_mul_x), 1)
            max_dy = max(int(dy * now_mul_x), 1)
            lim_l = max(x1, nx - max_dx)
            lim_r = min(x2, nx + max_dx)
            lim_u = max(y1, ny - max_dy)
            lim_d = min(y2, ny + max_dy)
            rand_coord = random.randint(lim_l, lim_r), random.randint(lim_u, lim_d)
        else:
            x1, x2, y1, y2 = rects[v]
            rand_coord = random.randint(x1, x2), random.randint(y1, y2)

        x_update[v] = rand_coord
        nx, ny = x[v]
        x1, x2, y1, y2 = rects[v]
        if x1 == x2:
            mul_x = 0.01
        else:
            mul_x = abs(nx - rand_coord[0]) / (x2 - x1)

    return x_update, mul_x


def simulated_annealing(
    x0: list[tuple[int, int]],
    t0: float,
    t1: float,
    max_time: float,
    init_mul_x: float,
    constrains: list[tuple[tuple[int, int], tuple[int, int]]],
    rectangles: list[list[int]],
    target_vs: list[int],
    vid_to_constid: dict[int, set[int]],
    display: bool = False,
):
    x = x0.copy()
    x_best = x
    x_update = dict()

    cost_list, cost_current = calc_init_cost(constrains, x)
    cost_best = cost_current
    cost_update: dict[int, int] = dict()

    simulate_cnt = 0
    good_cnt = 0
    st = time.perf_counter()

    sum_mul_x = 0.0
    now_mul_x = init_mul_x
    while True:
        elapsed_time = time.perf_counter() - st
        if elapsed_time >= max_time:
            break
        temp = exponential_schedule(t0, t1, elapsed_time, max_time)

        x_update, mul_x = neighbor(x, target_vs, rectangles, now_mul_x)
        cost_update, cost_new = calc_cost(
            constrains=constrains,
            x_now=x,
            x_update=x_update,
            cost_list=cost_list,
            cost_now=cost_current,
            vid_to_constid=vid_to_constid,
        )

        delta_cost = cost_new - cost_current
        if delta_cost < 0:
            good_cnt += 1
            sum_mul_x += mul_x
        if delta_cost <= 0 or random.random() < math.exp(-delta_cost / temp):
            cost_current = cost_new
            for v in x_update:
                x[v] = x_update[v]
            for ci in cost_update:
                cost_list[ci] = cost_update[ci]
        if cost_current < cost_best:
            x_best = x
            cost_best = cost_current

        simulate_cnt += 1
        if display and simulate_cnt % 5000 == 0:
            debug_print(
                f"i: {simulate_cnt}, cost_now: {cost_current}, best_cost: {cost_best}, temp: {temp:.2f}, good_cnt: {good_cnt}/{simulate_cnt}"
            )
        if good_cnt > 0 and good_cnt % 100 == 0:
            now_mul_x = max(min(0.01, sum_mul_x / good_cnt), 0.5)
            sum_mul_x = 0.0
    return x_best, cost_best


def calc_greedy_answer(
    env: Env,
    target_points: list[tuple[int, int]],
):
    graph = construct_dist_matrix(target_points)

    groups = [(i, g) for i, g in enumerate(env.G)]
    groups = sorted(groups, key=lambda x: x[1], reverse=True)

    ans_edges = [None for _ in range(len(groups))]
    ans_v = [None for _ in range(len(groups))]
    ans_costs = [None for _ in range(len(groups))]

    now_used: set = set()
    not_used: set = set(range(env.N))
    for group_n, group_size in groups:
        v = not_used.pop()
        prim_edges, prim_v, prim_cost = prim_k(graph, v, group_size, now_used, env.N)
        ans_edges[group_n] = prim_edges
        ans_v[group_n] = prim_v
        ans_costs[group_n] = prim_cost
        now_used.update(prim_v)
        not_used.difference_update(prim_v)

    return ans_v, ans_edges, ans_costs


def solve(env: Env):
    all_graph: dict[int, list[tuple[int, int]]] = construct_graph({i: p for i, p in enumerate(env.cneter_points)})

    max_d_list: list[tuple[int, int]] = []
    for i, (l, r, t, b) in enumerate(env.rectangles):
        dx = r - l
        dy = b - t
        max_d_list.append((i, dx + dy))  # あえて辺の合計にする
    max_d_list.sort(key=lambda x: x[1], reverse=True)
    max_d_points: list[int] = [p for i, p in max_d_list]

    constrains = set()
    random_vs = set()
    for big_v, _ in max_d_list[: env.Q]:
        other_points: list[tuple[int, int]] = all_graph[big_v][: env.L * 2]
        other_points_ind: list[int] = [p for d, p in other_points]
        other_points_ind = sorted(other_points_ind, key=lambda x: max_d_points[x], reverse=True)
        other_points_ind = other_points_ind[: env.L - 1]

        tmp_random_vs: list[int] = [big_v] + other_points_ind

        edges = env.query(tmp_random_vs)
        graph = construct_graph_from_edges(edges, tmp_random_vs)

        for e in edges:
            group_vs1, group_vs2 = cut_graph(graph, e)
            for v1 in group_vs1:
                for v2 in group_vs2:
                    sv1, sv2 = sort_pair((v1, v2))
                    if e == (sv1, sv2):
                        continue
                    constrains.add((e, (sv1, sv2)))
        random_vs.update(tmp_random_vs)
    constrains_list = list(constrains)
    random_vs_list = list(random_vs)

    vid_to_constid: dict[int, set[int]] = defaultdict(set)
    for i, const in enumerate(constrains_list):
        (p1, p2), (q1, q2) = const
        vid_to_constid[p1].add(i)
        vid_to_constid[p2].add(i)
        vid_to_constid[q1].add(i)
        vid_to_constid[q2].add(i)

    x_bests: list[list[tuple[int, int]]] = []
    for _ in range(1):
        x_best, cost_best = simulated_annealing(
            x0=env.cneter_points,
            t0=10.0,
            t1=1.0,
            max_time=MAX_TIME,
            init_mul_x=0.5,
            constrains=constrains_list,
            rectangles=env.rectangles,
            target_vs=random_vs_list,
            vid_to_constid=vid_to_constid,
            display=True,
        )
        x_bests.append(x_best)

    x_best_mean: list[tuple[int, int]] = []
    for i in range(env.N):
        total_x = 0
        total_y = 0
        for j in range(len(x_bests)):
            total_x += x_bests[j][i][0]
            total_y += x_bests[j][i][1]
        total_x //= len(x_bests)
        total_y //= len(x_bests)
        x_best_mean.append((total_x, total_y))

    ans_v, ans_edges, _ = calc_greedy_answer(env, x_best_mean)
    return env.answer(ans_v, ans_edges)


def main():
    if IS_ONLINE_JUDGE:
        solve(EnvOnline())
    else:
        costs = []
        for file_num in range(FILE_NUM):
            debug_print("=====")
            start_time = time.perf_counter()
            input_file_path = f"../in/{file_num:04d}.txt"
            output_file_path = f"../out/{file_num:04d}.txt"
            #####
            env = EnvOffline(input_file_path, output_file_path)
            cost = solve(env)
            #####
            debug_print(f"[{file_num}/{FILE_NUM}]: {cost} {time.perf_counter() - start_time:.2f}s")
            costs.append(cost)
        avg = sum(costs) / len(costs)
        debug_print(f"avg: {avg}")
        debug_print(f"max: {max(costs)}")
        with open("../result/mcmc10.txt", "w") as f:
            f.write(f"avg: {avg}\n")
            for pi, cost in enumerate(costs):
                f.write(f"{pi:0<4} {cost}\n")


if __name__ == "__main__":
    main()
