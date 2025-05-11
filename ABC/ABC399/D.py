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
    code = 36  # Cyan
    text = " ".join(map(str, args))
    sys.stderr.write(f"\033[{code}m{text}\033[0m\n")


# ================
# Main
# ================

MOD = 10**9 + 7
INF = 9 * 10**18


def solve(N, A):
    inds = defaultdict(list)
    for i in range(2 * N - 1):
        a, b = A[i], A[i + 1]
        inds[(a, b)].append(i)

    ans = 0
    for i in range(2 * N - 1):
        a, b = A[i], A[i + 1]
        if a == b:
            continue

        now_inds = inds[(a, b)]
        # debug(now_inds)
        if len(now_inds) == 2:
            other_inds = set(now_inds) - set([i])
            other_ind = list(other_inds)[0]
            if not (i - 1 <= other_ind <= i + 1):
                ans += 1

        now_inds = inds[(b, a)]
        # debug(now_inds)
        if len(now_inds) == 1:
            other_inds = set(now_inds) - set([i])
            other_ind = list(other_inds)[0]
            if not (i - 2 <= other_ind <= i + 2):
                ans += 1

    return ans // 2


def main():
    T = in_n()
    ans = []
    for _ in range(T):
        N = in_n()
        A = list(in_nn())
        ans.append(solve(N, A))

    print(*ans, sep="\n")


if __name__ == "__main__":
    main()
