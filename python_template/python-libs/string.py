""" 文字列関連 """


# 最長共通部分列
def LCS(S, T, f=False):

    L1 = len(S)
    L2 = len(T)
    dp = [[0] * (L2 + 1) for i in range(L1 + 1)]

    for i in range(L1 - 1, -1, -1):
        for j in range(L2 - 1, -1, -1):
            r = max(dp[i + 1][j], dp[i][j + 1])
            if S[i] == T[j]:
                r = max(r, dp[i + 1][j + 1] + 1)
            dp[i][j] = r

    if not f:
        return dp[0][0]

    res = []
    i = 0
    j = 0
    while i < L1 and j < L2:
        if S[i] == T[j]:
            res.append(S[i])
            i += 1
            j += 1
        elif dp[i][j] == dp[i + 1][j]:
            i += 1
        elif dp[i][j] == dp[i][j + 1]:
            j += 1

    return (dp[0][0], ''.join(res))


# z-algorithm

def z_algo(S):
    N = len(S)
    A = [0] * N
    i = 1
    j = 0
    A[0] = l = len(S)
    while i < l:
        while i + j < l and S[j] == S[i + j]:
            j += 1
        if not j:
            i += 1
            continue
        A[i] = j
        k = 1
        while l - i > k < j - A[k]:
            A[i + k] = A[k]
            k += 1
        i += k
        j -= k
    return A
