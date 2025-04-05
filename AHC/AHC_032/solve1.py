import pickle


HW = 3
N = 9
M = 20
K = 81
INF = 10**18
MOD = 998244353
# ONLINE_JUDGE = True if len(sys.argv) >= 2 and sys.argv[1] == "ONLINE_JUDGE" else False
ONLINE_JUDGE = True


def fastcopy(obj):
    return pickle.loads(pickle.dumps(obj, -1))


def debug_print(*args, **kwargs):
    if not ONLINE_JUDGE:
        print(*args, **kwargs)


def evaluate_score(grid):
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


def step(env: Env):
    score = evaluate_score(env.grid)
    ans = None
    for m1 in range(M):
        for h1 in range(N - 3):
            for w1 in range(N - 3):
                now_grid = fastcopy(env.grid)
                for h in range(HW):
                    for w in range(HW):
                        now_grid[h1 + h][w1 + w] = (now_grid[h1 + h][w1 + w] + env.stamps[m1][h][w]) % MOD
                        tmp_score = evaluate_score(now_grid)
                        if tmp_score > score:
                            score = tmp_score
                            ans = (m1, h1, w1)
    return ans


def solve(env: Env):
    ans_list = []
    for k in range(81):
        ans = step(env)
        if ans is None:
            break
        ans_list.append(ans)
        m1, h1, w1 = ans
        for h in range(HW):
            for w in range(HW):
                env.grid[h1 + h][w1 + w] += env.stamps[m1][h][w]
    return ans_list


def main():
    env = Env()
    ans_list = solve(env)
    print(len(ans_list))
    for line in ans_list:
        print(*line)


if __name__ == "__main__":
    main()
