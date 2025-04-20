#!/usr/bin/env python
# coding: utf-8

# =================================
# 設定
# =================================
IS_ONLINE_JUDGE = False

DEBUG = True
DEBUG_QUERY = None
DEBUG_L = None

MAX_TIME = 1.25
FILE_NUM = 100

# =================================
# 初期化
# =================================
import time

GLOBAL_START_TIME = time.perf_counter()

import random

random.seed(0)

if IS_ONLINE_JUDGE:
    DEBUG = False

import heapq

# =================================
# Import
# =================================
import math
import random
from abc import ABC, abstractmethod
from collections import deque

import _pickle as cPickle


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

            coordinates = []
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
def my_deepcopy(obj):
    return cPickle.loads(cPickle.dumps(obj, -1))


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


def calc_centroid(points: list[tuple[int, int]]) -> tuple[int, int]:
    """点の重心を計算する"""
    x_sum = sum(p[0] for p in points)
    y_sum = sum(p[1] for p in points)
    n = len(points)
    centroid = (x_sum // n, y_sum // n)
    return centroid


# =================================
# グラフアルゴリズム
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
    dist_matrix = [[0] * N for _ in range(N)]
    for i in range(N):
        for j in range(i + 1, N):
            dist = calc_dist(points[i], points[j])
            dist_matrix[i][j] = dist
            dist_matrix[j][i] = dist
    return dist_matrix


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


def prim_k(
    graph: list[list[int]],  # 全体の隣接行列
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
    ans_cost: int = 0
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


def prim_vs(
    graph: list[list[int]],  # 全体の隣接行列
    vs: list[int],  # 頂点のリスト
):
    init_v = vs[0]

    used = {init_v}
    que = [(graph[init_v][v], init_v, v) for v in vs]
    heapq.heapify(que)

    ans_edges: list[tuple[int, int]] = []
    ans_cost: int = 0
    while que and len(ans_edges) < len(vs) - 1:
        cost_v, from_v, to_v = heapq.heappop(que)
        if to_v in used:
            continue
        used.add(to_v)
        ans_edges.append(sort_pair((from_v, to_v)))
        ans_cost += cost_v
        for nxt in vs:
            if nxt in used:
                continue
            heapq.heappush(que, (graph[to_v][nxt], to_v, nxt))
    return ans_edges, ans_cost


def kruskals_algorithm(edges: list[tuple[int, int, int]], vs: list[int]):
    """最小全域木[クラスカル法] O(ElogV)
    edges : (c, a, b) 事前にソートしておくこと
    """
    uf = UnionFind(list(vs))
    cost = 0

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
# Helper関数
# =================================
def calc_point_avg_distances(points1, points2):
    distances = []
    for p1, p2 in zip(points1, points2):
        dist = math.dist(p1, p2)
        distances.append(dist)
    avg = sum(distances) / len(distances)
    return avg


# =================================
# 汎用（main.py専用）
# =================================
def debug_print(*args, **kwargs):
    if IS_ONLINE_JUDGE:
        return
    print(*args, **kwargs)


def print_elapsed_time():
    debug_print(f"elapsed time: {(time.perf_counter() - GLOBAL_START_TIME) * 1000:.2f}msecs")


# =================================
# 以下メイン処理
# =================================
class MyGraph:
    def __init__(self, vs: list[int], edges: list[tuple[int, int]], group_ind: int):
        self.vs: list[int] = vs
        self.edges: list[tuple[int, int]] = edges
        self.group_ind: int = group_ind

        self.visited_cnt: dict[int, int] = {v: 0 for v in vs}
        self._construct_graph()

    def _construct_graph(self):
        graph: dict[int, list[int]] = {v: [] for v in self.vs}
        for i, e in enumerate(self.edges):
            graph[e[0]].append(e[1])
            graph[e[1]].append(e[0])
        self.graph: dict[int, list[int]] = graph

    def update_edges(self, remove_edge: list[tuple[int, int]], add_edges: list[tuple[int, int]]):
        self.edges = list(set(self.edges) - set(remove_edge) | set(add_edges))
        self._construct_graph()

    def choice_min_visited(self):
        min_v = min(self.visited_cnt.values())
        min_vs = [v for v, cnt in self.visited_cnt.items() if cnt == min_v]
        return random.choice(min_vs)

    def walk_bfs(self, init_v: int, q_num: int):
        visited_v: set[int] = set()
        visited_edge: set[tuple[int, int]] = set()
        q: list[tuple[int, int]] = [(-1, init_v)]
        while q and len(visited_v) < q_num:
            random.shuffle(q)
            pv, v = q.pop()
            if v in visited_v:
                continue
            if pv != -1:
                visited_edge.add((min(pv, v), max(pv, v)))
            visited_v.add(v)
            for nv in self.graph[v]:
                if nv not in visited_v:
                    q.append((v, nv))

        for v in visited_v:
            self.visited_cnt[v] += 1

        visited_v_list = list(visited_v)
        visited_edge_list = list(visited_edge)
        return visited_v_list, visited_edge_list


def define_query_cnt(env: Env, limit_query_cnt: int):
    group_query_cnt: list[int] = [0 for _ in range(env.M)]
    group_query_v_cnt: list[int] = [0 for _ in range(env.M)]

    largest_groups: list[int] = []
    for g in range(env.M):
        if 3 <= env.G[g]:
            group_query_cnt[g] += 1
            group_query_v_cnt[g] += env.L
            if env.L < env.G[g]:
                largest_groups.append(g)

    for _ri in range(env.Q - sum(group_query_cnt)):
        min_cost = 10**9
        target_group = -1
        for g in largest_groups:
            cost = group_query_v_cnt[g] / env.G[g]
            if cost < min_cost:
                min_cost = cost
                target_group = g
        if target_group != -1:
            group_query_v_cnt[target_group] += env.L
            group_query_cnt[target_group] += 1
            if limit_query_cnt is not None and min_cost >= limit_query_cnt:
                break
    return group_query_cnt


def update_mst(
    env: Env,
    ans_v: list[list[int]],
    ans_edges: list[list[tuple[int, int]]],
    group_query_cnt: dict[int, int],
):
    for g in range(env.M):
        now_v: list[int] = ans_v[g]
        now_edges: list[tuple[int, int]] = ans_edges[g]
        if len(now_v) < 3:
            if group_query_cnt[g] > 0:
                print(g, "ERROR")
        elif 3 <= len(now_v) <= env.L:
            ans_edges[g] = env.query(now_v)
        else:
            my_graph = MyGraph(now_v, now_edges, g)
            for _ in range(group_query_cnt[g]):
                rand_v = my_graph.choice_min_visited()
                visited_v, visited_edge = my_graph.walk_bfs(rand_v, env.L)
                new_edges = env.query(visited_v)
                my_graph.update_edges(visited_edge, new_edges)
            ans_edges[g] = my_graph.edges

    return ans_v, ans_edges


def calc_greedy_answer(env: Env, target_points: list[tuple[int, int]]):
    graph: list[list[int]] = construct_dist_matrix(target_points)

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


# =================================
# 焼きなまし
# =================================
def neighbor_function_define_group(
    vs: list[list[int]],
    costs: list[int],
    centroids: list[tuple[int, int]],
    coords: list[tuple[int, int]],
    graph: list[list[int]],
):
    """近傍解"""
    MAX_G_NUM = 5
    next_vs = my_deepcopy(vs)
    G = len(vs)

    weighted_costs = [cost / len(vs_list) for vs_list, cost in zip(vs, costs)]
    g1 = random.choices(list(range(G)), weights=weighted_costs, k=1)[0]

    # 重心からの距離が近いグループを選ぶ
    now_centroid = centroids[g1]
    dists = [1 / math.dist(centroids[g], now_centroid) for g in range(G) if g != g1]
    other_inds = [g for g in range(G) if g != g1]
    g2 = random.choices(other_inds, weights=dists, k=1)[0]

    g1_num = len(next_vs[g1])
    g2_num = len(next_vs[g2])
    g_num = random.randint(1, min(MAX_G_NUM, min(g1_num, g2_num)))

    v1 = random.sample(next_vs[g1], k=g_num)
    v2 = random.sample(next_vs[g2], k=g_num)

    next_vs[g1] = list(set(next_vs[g1]) - set(v1))
    next_vs[g2] = list(set(next_vs[g2]) - set(v2))
    next_vs[g1] += v2
    next_vs[g2] += v1

    g1_edges, g1_cost = prim_vs(graph, next_vs[g1])
    g2_edges, g2_cost = prim_vs(graph, next_vs[g2])

    g1_centroid = calc_centroid([coords[v] for v in next_vs[g1]])
    g2_centroid = calc_centroid([coords[v] for v in next_vs[g2]])

    return next_vs, [g1, g2], [g1_cost, g2_cost], [g1_edges, g2_edges], [g1_centroid, g2_centroid]


def simulated_annealing_define_group(
    x0: list[list[int]],
    t0: float,
    t1: float,
    coords: list[tuple[int, int]],
    ans_edges: list[list[tuple[int, int]]],
    ans_costs: list[int],
    max_time: float,
    display: bool,
):
    x = x0
    graph = construct_dist_matrix(coords)
    costs = ans_costs
    edges = ans_edges
    centroids = []
    for vs_list in x:
        target_coords = [coords[v] for v in vs_list]
        centroid = calc_centroid(target_coords)
        centroids.append(centroid)

    current_cost = sum(costs)

    best_x = x
    best_edges = edges
    best_cost = current_cost

    start_time = time.perf_counter()  # 開始時刻を記録
    iteration = 0
    while True:
        elapsed_time = time.perf_counter() - start_time
        if elapsed_time >= max_time:
            break
        temp = exponential_schedule(t0, t1, elapsed_time, max_time)

        # 近傍解を生成
        new_x, target_groups, target_costs, target_edges, target_centroids = neighbor_function_define_group(
            x,
            costs,
            centroids,
            coords,
            graph,
        )

        # 新しいコストを計算
        new_cost = sum(target_costs)
        for g in range(len(new_x)):
            if g in target_groups:
                continue
            new_cost += costs[g]

        # 受け入れ or 拒否
        delta_cost = new_cost - current_cost
        if delta_cost < 0 or random.random() < math.exp(-delta_cost / temp):
            x = new_x
            current_cost = new_cost
            costs[target_groups[0]] = target_costs[0]
            costs[target_groups[1]] = target_costs[1]
            edges[target_groups[0]] = target_edges[0]
            edges[target_groups[1]] = target_edges[1]
            centroids[target_groups[0]] = target_centroids[0]
            centroids[target_groups[1]] = target_centroids[1]

        # 最良解の更新
        if current_cost < best_cost:
            best_x = x
            best_edges = my_deepcopy(edges)
            best_cost = current_cost

        iteration += 1

        if display and iteration % 10000 == 0:
            debug_print(f"Iteration: {iteration}, Best cost: {best_cost}, Current cost: {current_cost}")

    return best_x, best_edges


def solve(env: Env):
    ans_v, ans_edges, ans_costs = calc_greedy_answer(env, env.cneter_points)
    if env.M > 1:
        ans_v, ans_edges = simulated_annealing_define_group(
            x0=ans_v,
            t0=5,
            t1=0.1,
            coords=env.cneter_points,
            ans_edges=ans_edges,
            ans_costs=ans_costs,
            max_time=MAX_TIME,
            display=True,
        )
    ans_v, ans_edges = update_mst(
        env,
        ans_v,
        ans_edges,
        define_query_cnt(env, env.Q),
    )
    return env.answer(ans_v, ans_edges)


file_num = 0
input_file_path = f"../in/{file_num:04d}.txt"
output_file_path = f"../out/{file_num:04d}.txt"
env = EnvOffline(input_file_path, output_file_path)


ans_v, ans_edges, ans_costs = calc_greedy_answer(env, env.cneter_points)
ans_v, ans_edges = simulated_annealing_define_group(
    x0=ans_v,
    t0=5,
    t1=0.1,
    coords=env.cneter_points,
    ans_edges=ans_edges,
    ans_costs=ans_costs,
    max_time=1,
    display=True,
)
ans_v, ans_edges = update_mst(
    env,
    ans_v,
    ans_edges,
    define_query_cnt(env, env.Q),
)
print(env.answer(ans_v, ans_edges))
