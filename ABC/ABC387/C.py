def snake_num(X: int):
    # dp[keta][max_digit][is_limit][is_zero]
    str_x = str(X)
    keta = len(str_x)
    dp = [[[[0] * 2 for _ in range(2)] for _ in range(10)] for _ in range(keta)]

    init_num = int(str_x[0])
    dp[0][init_num][1][0] = 1
    for i in range(1, init_num):
        dp[0][i][0][0] = 1
    dp[0][0][0][1] = 1

    for k in range(1, keta):
        # non limit -> non limit
        for d1 in range(10):
            dp[k][d1][0][0] += dp[k - 1][d1][0][0] * d1
        for d2 in range(1, 10):
            dp[k][d2][0][0] += dp[k - 1][0][0][1]
        dp[k][0][0][1] += dp[k - 1][0][0][1]

        # limit -> limit
        now_x = int(str_x[k])
        if now_x < init_num:
            dp[k][init_num][1][0] += dp[k - 1][init_num][1][0]

        # limit -> non limit
        dp[k][init_num][0][0] += dp[k - 1][init_num][1][0] * min(now_x, init_num)

    ret = 0
    for d1 in range(10):
        for is_limit in range(2):
            for is_zero in range(2):
                ret += dp[keta - 1][d1][is_limit][is_zero]
    return ret


L, R = map(int, input().split())
ans = snake_num(R) - snake_num(L - 1)
print(ans)
