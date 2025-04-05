#!/usr/bin/env python
# coding: utf-8

# =================================
# 設定
# =================================
IS_ONLINE_JUDGE = False

LIMIT_QUERY_CNT = None

DEBUG = True
DEBUG_QUERY = None
DEBUG_L = None

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

# === 共通ファイル ===
import _pickle as cPickle
import heapq

# =================================
# Import
# =================================
import math
import random
from abc import ABC, abstractmethod
from sys import stdin


# =================================
# utils
# =================================
def sort_pair(t: tuple) -> tuple:
    """ペアをソートする"""
    a, b = t
    if a > b:
        return b, a
    return (a, b)


def my_deepcopy(obj):
    return cPickle.loads(cPickle.dumps(obj, -1))


# =================================
# 幾何計算
# =================================
def calc_dist(p1: tuple, p2: tuple) -> int:
    """問題専用距離計算"""
    return int(math.dist(p1, p2))


def calc_rectangle_area(rectangle: tuple[int, int, int, int]) -> float:
    """四角形の面積を計算する"""
    l, r, t, b = rectangle
    area = (r - l) * (b - t)
    return area


def get_four_points_from_rectangle(rectangle: tuple[int, int, int, int]) -> list[tuple[int, int]]:
    """四角形の4つの頂点を取得する"""
    l, r, t, b = rectangle
    points = [
        (l, t),  # top left
        (r, t),  # top right
        (r, b),  # bottom right
        (l, b),  # bottom left
    ]
    return points


def clac_cross_point(v1, v2):
    def ccw(a, b, c):
        """3点が反時計回りかどうか判定する"""
        return (c[1] - a[1]) * (b[0] - a[0]) > (b[1] - a[1]) * (c[0] - a[0])

    p1, p2, q1, q2 = v1[0], v1[1], v2[0], v2[1]

    # 線分が交差するかを判定
    if (ccw(p1, q1, q2) != ccw(p2, q1, q2)) and (ccw(p1, p2, q1) != ccw(p1, p2, q2)):
        # 交差する場合、交点を求める
        xdiff = (p1[0] - p2[0], q1[0] - q2[0])
        ydiff = (p1[1] - p2[1], q1[1] - q2[1])

        def det(a, b):
            return a[0] * b[1] - a[1] * b[0]

        div = det(xdiff, ydiff)
        if div == 0:
            return None  # 平行な場合

        d = (det(p1, p2), det(q1, q2))
        x = det(d, xdiff) / div
        y = det(d, ydiff) / div
        return (x, y)

    return None  # 交差しない場合


def get_rect_edge(rectangle):
    """四角形の辺を取得する"""
    l, r, t, b = rectangle
    edges = [
        ((l, t), (r, t)),  # top
        ((r, t), (r, b)),  # right
        ((r, b), (l, b)),  # bottom
        ((l, b), (l, t)),  # left
    ]
    return edges


def calc_rect_center(rectangle):
    """四角形の中心を計算する"""
    l, r, t, b = rectangle
    center = (l + r) / 2, (t + b) / 2
    return center


def calc_rect_diagonal(rectangle):
    """対角線の長さを計算"""
    l, r, t, b = rectangle
    dist = math.dist((l, t), (r, b))
    return dist


def calc_centroid(points):
    """点の重心を計算する"""
    x_sum = sum(p[0] for p in points)
    y_sum = sum(p[1] for p in points)
    n = len(points)
    centroid = (x_sum / n, y_sum / n)
    return centroid


def isin_rectangel(point, rectangle):
    """点が四角形の中にあるかどうかを判定する"""
    l, r, t, b = rectangle
    x, y = point
    return l <= x <= r and t <= y <= b


def calc_rectangle_dist(rectangle1, rectangle2):
    """四角形の最大距離を計算する"""
    ps1 = get_four_points_from_rectangle(rectangle1)
    ps2 = get_four_points_from_rectangle(rectangle2)

    dists = []
    for p1 in ps1:
        for p2 in ps2:
            dist = math.dist(p1, p2)
            dists.append(dist)
    return max(dists)


