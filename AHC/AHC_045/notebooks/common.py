# === 共通ファイル ===
import _pickle as cPickle
import heapq
import math


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
    """問題専用"""
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
        dist = math.dist(p1, p2)
        distances.append(dist)
    avg = sum(distances) / len(distances)
    return avg
