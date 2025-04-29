import numpy as np

MOD = 10**9 + 7


def eig_diag(x):
    eig = np.linalg.eig(x)
    e = np.diag(eig[0])
    p = eig[1]
    return e, p


def mat_pow(a, n):
    e = eig_diag(a)

    d = e[0]
    p = e[1]
    ip = np.linalg.inv(p)
    a_n = p @ d**n @ ip
    return a_n


def main():
    N, B, K = map(int, input().split())
    c = list(map(int, input().split()))

    vec = np.zeros((B, B), dtype=int)
    for b in range(B):
        for digit in c:
            x = (b * 10 + digit) % B
            vec[x][b] += 1

    dp = np.zeros((B, 1), dtype=int)
    for digit in c:
        dp[digit % B][0] += 1

    x = mat_pow(vec, N - 1)
    ans = x @ dp

    print(int(round(ans[0][0])))


if __name__ == "__main__":
    main()
