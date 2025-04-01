import bisect

# (広義)最長増加部分列
LIS = [A[0]]
for i in range(1, N):

    if A[i] > LIS[-1]:
        LIS.append(A[i])
    else:
        LIS[bisect.bisect_left(LIS, A[i])] = A[i]

# (狭義)最長増加部分列
LIS = [A[0]]
for i in range(1, N):

    if A[i] >= LIS[-1]:
        LIS.append(A[i])
    else:
        LIS[bisect.bisect_right(LIS, A[i])] = A[i]
