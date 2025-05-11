# ================
# Template
# ================

import pickle
import sys

# import itertools
# import heapq
# import bisect
from collections import Counter, defaultdict, deque

# from dataclasses import dataclass

# sys.setrecursionlimit(10**7)

input = sys.stdin.buffer.readline
in_n = lambda: int(input())
in_nn = lambda: map(int, input().split())
in_s = lambda: input().rstrip().decode("utf-8")
in_map = lambda: [s == ord(".") for s in input() if s != ord("\n")]


def fast_copy(obj):
    return pickle.loads(pickle.dumps(obj, -1))


def create_array(shape: tuple, fill_value):
    if len(shape) == 1:
        return [fill_value() if callable(fill_value) else fill_value for _ in range(shape[0])]
    else:
        return [create_array(shape[1:], fill_value) for _ in range(shape[0])]


def debug(obj):
    code = 36  # Cyan
    text = str(obj)
    sys.stderr.write(f"\033[{code}m{text}\033[0m\n")


# ================
# Main
# ================

MOD = 10**9 + 7
INF = 9 * 10**18


def main():
    H, W = in_nn()
    grid = []
    for _ in range(H):
        s = list(in_s())
        grid.append(s)

    is_e = []
    for h in range(H):
        for w in range(W):
            if grid[h][w] == "E":
                is_e.append((h, w))

    dist = [[INF] * W for _ in range(H)]
    q = deque(is_e)
    for h, w in is_e:
        dist[h][w] = 0

    while q:
        h, w = q.popleft()
        for dh, dw in ((-1, 0), (1, 0), (0, -1), (0, 1)):
            nh, nw = h + dh, w + dw
            if 0 <= nh < H and 0 <= nw < W:
                if grid[nh][nw] != ".":
                    continue
                if dist[nh][nw] > dist[h][w] + 1:
                    dist[nh][nw] = dist[h][w] + 1
                    q.append((nh, nw))

    directions = ((-1, 0), (1, 0), (0, -1), (0, 1))
    directions_str = ("^", "v", "<", ">")
    ans = fast_copy(grid)
    for h in range(H):
        for w in range(W):
            for ds, (dh, dw) in zip(directions_str, directions):
                nh, nw = h + dh, w + dw
                if 0 <= nh < H and 0 <= nw < W:
                    if dist[h][w] == dist[nh][nw] + 1:
                        ans[h][w] = ds
                        break

    for h in range(H):
        print("".join(ans[h]))


if __name__ == "__main__":
    main()
