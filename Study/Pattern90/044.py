# ================
# Template
# ================

import pickle
import sys

# import itertools
# import heapq
# import bisect
# from collections import deque, defaultdict, Counter
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


def debug_print(obj, color="red"):
    code = 36  # Cyan
    text = str(obj)
    sys.stderr.write(f"\033[{code}m{text}\033[0m\n")


# ================
# Main
# ================

MOD = 10**9 + 7
INF = 9 * 10**18


def main():
    N, Q = in_nn()
    A = list(in_nn())

    shift = 0
    ans = []
    for i in range(Q):
        t, x, y = in_nn()
        x -= 1
        y -= 1
        # tmp_A = [A[(i - shift) % N] for i in range(N)]
        # debug_print(tmp_A)
        if t == 1:
            new_x = (x - shift) % N
            new_y = (y - shift) % N
            A[new_x], A[new_y] = A[new_y], A[new_x]
        elif t == 2:
            shift += 1
            shift %= N
        else:
            ans.append(A[(x - shift) % N])
    print(*ans, sep="\n")


if __name__ == "__main__":
    main()
