""" その他ライブラリ """


# 2次元累積和
total_sum = [[0] * (W + 1) for _ in range(H + 1)]

for y in range(H):
    for x in range(W):
        total_sum[y + 1][x + 1] = C[y][x]

for y in range(H + 1):
    for x in range(W):
        total_sum[y][x + 1] += total_sum[y][x]

for x in range(W + 1):
    for y in range(H):
        total_sum[y + 1][x] += total_sum[y][x]


def calc_area(u, d, l, r):
    return total_sum[d][r] - total_sum[u][r] - total_sum[d][l] + total_sum[u][l]
