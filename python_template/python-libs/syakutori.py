""" 尺取法テンプレート """

# 尺取法（一般）
count = 0
right = 0
for left in range(N):
    while right < N:
        if judge():
            right += 1
        else:
            break
    count += right - left
    if left == right:
        right += 1


# 尺取法（部分列の積がK以下）
right = 0
ans = 0
now = 1

for left in range(N):

    while right < N:
        if now * S[right] <= K:
            now *= S[right]
            right += 1
        else:
            break

    ans = max(ans, right - left)
    if left == right:
        now = 1
        right += 1
    else:
        now //= S[left]
