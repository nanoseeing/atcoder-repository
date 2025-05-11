import itertools
import sys
from collections import Counter

input = sys.stdin.buffer.readline
in_n = lambda: int(input())
in_nn = lambda: map(int, input().split())
in_s = lambda: input().rstrip().decode('utf-8')
in_map = lambda: [s == ord('.') for s in input() if s != ord('\n')]

def create_array(shape: tuple, fill_value):
    if len(shape) == 1:
        return [fill_value() if callable(fill_value) else fill_value for _ in range(shape[0])]
    else:
        return [create_array(shape[1:], fill_value) for _ in range(shape[0])]

MOD = 10**9 + 7
INF = 8 * 10**18


def main():
    N, M = in_nn()
    C = list(in_nn())
    A = create_array((N,), list)
    for m in range(M):
        tmp = list(in_nn())
        for a in tmp[1:]:
            A[a - 1].append(m)
    
    ans = INF
    
    
    
    for comb in itertools.product((0, 1, 2), repeat=N):
        money = 0
        animals = []
        for i, cnt in enumerate(comb):
            money += C[i] * cnt
            animals += A[i] * cnt
        animals = Counter(animals)
        for m in range(M):
            if animals[m] <= 1:
                money = INF
                break
        ans = min(ans, money)
    print(ans)

if __name__ == '__main__':
    main()
