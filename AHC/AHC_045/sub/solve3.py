import heapq
import sys

import matplotlib.pyplot as plt

IS_FILE_INPUT = True
ONLINE_JUDGE = True if len(sys.argv) >= 2 and sys.argv[1] == "ONLINE_JUDGE" else False


def prim(G: dict[int, list[tuple[int, int]]]) -> list[tuple[int, int]]:
    """
    G: 隣接グラフ
    G := [ [(v_0, cost_0), (v_1,cost_1),..], [(v_2, cost_2)],...]

    返り値 ans :最小全域木の重みの総和
    """
    v_list = list(G.keys())
    ans: list[tuple[int, int]] = []
    used = {v: False for v in v_list}

    pre = v_list[0]
    used[pre] = True
    que = [(cost, v) for v, cost in G[pre]]
    heapq.heapify(que)
    while que:
        cost_v, v = heapq.heappop(que)
        if used[v]:
            continue
        used[v] = True
        ans.append((min(pre, v), max(pre, v)))
        pre = v
        for nxt, cost_nxt in G[v]:
            if used[nxt]:
                continue
            heapq.heappush(que, (cost_nxt, nxt))
    return ans


class Env:
    def __init__(self, input_file_path: str):
        with open(input_file_path) as f:
            lines = f.readlines()
            N, M, Q, L, W = map(int, lines[0].split())
            G = list(map(int, lines[1].split()))
            rectangles = []
            for l in range(2, 2 + N):
                rectangles.append(list(map(int, lines[l].split())))
            coordinates = []
            for l in range(2 + N, 2 + N + N):
                coordinates.append(list(map(int, lines[l].split())))

        self.N, self.M, self.Q, self.L, self.W = N, M, Q, L, W
        self.G = G
        self.rectangles = rectangles
        self.coordinates = coordinates

        self._query_history: list[str] = []

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

        ans = prim(now_graph)
        return ans

    def answer(
        self,
        groups: list[list[int]],
        edges: list[list[tuple[int, int]]],
        output_file_path: str,
    ):
        ans_str = "!\n"
        for i in range(len(groups)):
            ans_str += " ".join(map(str, groups[i])) + "\n"
            for e in edges[i]:
                ans_str += " ".join(map(str, e)) + "\n"

        with open(output_file_path, "w") as f:
            for query_str in self._query_history:
                f.write(query_str + "\n")
            f.write(ans_str)

    def view(self):
        "点とそれに対応する四角形を表示する"
        fig, ax = plt.subplots()
        for i in range(self.N):
            x, y = self.coordinates[i]
            a, b, c, d = self.rectangles[i]
            ax.add_patch(plt.Rectangle((a, b), c - a, d - b, fill=False))
            ax.text(x, y, str(i), fontsize=10)

    def _input(self):
        W, D, N = map(int, input().split())
        a = []
        for d in range(D):
            a.append(list(map(int, input().split())))
        return W, D, N, a


def solve(env: Env):
    N, M, Q, L, W = env.N, env.M, env.Q, env.L, env.W
    G = env.G
    rectangles = env.rectangles
    lx, rx, ly, ry = [], [], [], []
    for n in range(N):
        a, b, c, d = rectangles[n]
        lx.append(a)
        rx.append(b)
        ly.append(c)
        ry.append(d)

    # use center of rectangle
    x = [(l + r) // 2 for l, r in zip(lx, rx)]
    y = [(l + r) // 2 for l, r in zip(ly, ry)]

    # split cities into groups
    cities = list(range(N))
    cities.sort(key=lambda i: (x[i], y[i]))
    groups = []
    start_idx = 0
    for g in G:
        groups.append(cities[start_idx : start_idx + g])
        start_idx += g

    # get edges from queries
    edges = []
    for k in range(M):
        edges.append([])
        for i in range(0, G[k] - 1, 2):
            if i < G[k] - 2:
                ret = env.query(groups[k][i : i + 3])
                edges[k].extend(ret)
            else:
                edges[k].append(groups[k][i : i + 2])

    env.answer(groups, edges, "../out/0000.txt")


def main():
    env = Env("../in/0000.txt")
    env.view()
    # solve(env)


if __name__ == "__main__":
    main()
