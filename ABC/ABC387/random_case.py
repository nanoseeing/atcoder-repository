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


L, R = map(int, input().split())
ret = greedy_snake_num(L, R)
print(ret)
