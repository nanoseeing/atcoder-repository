N, K = map(int, input().split())

cnt = [0] * (N + 1)
for i in range(2, N + 1):
    if cnt[i] != 0:
        continue
    for j in range(i, N + 1, i):
        cnt[j] += 1
    cnt[i] = 1

ans = sum(cnt[i] >= K for i in range(1, N + 1))
print(ans)
