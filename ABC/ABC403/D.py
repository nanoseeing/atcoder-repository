import sys

input = sys.stdin.buffer.readline
in_n = lambda: int(input())
in_nn = lambda: map(int, input().split())
in_s = lambda: input().rstrip().decode("utf-8")
in_map = lambda: [s == ord(".") for s in input() if s != ord("\n")]

MOD = 10**9 + 7
INF = 8 * 10**18


def main():
    N, D = in_nn()
    A = list(in_nn())

    if D == 0:
        unique_A = set(A)
        print(len(A) - len(unique_A))
        return

    a_dict = {}
    for i in range(N):
        if A[i] not in a_dict:
            a_dict[A[i]] = 0
        a_dict[A[i]] += 1

    max_A = max(A)
    ans = 0
    for d in range(D):
        pre_zero_score = 0
        pre_not_zero_score = 0
        pre_num = 0
        for x in range(max_A // D + 5):
            val = a_dict.get(d + x * D, 0)
            next_zero_score = min(pre_zero_score + val, pre_not_zero_score + val)
            next_not_zero_score = pre_zero_score
            if pre_num == 0:
                next_not_zero_score = min(next_not_zero_score, pre_not_zero_score)
            pre_num = val
            pre_zero_score = next_zero_score
            pre_not_zero_score = next_not_zero_score
        ans += min(pre_zero_score, pre_not_zero_score)
    print(ans)


if __name__ == "__main__":
    main()