# =================================
# グラフアルゴリズム
# =================================
def construct_dist_matrix_rectangle(rectangles: list[tuple[int, int]]):
    """四角形の最大距離で距離行列を構築する"""
    N = len(rectangles)
    dist_matrix = [[0.0] * N for _ in range(N)]
    for i in range(N):
        for j in range(i + 1, N):
            dist = calc_rectangle_dist(rectangles[i], rectangles[j])
            dist_matrix[i][j] = dist
            dist_matrix[j][i] = dist
    return dist_matrix


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


def prim(
    graph: dict[int, list[tuple[int, int]]],  # 隣接リスト（辞書対応）
):
    """
    graph: [{頂点: [(cost, 隣接頂点), ...], ...}]
    """
    v_list = list(graph.keys())
    used = {v: False for v in v_list}

    init_v = min(v_list)  # 初期点はindex最小とする
    used[init_v] = True
    que = [(cost, init_v, v) for cost, v in graph[init_v]]
    heapq.heapify(que)

    cost = 0.0
    ans_edges: list[tuple[int, int]] = []
    while que:
        cost_v, from_v, to_v = heapq.heappop(que)
        if used[to_v]:
            continue
        used[to_v] = True
        ans_edges.append(sort_pair((from_v, to_v)))
        cost += cost_v
        for cost_nxt, nxt in graph[to_v]:
            if used[nxt]:
                continue
            heapq.heappush(que, (cost_nxt, to_v, nxt))
    return ans_edges, cost


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


std_input = lambda: stdin.readline()[:-1]


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
    cneter_points: list[tuple[float, float]]
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

        self.cneter_points = [((l + r) / 2, (c + d) / 2) for l, r, c, d in rectangles]


class EnvOnline(Env):
    def __init__(self):
        self.read_first_line(std_input())
        self.read_second_line([std_input() for _ in range(self.N)])

        self._query_history: list[str] = []

    def query(self, c_list: list[int]) -> list[tuple[int, int]]:
        query_str = " ".join(["?", str(len(c_list)), *map(str, c_list)])
        print(query_str)
        return [tuple(map(int, std_input().split())) for _ in range(len(c_list) - 1)]

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
    def __init__(self, input_file_path: str, output_file_path: str):
        with open(input_file_path) as f:
            lines = f.readlines()
            self.read_first_line(lines[0])
            self.read_second_line(lines[1 : 2 + self.N])

            coordinates = []
            for l in range(2 + self.N, 2 + self.N * 2):
                coordinates.append(tuple(map(int, lines[l].split())))
            self.coordinates = coordinates

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
            debug_print(f"!!!query limit exceeded {len(self._query_history)}/{self.Q}")
        else:
            debug_print(f"query: {len(self._query_history)}")

        return cost


# <!-- やること
#
# - 初期解を生成する
#
#   - グループを 2 つ選ぶ
#     - グループを 1 つ選ぶ
#
# - 以下を持つオブジェクト
#   - ソート済み辺(cost, a, b)
#   - 対象頂点 set
#   - 現在のエッジ
#   - 正解のエッジ
#   - 現在の誤りエッジ数
#   - V = len(頂点 set)
#   - 辺の更新(input: ソート済み更新辺)
#     - 更新辺に含まれているエッジがあれば削除
#     - 元の辺と、更新辺を前から捜査していき、小さい順に追加する
#     - クラスカル法で現在のエッジを更新
#     - V - 正解エッジ数を算出
#
# 全辺の総数 = (env.L - 1) / env.Q
#
# - 1 点を更新する
#   - cost = 0
#   - 1 つの全域木 INFO for 全全域木 INFO
#     - その全域木内の点が更新対象でないな
#       - cost += 誤りエッジ数
#     - 更新対象である
#       - 更新点と、全域木内の点の距離を計算してエッジを作成
#       - エッジをソート
#       - 辺の更新メソッドを呼ぶ
#       - cost += 誤りエッジ数
#   - total_cost = cost / 全辺の総数 -->
#


