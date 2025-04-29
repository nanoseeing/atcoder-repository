import sys

input = sys.stdin.buffer.readline
in_n = lambda: int(input())
in_nn = lambda: map(int, input().split())
in_s = lambda: input().rstrip().decode("utf-8")
in_map = lambda: [s == ord(".") for s in input() if s != ord("\n")]

MOD = 10**9 + 7
INF = 8 * 10**18


def main():
    T = in_s()
    U = in_s()

    u_len = len(U)
    t_len = len(T)

    for j in range(t_len - u_len + 1):
        flag = True
        for i in range(u_len):
            if not (U[i] == T[j + i] or T[j + i] == "?"):
                flag = False
                break
        if flag:
            print("Yes")
            return
    print("No")


if __name__ == "__main__":
    main()
