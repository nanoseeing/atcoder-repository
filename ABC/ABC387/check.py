def greedy_snake_num(L, R):
    ret = 0
    for x in range(L, R + 1):
        str_x = str(x)
        len_x = len(str_x)
        flag = True
        for i in range(1, len_x):
            if str_x[0] <= str_x[i]:
                flag = False
                break
        if flag:
            ret += 1
    return ret


def snake_num(X: int):
    return greedy_snake_num(0, X)


for i in range(100, 500):
    snake_num_ans = snake_num(i)
    greedy_snake_num_ans = greedy_snake_num(0, i)
    print(i, snake_num_ans, greedy_snake_num_ans)
    assert snake_num_ans == greedy_snake_num_ans