# =================================
# 以下メイン処理
# =================================
class MyGraph:
    def __init__(self, vs, edges, group_ind):
        self.vs = vs
        self.edges = edges
        self.group_ind = group_ind

        self.visited_cnt = {v: 0 for v in vs}
        self._construct_graph()

    def _construct_graph(self):
        graph = {v: [] for v in self.vs}
        for i, e in enumerate(self.edges):
            graph[e[0]].append(e[1])
            graph[e[1]].append(e[0])
        self.graph = graph

    def update_edges(self, remove_edge, add_edges):
        self.edges = list(set(self.edges) - set(remove_edge) | set(add_edges))
        self._construct_graph()

    def choice_min_visited(self):
        min_v = min(self.visited_cnt.values())
        min_vs = [v for v, cnt in self.visited_cnt.items() if cnt == min_v]
        return random.choice(min_vs)

    def walk_bfs(self, init_v, q_num):
        visited_v = set()
        visited_edge = set()
        q = [(None, init_v)]
        while q and len(visited_v) < q_num:
            random.shuffle(q)
            pv, v = q.pop()
            if v in visited_v:
                continue
            if pv is not None:
                visited_edge.add((min(pv, v), max(pv, v)))
            visited_v.add(v)
            for nv in self.graph[v]:
                if nv not in visited_v:
                    q.append((v, nv))

        for v in visited_v:
            self.visited_cnt[v] += 1

        return visited_v, visited_edge


class MSTInfo:
    def __init__(self, correct_edges: list, vs: list, sorted_edges: list):
        self.set_correct_edges = correct_edges
        self.vs = vs
        self.vs_set = set(vs)

        self.edge_num = len(correct_edges)

        self._next_edges = None
        self._next_cost = None

        self._now_sorted_edges = sorted_edges
        self._now_cost = self._calc_cost(self._now_sorted_edges)

    def temp_update(self, ex_edges):
        self._next_edges = self._get_ex_edges(ex_edges)
        self._next_cost = self._calc_cost(self._next_edges)

    def accept(self):
        if self._next_edges is not None:
            self._now_sorted_edges = self._next_edges
            self._next_edges = None

            self._now_cost = self._next_cost
            self._next_cost = None

    def reject(self):
        self._next_cost = None
        self._next_edges = None

    def cost(self):
        if self._next_cost is not None:
            return self._next_cost
        return self._now_cost

    def _calc_cost(self, target_edges):
        mistakes = 0
        uf = UnionFind(self.vs)
        for edge in target_edges:
            _, a, b = edge
            if not uf.same(a, b):
                uf.union(a, b)
                if (a, b) not in self.set_correct_edges:
                    mistakes += 1
        return mistakes

    def _get_ex_edges(self, ex_edges):
        edge_vs = set()
        for e in ex_edges:
            edge_vs.add((e[1], e[2]))

        new_edges = []
        for e in self._now_sorted_edges:
            if (e[1], e[2]) in edge_vs:
                continue
            new_edges.append(e)

        ret_edges = []
        i, j = 0, 0
        N, M = len(new_edges), len(ex_edges)
        while i < N and j < M:
            if new_edges[i][0] < ex_edges[j][0]:
                ret_edges.append(new_edges[i])
                i += 1
            else:
                ret_edges.append(ex_edges[j])
                j += 1

        return ret_edges


def define_query_cnt(env: Env, limit_query_cnt=LIMIT_QUERY_CNT):
    group_query_cnt = [0 for _ in range(env.M)]
    group_query_v_cnt = [0 for _ in range(env.M)]

    largest_groups = []
    for g in range(env.M):
        if 3 <= env.G[g]:
            group_query_cnt[g] += 1
            group_query_v_cnt[g] += env.L
            if env.L < env.G[g]:
                largest_groups.append(g)

    for _ri in range(env.Q - sum(group_query_cnt)):
        min_cost = 10**9
        target_group = None
        for g in largest_groups:
            cost = group_query_v_cnt[g] / env.G[g]
            if cost < min_cost:
                min_cost = cost
                target_group = g
        if target_group is not None:
            group_query_v_cnt[target_group] += env.L
            group_query_cnt[target_group] += 1
            if limit_query_cnt is not None and min_cost >= limit_query_cnt:
                break
    return group_query_cnt


