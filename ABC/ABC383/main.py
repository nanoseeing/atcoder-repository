import sys

input = sys.stdin.buffer.readline
in_n = lambda: int(input())
in_nn = lambda: map(int, input().split())
in_s = lambda: input().rstrip().decode("utf-8")
in_map = lambda: [s == ord(".") for s in input() if s != ord("\n")]

MOD = 10**9 + 7
INF = 8 * 10**18


def f(N):
    n = N
    i = 1
    while n > 0:
        x = 9 * 10 ** ((i - 1) // 2)
        if n > x:
            n -= x
            i += 1
        else:
            break
    keta = i
    amari = N
    return keta, amari


def main():
    N = in_n()
    keta, amari = f(N)
    print(keta, amari)


if __name__ == "__main__":
    main()
