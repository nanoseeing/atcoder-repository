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

MOD = 998244353
INF = 9 * 10**18


class CombMod:
    def __init__(self, n, mod):
        n = n + 1
        inv = [0] * n
        fact = [0] * n
        fact_inv = [0] * n

        inv[0] = 0
        inv[1] = 1
        for n in range(2, n):
            q, r = divmod(mod, n)
            inv[n] = inv[r] * (-q) % mod

        fact[0] = 1
        for n in range(1, n):
            fact[n] = n * fact[n - 1] % mod

        fact_inv[0] = 1
        for n in range(1, n):
            fact_inv[n] = fact_inv[n - 1] * inv[n] % mod

        self.fact = fact
        self.fact_inv = fact_inv
        self.mod = mod

    def comb(self, n, r):
        if n < r or n < 0 or r < 0:
            return 0
        return (self.fact[n] * self.fact_inv[r] * self.fact_inv[n - r]) % self.mod

    def perm(self, n, r):
        return (self.fact[n] * self.fact_inv[n - r]) % self.mod


def main():
    A, B, C, D = in_nn()
    N = A + B + C + D

    comb = CombMod(10**7, MOD)
    ans = 0
    for i in range(A, min(A + B, N - C) + 1):
        ans += comb.comb(i - 1, A - 1) * comb.comb(N - i, C) % MOD
        ans %= MOD
    print(ans)


if __name__ == "__main__":
    main()