def update_mst(
    env: Env,
    ans_v,
    ans_edges,
    group_query_cnt: dict[int, int],
):
    for g in range(env.M):
        now_v = ans_v[g]
        now_edges = ans_edges[g]
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


def select_neighbor_rectangle_k(
    rectangles: list[list[int]],
    init_v: int,
    k: int,
    used_v: set[int],
    N: int,
):
    # 選んだ四角形の存在領域が大きくならないように選択を繰り返す O(N^2)
    used = used_v | {init_v}
    not_used = set(range(N)) - used

    ans_v = [init_v]
    now_rects = rectangles[init_v]
    while len(ans_v) < k:
        min_area = float("inf")
        min_v = None
        min_rects = None
        for v in not_used:
            nl, nr, nt, nb = now_rects
            l, r, t, b = rectangles[v]
            tl, tr, tt, tb = min(nl, l), max(nr, r), min(nt, t), max(nb, b)
            area = calc_rectangle_area((tl, tr, tt, tb))
            if area < min_area:
                min_area = area
                min_v = v
                min_rects = tl, tr, tt, tb
        if min_v is None:
            debug_print("ERROR: min_v is None")
            break
        ans_v.append(min_v)
        not_used.remove(min_v)
        used.add(min_v)
        now_rects = min_rects
    return ans_v


def calc_greedy_answer(env: Env, target_points):
    graph = construct_dist_matrix(target_points)

    groups = [(i, g) for i, g in enumerate(env.G)]
    groups = sorted(groups, key=lambda x: x[1], reverse=True)

    ans_edges = [None for _ in range(len(groups))]
    ans_v = [None for _ in range(len(groups))]

    now_used: set = set()
    not_used: set = set(range(env.N))
    for group_n, group_size in groups:
        v = not_used.pop()
        prim_edges, prim_v, _ = prim_k(graph, v, group_size, now_used, env.N)
        ans_edges[group_n] = prim_edges
        ans_v[group_n] = prim_v
        now_used.update(prim_v)
        not_used.difference_update(prim_v)

    return ans_v, ans_edges


def _select_min_area(rectangles, not_used):
    min_area = float("inf")
    min_v = None
    for v in not_used:
        area = calc_rectangle_area(rectangles[v])
        if area < min_area:
            min_area = area
            min_v = v
    return min_v


def calc_greedy_answer_from_rectangle(env: Env):
    groups = [(i, g) for i, g in enumerate(env.G)]
    groups = sorted(groups, key=lambda x: x[1], reverse=True)

    ans_edges = [None for _ in range(len(groups))]
    ans_v = [None for _ in range(len(groups))]

    now_used: set = set()
    not_used: set = set(range(env.N))
    for group_n, group_size in groups:
        v = _select_min_area(env.rectangles, not_used)
        prim_v = select_neighbor_rectangle_k(env.rectangles, v, group_size, now_used, env.N)
        graph = construct_graph({v: env.cneter_points[v] for v in prim_v})
        prim_edges, _ = prim(graph)

        ans_edges[group_n] = prim_edges
        ans_v[group_n] = prim_v
        now_used.update(prim_v)
        not_used.difference_update(prim_v)

    return ans_v, ans_edges


def solve(env: Env):
    # ans_v, ans_edges = calc_greedy_answer(env, env.cneter_points)
    # ans_v, ans_edges = update_mst(env, ans_v, ans_edges, define_query_cnt(env))
    # return env.answer(ans_v, ans_edges)
    anv_v_estimate, ans_edges_estimate = calc_greedy_answer_from_rectangle(env)
    ans_v_update, ans_edges_update = update_mst(
        env,
        my_deepcopy(anv_v_estimate),
        my_deepcopy(ans_edges_estimate),
        define_query_cnt(env),
    )
    return env.answer(ans_v_update, ans_edges_update)


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
        with open("../result/dense_test02.txt", "w") as f:
            f.write(f"avg: {avg}\n")
            for pi, cost in enumerate(costs):
                f.write(f"{pi:0<4} {cost}\n")


if __name__ == "__main__":
    main()
