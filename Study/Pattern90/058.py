N, K = map(int, input().split())

MAX_K = 70
MAX_N = 10**5
doubling = [[0 for _ in range(MAX_N)] for _ in range(MAX_K)]


def calc(x):
    ret = 0
    for v in str(x):
        ret += int(v)
    return (ret + x) % (10**5)


for i in range(MAX_N):
    doubling[0][i] = calc(i)

for k in range(1, MAX_K):
    for n in range(MAX_N):
        doubling[k][n] = doubling[k - 1][doubling[k - 1][n]]


cnt = 0
k = K
start = N
while k > 0:
    if k & 1:
        start = doubling[cnt][start]
    k >>= 1
    cnt += 1
print(start)
