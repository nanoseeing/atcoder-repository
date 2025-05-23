# ================
# Main
# ================

# import itertools
# import heapq
# import bisect
# from collections import Counter
# from collections import defaultdict
# from collections import deque

# sys.setrecursionlimit(10**7)

MOD = 10**9 + 7
INF = 9 * 10**18


def main():
    H, W, N = in_nn()
    X = []
    Y = []
    for i in range(N):
        x, y = in_nn()
        X.append(x)
        Y.append(y)

    Xsort = sorted(set(X))
    Ysort = sorted(set(Y))

    XN = len(Xsort)
    YN = len(Ysort)

    x_id2num = {}
    x_num2id = {}
    y_id2num = {}
    y_num2id = {}
    for i in range(XN):
        x_id2num[i] = Xsort[i]
        x_num2id[Xsort[i]] = i
    for i in range(YN):
        y_id2num[i] = Ysort[i]
        y_num2id[Ysort[i]] = i

    new_X = []
    new_Y = []
    for i in range(N):
        new_X.append(x_num2id[X[i]])
        new_Y.append(y_num2id[Y[i]])

    # debug(new_X)
    # debug(new_Y)
    # debug(x_id2num)
    # debug(y_id2num)

    Xhave = [set() for _ in range(XN)]
    Yhave = [set() for _ in range(YN)]
    for i in range(N):
        x = new_X[i]
        y = new_Y[i]
        Xhave[x].add(y)
        Yhave[y].add(x)

    # debug(Xhave)
    # debug(Yhave)

    is_x_query = [False] * XN
    is_y_query = [False] * YN

    ans = []
    Q = in_n()
    for q in range(Q):
        qtype, num = in_nn()
        if qtype == 1:
            x = num
            if x not in x_num2id:
                ans.append(0)
            else:
                new_X = x_num2id[x]
                if is_x_query[new_X]:
                    ans.append(0)
                else:
                    ans.append(len(Xhave[new_X]))
                    for y in Xhave[new_X]:
                        Yhave[y].discard(new_X)
                    is_x_query[new_X] = True
        else:
            y = num
            if y not in y_num2id:
                ans.append(0)
            else:
                new_Y = y_num2id[y]
                if is_y_query[new_Y]:
                    ans.append(0)
                else:
                    ans.append(len(Yhave[new_Y]))
                    for x in Yhave[new_Y]:
                        Xhave[x].discard(new_Y)
                    is_y_query[new_Y] = True

    print(*ans, sep="\n")


# ================
# Template
# ================
import sys

input = sys.stdin.buffer.readline
in_n = lambda: int(input())
in_nn = lambda: map(int, input().split())
in_s = lambda: input().rstrip().decode("utf-8")
in_map = lambda: [s == ord(".") for s in input() if s != ord("\n")]


if __name__ == "__main__":
    main()
