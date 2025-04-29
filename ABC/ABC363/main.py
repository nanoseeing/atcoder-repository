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
    amari = n
    return keta, amari - 1


def main():
    N = in_n()
    if N == 1:
        print(0)
        return

    N -= 1
    keta, amari = f(N)
    s = str(amari)
    s_rev = s[::-1]

    if keta == 1:
        print(amari + 1)
        return

    ans_str = ""
    if keta % 2 == 0:
        ans_str = s_rev[0] + s_rev[0]
    else:
        ans_str = s_rev[0]

    s_rev = s_rev[1:]
    for x in s_rev:
        if len(ans_str) + 2 == keta:
            nes_w = str(int(x) + 1)
            ans_str = nes_w + ans_str + nes_w
        else:
            ans_str = x + ans_str + x

    for _ in range(1000):
        if len(ans_str) == keta:
            break
        elif len(ans_str) + 2 == keta:
            ans_str = "1" + ans_str + "1"
        else:
            ans_str = "0" + ans_str + "0"

    print(ans_str)


if __name__ == "__main__":
    main()
