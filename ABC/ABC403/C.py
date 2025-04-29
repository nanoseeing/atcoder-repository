import sys

input = sys.stdin.buffer.readline
in_n = lambda: int(input())
in_nn = lambda: map(int, input().split())
in_s = lambda: input().rstrip().decode("utf-8")
in_map = lambda: [s == ord(".") for s in input() if s != ord("\n")]

MOD = 10**9 + 7
INF = 8 * 10**18


def main():
    N, M, Q = in_nn()
    ok_set = [set() for _ in range(N)]
    all_ok = [False for _ in range(N)]

    ans = []
    for _ in range(Q):
        query = in_s()
        if query[0] == "1":
            _, i, j = map(int, query.split())
            ok_set[i - 1].add(j - 1)
        elif query[0] == "2":
            _, i = map(int, query.split())
            all_ok[i - 1] = True
        else:
            _, x, y = map(int, query.split())
            x -= 1
            y -= 1
            if all_ok[x] or (y in ok_set[x]):
                ans.append("Yes")
            else:
                ans.append("No")

    print("\n".join(ans))


if __name__ == "__main__":
    main()
