# =================================
# 設定
# =================================
IS_ONLINE_JUDGE = False
DEBUG = True

DEBUG_QUERY = 400
DEBUG_L = 100

INF = 10**18

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
# =================================
# 汎用
# =================================
# === 共通ファイル ===
import _pickle as cPickle
import copy

# === 共通ファイル ===
import heapq
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
def calc_dist(p1: tuple, p2: tuple) -> float:
    return math.dist(p1, p2)


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


def calc_rect_center(rectangle):
    """四角形の中心を計算する"""
    l, r, t, b = rectangle
    center = (l + r) / 2, (t + b) / 2
    return center


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


def calc_rect_half_dist(rectangle):
    """対角線の長さを計算"""
    l, r, t, b = rectangle
    dist = math.dist((l, t), (r, b))
    return dist / 2


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
            dist = calc_dist(p1, p2)
            dists.append(dist)
    return max(dists)


# =================================
# グラフアルゴリズム
# =================================
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
            edges.append((dist, v2, v1))
    return sorted(edges)


def prim(
    graph: dict[int, list[tuple[int, int]]],  # 隣接リスト（辞書対応）
):
    """プリム法"""
    v_list = list(graph.keys())
    used = {v: False for v in v_list}

    init_v = min(v_list)  # 初期点はindex最小とする
    used[init_v] = True
    que = [(cost, init_v, v) for v, cost in graph[init_v]]
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
        for nxt, cost_nxt in graph[to_v]:
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
        dist = calc_dist(p1, p2)
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
            self.L = DEBUG_L
            self.Q = DEBUG_QUERY

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


# =================================
# 以下メイン処理
# =================================
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


class EstimatePoint:
    def __init__(self, rectangle, ind):
        self.rectangle = rectangle
        self.ind = ind
        self.move_cnt = 0
        self.estimate_coordinate = calc_rect_center(rectangle)
        self._can_move_dist = calc_rect_half_dist(rectangle)

    def random_move(self, move_dist_mul: float):
        # 現在の推測点からrandom_vector方向に、最大移動可能距離x倍率分だけ移動する。四角形の外に出ないようにする。
        random_x = random.uniform(-1, 1)
        random_y = random.uniform(-1, 1)
        random_vec = (random_x, random_y)
        move_dist = self._can_move_dist * move_dist_mul

        p1 = self.estimate_coordinate
        tmp_p2 = p1[0] + random_vec[0], p1[1] + random_vec[1]
        now_dist = calc_dist(p1, tmp_p2)
        dist = 1 / now_dist * move_dist
        p2 = p1[0] + random_vec[0] * dist, p1[1] + random_vec[1] * dist

        for edge in get_rect_edge(self.rectangle):
            cross_point = clac_cross_point((p1, p2), edge)
            if cross_point:
                self.estimate_coordinate = cross_point
                break

        if isin_rectangel(p2, self.rectangle):
            self.estimate_coordinate = p2


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


# =================================
# 焼きなまし
# =================================
def calc_score(mst_infos: list[MSTInfo]):
    """コスト計算"""
    mistakes = 0
    total_num = 0
    for mst_info in mst_infos:
        mistakes += mst_info.cost()
        total_num += mst_info.edge_num
    return mistakes / total_num


def neighbor_function(estimate_points: list[EstimatePoint], now_dist_mul):
    """近傍解"""
    ret_estimate_points = my_deepcopy(estimate_points)
    N = len(ret_estimate_points)
    random_ind = random.randint(0, N - 1)
    ret_estimate_points[random_ind].random_move(now_dist_mul)
    return ret_estimate_points, random_ind


def simulated_annealing(
    x0: list[EstimatePoint],
    t0: float,
    t1: float,
    d0: float,
    d1: float,
    mst_infos: list[MSTInfo],
    max_time: float,
    display: bool,
) -> tuple[tuple[list[int], list[int]], float]:
    ESP = 0.01
    x = x0
    current_cost = calc_score(mst_infos)  # 初期解のコストを計算

    if current_cost <= ESP:
        return x
    best_x = x
    best_cost = current_cost

    start_time = time.perf_counter()  # 開始時刻を記録
    iteration = 0
    while True:
        elapsed_time = time.perf_counter() - start_time
        if elapsed_time >= max_time:
            break

        temp = exponential_schedule(t0, t1, elapsed_time, max_time)
        now_dist_mul = linear_schedule(d0, d1, elapsed_time, max_time)

        # 近傍解を生成
        new_x, randv = neighbor_function(x, now_dist_mul)

        # 一時的にMSTを計算
        for mst_info in mst_infos:
            if randv not in mst_info.vs_set:
                continue
            temp_edges = []
            for v in mst_info.vs_set:
                if v == randv:
                    continue
                coords1 = new_x[v].estimate_coordinate
                coords2 = new_x[randv].estimate_coordinate
                dist = calc_dist(coords1, coords2)
                temp_edges.append((dist, v, randv))
                temp_edges.append((dist, randv, v))
                temp_edges.sort()
            mst_info.temp_update(ex_edges=temp_edges)

        # 新しいコストを計算
        new_cost = calc_score(mst_infos)
        if new_cost <= ESP:
            debug_print(f"Iteration: {iteration}, Best cost: {new_cost}")
            return new_x

        # 受け入れ or 拒否
        delta_cost = new_cost - current_cost
        if delta_cost < 0 or random.random() < math.exp(-delta_cost / temp):
            x = new_x
            current_cost = new_cost
            for mst_info in mst_infos:
                mst_info.accept()
        else:
            for mst_info in mst_infos:
                mst_info.reject()

        # 最良解の更新
        if current_cost < best_cost:
            best_x = x
            best_cost = current_cost

        iteration += 1

        if display and iteration % 100 == 0:
            debug_print(f"Iteration: {iteration}, Best cost: {best_cost}, Current cost: {current_cost}")

    return best_x


file_num = 0
input_file_path = f"../in/{file_num:04d}.txt"
output_file_path = f"../out/{file_num:04d}.txt"
env = EnvOffline(input_file_path, output_file_path)
#####

random_vs_list = []
random_edges_list = []
for _ in range(400):
    vs = list(range(env.N))
    random_vs = random.sample(vs, k=15)
    mst_edges = env.query(random_vs)
    random_vs_list.append(random_vs)
    random_edges_list.append(mst_edges)

estimate_points: list[EstimatePoint] = []
for i, rect in enumerate(env.rectangles):
    estimate_points.append(EstimatePoint(rect, i))

mst_infos: list[MSTInfo] = []
for i in range(len(random_vs_list)):
    mst_infos.append(
        MSTInfo(
            correct_edges=random_edges_list[i],
            vs=random_vs_list[i],
            sorted_edges=construct_sorted_edges({v: env.cneter_points[v] for v in random_vs_list[i]}),
        )
    )

opt_estimate_points = simulated_annealing(
    x0=estimate_points,
    t0=(1 / (100 * 100)),
    t1=(1 / (100 * 100)) * 0.001,
    d0=1,
    d1=0.001,
    max_time=20,
    mst_infos=mst_infos,
    display=True,
)
