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


def debug(*args):
    import ast
    import inspect
    import textwrap

    CYAN = "\033[96m"
    RESET = "\033[0m"

    frame = inspect.currentframe().f_back
    code_line = inspect.getframeinfo(frame).code_context[0]
    code_line = textwrap.dedent(code_line).strip()

    try:
        root = ast.parse(code_line)
    except SyntaxError as e:
        print(f"{CYAN}SyntaxError while parsing code line: {e}{RESET}", file=sys.stderr)
        return

    call_node = root.body[0].value
    if not isinstance(call_node, ast.Call):
        print(f"{CYAN}debug_print() not found in the code line.{RESET}", file=sys.stderr)
        return

    arg_names = []
    for arg in call_node.args:
        arg_str = ast.get_source_segment(code_line, arg)
        arg_names.append(arg_str)

    output = []
    for name, val in zip(arg_names, args):
        output.append(f"{name} = {val!r}")

    print(f"{CYAN}" + ", ".join(output) + f"{RESET}", file=sys.stderr)


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

    A, B, C, D = in_nn()
    A -= 1
    B -= 1
    C -= 1
    D -= 1

    dist = create_array((H, W), INF)
    dist[A][B] = 0
    que = deque()
    que.append((A, B))

    directions = [(1, 0), (-1, 0), (0, 1), (0, -1)]
    kecks = [(1, 0), (2, 0), (0, 1), (0, 2), (-1, 0), (-2, 0), (0, -1), (0, -2)]
    while que:
        h, w = que.popleft()
        for dh, dw in directions:
            nh, nw = h + dh, w + dw
            if 0 <= nh < H and 0 <= nw < W and grid[nh][nw] == "." and dist[nh][nw] > dist[h][w]:
                dist[nh][nw] = dist[h][w]
                que.appendleft((nh, nw))
        for dh, dw in kecks:
            nh, nw = h + dh, w + dw
            if 0 <= nh < H and 0 <= nw < W and grid[nh][nw] == "#" and dist[nh][nw] > dist[h][w] + 1:
                dist[nh][nw] = dist[h][w] + 1
                que.append((nh, nw))

    for h in range(H):
        debug(dist[h])
    print(dist[C][D] if dist[C][D] != INF else -1)


if __name__ == "__main__":
    main()
