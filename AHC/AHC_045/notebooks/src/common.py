import heapq
import math
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
    query_history: list[tuple[int, int]]

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
        self.query_history: list[tuple[int, int]] = []

    def query(self, c_list: list[int]):
        query_str = " ".join(["?", str(len(c_list)), *map(str, c_list)])
        print(query_str)

        ret = [tuple(map(int, input().split())) for _ in range(len(c_list) - 1)]
        self.query_history.append(ret)
        return ret

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
        self.query_history: list[tuple[int, int]] = []

        if DEBUG:
            self.L = DEBUG_L if DEBUG_L is not None else self.L
            self.Q = DEBUG_QUERY if DEBUG_QUERY is not None else self.Q

    def query(self, c_list: list[int]) -> list[tuple[int, int]]:
        # query_str = " ".join(["?", str(len(c_list)), *map(str, c_list)])
        ans_edges, _ = kruskals_algorithm(construct_sorted_edges({v: self.coordinates[v] for v in c_list}), c_list)
        self.query_history.append(ans_edges)
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

        # for query_str in self._query_history:
        #     ans_str += query_str + "\n"

        # with open(self.output_file_path, "w") as f:
        #     f.write(ans_str)

        # if len(self._query_history) > self.Q:
        #     print(f"!!!query limit exceeded {len(self._query_history)}/{self.Q}")
        # else:
        #     print(f"query: {len(self._query_history)}")

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
def divide_interval(l: float, r: float, n: int, is_exclude_edge: bool = False) -> list[float]:
    """区間[l, r]をn等分する"""

    if is_exclude_edge:
        step = (r - l) / (n + 1)
        return [l + (i + 1) * step for i in range(n)]
    else:
        step = (r - l) / (n - 1)
        return [l + i * step for i in range(n)]


def is_overlap(l1, r1, l2, r2):
    """線分の重なり判定"""
    return not (l1 > r2 or l2 > r1)


def calc_dist(p1: tuple, p2: tuple) -> int:
    """問題専用距離計算"""
    return int(math.dist(p1, p2))


def is_overlap_rectangle(r1: list[int], r2: list[int]) -> bool:
    """2つの四角形が重なっているかどうかを判定する"""
    l1, r1, t1, b1 = r1
    l2, r2, t2, b2 = r2
    return is_overlap(l1, r1, l2, r2) and is_overlap(t1, b1, t2, b2)


def calc_rectangle_area(rectangle: tuple[int, int, int, int]) -> float:
    """四角形の面積を計算する"""
    l, r, t, b = rectangle
    area = (r - l) * (b - t)
    return area


def get_four_points_from_rectangle(rectangle: tuple[int, int, int, int]) -> list[tuple[int, int]]:
    """四角形の4つの頂点を取得する"""
    l, r, t, b = rectangle
    points = [(l, t), (r, t), (r, b), (l, b)]
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


def calc_rectangle_max_dist(rectangle1, rectangle2):
    l1, r1, t1, b1 = rectangle1
    l2, r2, t2, b2 = rectangle2

    candidates = [
        ((l1, t1), (r2, b2)),
        ((r1, t1), (l2, b2)),
        ((l1, b1), (r2, t2)),
        ((r1, b1), (l2, t2)),
    ]
    return max(math.dist(p1, p2) for p1, p2 in candidates)


def calc_rectangle_min_dist(rectangle1, rectangle2):
    if is_overlap_rectangle(rectangle1, rectangle2):
        return 0.0
    l1, r1, t1, b1 = rectangle1
    l2, r2, t2, b2 = rectangle2
    min_width = max(0, max(l2 - r1, l1 - r2))
    min_height = max(0, max(t2 - b1, t1 - b2))
    return math.hypot(min_width, min_height)


def calc_rectangle_dist_min_max(rectangle1, rectangle2):
    """四角形の距離を計算する"""
    min_dist = calc_rectangle_min_dist(rectangle1, rectangle2)
    max_dist = calc_rectangle_max_dist(rectangle1, rectangle2)
    return min_dist, max_dist


