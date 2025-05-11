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
    N = in_n()
    A = list(in_nn())
    sum_A = sum(A)
    ans = 0
    for i in range(N):
        a = A[i]
        tmp_sum = sum_A - a
        ans += tmp_sum * a
    print(ans // 2)


if __name__ == "__main__":
    main()
