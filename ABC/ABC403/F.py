import sys
from collections import Counter
from itertools import product

input = sys.stdin.buffer.readline
in_n = lambda: int(input())
in_nn = lambda: map(int, input().split())
in_s = lambda: input().rstrip().decode("utf-8")
in_map = lambda: [s == ord(".") for s in input() if s != ord("\n")]

MOD = 10**9 + 7
INF = 8 * 10**18

max_N = 10**7  # 最大値
spf = list(range(max_N + 1))
for i in range(2, int(max_N**0.5) + 1):
    if spf[i] == i:  # iが素数なら
        for j in range(i * i, max_N + 1, i):
            if spf[j] == j:
                spf[j] = i


def factorize_with_counts(x):
    cnt = Counter()
    while x != 1:
        cnt[spf[x]] += 1
        x //= spf[x]
    return list(cnt.items())


def divisors(x):
    factors = factorize_with_counts(x)  # [(素因数, 個数), ...]

    # 各素因数について、[p^0, p^1, ..., p^k] を作る
    exp_lists = []
    for prime, count in factors:
        exp_lists.append([prime**e for e in range(count + 1)])

    divs = []
    for combo in product(*exp_lists):
        mul = 1
        for v in combo:
            mul *= v
        divs.append(mul)

    return divs


def main():
    N = in_n()

    dp_S = [INF] * (N + 1)
    dp_M = [INF] * (N + 1)

    str_S = [""] * (N + 1)
    str_M = [""] * (N + 1)

    for next_n in range(1, N + 1):
        s = list(str(next_n))
        flag = True
        for j in range(len(s)):
            if s[j] != "1":
                flag = False
                break
        if flag:
            if dp_S[next_n] > len(s):
                dp_S[next_n] = len(s)
                str_S[next_n] = "1" * len(s)
            if dp_M[next_n] > len(s):
                dp_M[next_n] = len(s)
                str_M[next_n] = "1" * len(s)

        # 足し算する
        for x1 in range(1, next_n):
            x2 = next_n - x1
            if dp_M[next_n] > dp_M[x1] + dp_M[x2] + 3:
                dp_M[next_n] = dp_M[x1] + dp_M[x2] + 3
                str_M[next_n] = "(" + str_M[x1] + "+" + str_M[x2] + ")"
            if dp_M[next_n] > dp_M[x1] + dp_S[x2] + 3:
                dp_M[next_n] = dp_M[x1] + dp_S[x2] + 3
                str_M[next_n] = "(" + str_M[x1] + "+" + str_S[x2] + ")"
            if dp_M[next_n] > dp_S[x1] + dp_M[x2] + 3:
                dp_M[next_n] = dp_S[x1] + dp_M[x2] + 3
                str_M[next_n] = "(" + str_S[x1] + "+" + str_M[x2] + ")"

            if dp_S[next_n] > dp_M[x1] + dp_M[x2] + 1:
                dp_S[next_n] = dp_M[x1] + dp_M[x2] + 1
                str_S[next_n] = str_M[x1] + "+" + str_M[x2]
            if dp_S[next_n] > dp_M[x1] + dp_S[x2] + 1:
                dp_S[next_n] = dp_M[x1] + dp_S[x2] + 1
                str_S[next_n] = str_M[x1] + "+" + str_S[x2]
            if dp_S[next_n] > dp_S[x1] + dp_M[x2] + 1:
                dp_S[next_n] = dp_S[x1] + dp_M[x2] + 1
                str_S[next_n] = str_S[x1] + "+" + str_M[x2]
            if dp_S[next_n] > dp_S[x1] + dp_S[x2] + 1:
                dp_S[next_n] = dp_S[x1] + dp_S[x2] + 1
                str_S[next_n] = str_S[x1] + "+" + str_S[x2]

        vals = divisors(next_n)
        for val1 in vals:
            val2 = next_n // val1
            if dp_M[next_n] > dp_M[val1] + dp_M[val2] + 1:
                dp_M[next_n] = dp_M[val1] + dp_M[val2] + 1
                str_M[next_n] = str_M[val1] + "*" + str_M[val2]
            if dp_S[next_n] > dp_M[val1] + dp_M[val2] + 1:
                dp_S[next_n] = dp_M[val1] + dp_M[val2] + 1
                str_S[next_n] = str_M[val1] + "*" + str_M[val2]

    if dp_M[N] < dp_S[N]:
        print(str_M[N])
    else:
        print(str_S[N])


if __name__ == "__main__":
    main()