# =================================
# グラフアルゴリズム
# =================================
def scc(
    graph: dict[int, list[int]],  # 隣接リスト
    r_graph: dict[int, list[int]],  # 逆グラフ
    vs: list[int],  # 頂点のリスト
):
    """強連結成分分解"""
    order = []
    used = {v: 0 for v in vs}
    group = {v: None for v in vs}

    def dfs(s):
        used[s] = 1
        for t in graph[s]:
            if not used[t]:
                dfs(t)
        order.append(s)

    def rdfs(s, col):
        group[s] = col
        used[s] = 1
        for t in r_graph[s]:
            if not used[t]:
                rdfs(t, col)

    for i in vs:
        if not used[i]:
            dfs(i)

    used = {v: 0 for v in vs}
    label_num = 0
    for s in reversed(order):
        if not used[s]:
            rdfs(s, label_num)
            label_num += 1
    return label_num, group


def scc_construct(
    scc_graph: dict[int, list[int]],  # 隣接リスト
    label_num: int,
    group: dict[int, int],  # 頂点が属するグループ
    vs: list[int],  # 頂点のリスト
):
    """強連結成分の隣接リストを構築する"""
    ssc_graph: list[set] = [set() for _ in range(label_num)]
    belong_lists: list[list[int]] = [[] for _ in range(label_num)]
    for v in vs:
        lbs = group[v]
        for w in scc_graph[v]:
            lbt = group[w]
            if lbs == lbt:
                continue
            ssc_graph[lbs].add(lbt)
        belong_lists[lbs].append(v)
    return ssc_graph, belong_lists


def priority_topological_sort(
    graph: list[list[int]],  # 隣接リスト
    initial_priority: dict[int, int],  # 初期の優先度リスト{node: rank}
) -> list[int]:
    """優先度付きトポロジカルソート"""
    N = len(initial_priority)
    in_degree = [0] * N

    # グラフと入次数を作成
    for u in range(N):
        for v in graph[u]:
            in_degree[v] += 1

    heap: list[tuple[int, int]] = []
    for node in range(N):
        if in_degree[node] == 0:
            heapq.heappush(heap, (initial_priority[node], node))

    result = []
    while heap:
        _, u = heapq.heappop(heap)
        result.append(u)

        for v in graph[u]:
            in_degree[v] -= 1
            if in_degree[v] == 0:
                heapq.heappush(heap, (initial_priority[v], v))

    if len(result) != N:
        raise ValueError("Graph is not a DAG or has cycles")

    return result


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


def construct_dist_matrix_rectangle(rectangles: list[tuple[int, int]]):
    """四角形の最大距離で距離行列を構築する"""
    N = len(rectangles)
    dist_matrix = [[0] * N for _ in range(N)]
    for i in range(N):
        for j in range(i + 1, N):
            dist = calc_rectangle_max_dist(rectangles[i], rectangles[j])
            dist_matrix[i][j] = dist
            dist_matrix[j][i] = dist
    return dist_matrix


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


def construct_graph_from_list(points: list[tuple[int, int]]):
    """点の隣接リストを構築する"""

    N = len(points)
    graph: list[list[tuple[int, int]]] = [[] for _ in range(N)]
    for v1 in range(N):
        for v2 in range(N):
            if v1 == v2:
                continue
            dist = calc_dist(points[v1], points[v2])
            graph[v1].append((dist, v2))
    for v in range(N):
        graph[v].sort()

    return graph


def construct_dag_from_edges(
    edges: list[tuple[int, int]],  # 辺のリスト
    vs: list[int],  # 頂点のリスト
):
    """隣接リストを構築する"""
    graph: dict[int, list[int]] = {v: [] for v in vs}
    for e in edges:
        v1, v2 = e
        graph[v1].append((v2))
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


def construct_dag_and_rdag_from_edges(
    edges: list[tuple[int, int]],  # 辺のリスト
    vs: list[int],  # 頂点のリスト
):
    """隣接リストを構築する"""
    graph: dict[int, list[int]] = {v: [] for v in vs}
    graph_r: dict[int, list[int]] = {v: [] for v in vs}
    for e in edges:
        v1, v2 = e
        graph[v1].append((v2))
        graph_r[v2].append((v1))
    return graph, graph_r


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


def prim_vs(
    graph: list[list[float]],  # 全体の隣接行列
    vs: list[int],  # 頂点のリスト
):
    init_v = vs[0]

    used = {init_v}
    que = [(graph[init_v][v], init_v, v) for v in vs]
    heapq.heapify(que)

    ans_edges: list[tuple[int, int]] = []
    ans_cost = 0.0
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
    return avg
    return avg
    return avg
    return avg
    return avg
    return avg
    return avg
    return avg
    return avg
    return avg
