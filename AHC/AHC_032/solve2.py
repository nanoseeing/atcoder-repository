import sys
import pickle
import random
import time

# ONLINE_JUDGE = True if len(sys.argv) >= 2 and sys.argv[1] == "ONLINE_JUDGE" else False
ONLINE_JUDGE = True

HW = 3
N = 9
M = 20
K = 81
INF = 10**18
MOD = 998244353


SIMULATE_NUM = 100000
MAX_TIME = 1.6


def fastcopy(obj):
    return pickle.loads(pickle.dumps(obj, -1))


def debug_print(*args, **kwargs):
    if not ONLINE_JUDGE:
        print(*args, **kwargs)


def evaluate_score(grid: list[list[int]]) -> int:
    s = 0
    for h in range(N):
        for w in range(N):
            s += grid[h][w] % MOD
    return s


class Env:
    def __init__(self):
        self.grid, self.stamps = self._input()

    def _input(self):
        lines = []
        for _ in range(1 + N + M * HW):
            line = list(map(int, input().split()))
            lines.append(line)

        grid = []
        for i in range(1, 1 + N):
            grid.append(lines[i])
        stamps = []
        for i in range(1 + N, 1 + N + M * HW, 3):
            tmp_stamp = []
            for j in range(HW):
                tmp_stamp.append(lines[i + j])
            stamps.append(tmp_stamp)
        return grid, stamps

    def print_grid(self):
        for d in self.grid:
            debug_print(d)

    def pring_stamps(self):
        for s in self.stamps:
            for d in s:
                debug_print(d)
            debug_print()


def step(grid: list[list[int]], stamps):
    score = evaluate_score(grid)
    ans = None
    for m1 in range(M):
        for h1 in range(N - 3):
            for w1 in range(N - 3):
                now_grid = fastcopy(grid)
                for h in range(HW):
                    for w in range(HW):
                        now_grid[h1 + h][w1 + w] = (now_grid[h1 + h][w1 + w] + stamps[m1][h][w]) % MOD
                tmp_score = evaluate_score(now_grid)
                if tmp_score > score:
                    score = tmp_score
                    ans = (m1, h1, w1)
    return ans


def random_step():
    rand_m = random.randint(0, M - 1)
    rand_h = random.randint(0, N - 3)
    rand_w = random.randint(0, N - 3)
    return rand_m, rand_h, rand_w


def simulate(env: Env, random_k):
    ans_list = []
    now_grid = fastcopy(env.grid)
    for k in range(random_k):
        ans = random_step()
        ans_list.append(ans)
        m1, h1, w1 = ans
        for h in range(HW):
            for w in range(HW):
                now_grid[h1 + h][w1 + w] += env.stamps[m1][h][w]

    for k in range(K - random_k):
        ans = step(now_grid, env.stamps)
        if ans is None:
            debug_print("None", k)
            break
        ans_list.append(ans)
        m1, h1, w1 = ans
        for h in range(HW):
            for w in range(HW):
                now_grid[h1 + h][w1 + w] += env.stamps[m1][h][w]

    score = evaluate_score(now_grid)

    return ans_list, score


def solve(env: Env):
    max_score = 0
    max_ans_list = []
    start_time = time.perf_counter()
    for simulate_n in range(SIMULATE_NUM):
        if time.perf_counter() - start_time > MAX_TIME:
            break
        if simulate_n == 0:
            random_k = 0
        else:
            random_k = 50
        ans_list, score = simulate(env, random_k)
        if score > max_score:
            max_score = score
            max_ans_list = ans_list
    return max_ans_list, max_score


def main():
    env = Env()
    ans_list, score = solve(env)
    print(len(ans_list))
    ans_s = []
    for line in ans_list:
        ans_s.append(" ".join(map(str, line)))
    print("\n".join(ans_s))


if __name__ == "__main__":
    main()
