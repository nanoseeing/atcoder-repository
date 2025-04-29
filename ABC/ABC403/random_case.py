import sys
from collections import deque
from dataclasses import dataclass

input = sys.stdin.buffer.readline
in_n = lambda: int(input())
in_nn = lambda: map(int, input().split())
in_s = lambda: input().rstrip().decode("utf-8")
in_map = lambda: [s == ord(".") for s in input() if s != ord("\n")]

MOD = 10**9 + 7
INF = 8 * 10**18


def generate_random_case(seed):
    import random

    random.seed(seed)

    Q = 500
    queries = []
    for _ in range(Q):
        t = random.randint(1, 2)
        s = "".join(random.choices("abcdefghijklmnopqrstuvwxyz", k=random.randint(1, 7)))
        queries.append(f"{t} {s}")

    correct_ans = []
    X = []
    Y = []
    for i in range(Q):
        t, s = queries[i].split()
        t = int(t)
        if t == 1:
            X.append(s)
        else:
            Y.append(s)
        turn_ans = 0
        for ys in Y:
            flag = True
            for xs in X:
                if len(xs) <= len(ys) and ys[: len(xs)] == xs:
                    flag = False
                    break
            if flag:
                turn_ans += 1
        correct_ans.append(turn_ans)

    print(Q)
    print("\n".join(queries))
    print("\n".join(map(str, correct_ans)), file=sys.stderr)


import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--seed", type=int, default=0)
seed = parser.parse_args().seed
generate_random_case(seed)
