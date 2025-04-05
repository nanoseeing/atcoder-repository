import sys
import pickle

BEAM_WIDTH = 5

HW = 3
N = 9
M = 20
K = 81
INF = 10**18
MOD = 998244353
ONLINE_JUDGE = True if len(sys.argv) >= 2 and sys.argv[1] == "ONLINE_JUDGE" else False


def fastcopy(obj):
    return pickle.loads(pickle.dumps(obj, -1))


def debug_print(*args, **kwargs):
    if not ONLINE_JUDGE:
        print(*args, **kwargs)


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


def evaluate_score(grid: list[list[int]]) -> int:
    s = 0
    for h in range(N):
        for w in range(N):
            s += grid[h][w] % MOD
    return s


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


class API:
    def __init__(self, n, env: Env):
        self.iter = 0
        self.n = n
        self.env = env

    def init(self):
        self.iter = 0

    def step(self, grid_and_paths):
        grid, paths = grid_and_paths
        for m1 in range(M):
            for h1 in range(N - 3):
                for w1 in range(N - 3):
                    now_grid = fastcopy(grid)
                    for h in range(HW):
                        for w in range(HW):
                            now_grid[h1 + h][w1 + w] = (now_grid[h1 + h][w1 + w] + self.env.stamps[m1][h][w]) % MOD
                    yield now_grid, paths + [(m1, h1, w1)]

    def score(self, grid_and_paths):
        grid, paths = grid_and_paths
        return evaluate_score(grid)

    def count(self):
        self.iter += 1

    def terminate(self):
        return self.iter >= self.n


from heapq import heapify, heappush, heappop, heappushpop


def beam_search(root, k, api: API):
    """
    Args:
        root : root node
        k : number of remain paths during search
        api : apis for beam search
    Notes:
        api must have functions as follows.
        (1) init : this is called at the begenning of this function
        (2) step : return path-list or path-generator of extended path from inputed path
        (3) score : return score for path, higher scores indicate better
        (4) count : this function is called for every end of loop
        (5) terminate : return true if it should terminate to search else false
    """
    paths = [(None, root)]
    heapify(paths)
    api.init()

    while not api.terminate():
        top_paths = []
        heapify(top_paths)
        for _, path in paths:
            for extend_path in api.step(path):
                score = api.score(extend_path)
                if len(top_paths) < k:
                    heappush(top_paths, (score, extend_path))
                else:
                    heappushpop(top_paths, (score, extend_path))
        paths = top_paths
        api.count()

    result_paths = []
    result_paths_score = []
    for _, path in paths:
        result_paths.append(path)
        result_paths_score.append(score)

    max_ind = -1
    max_score = -1
    for i, score in enumerate(result_paths_score):
        if score > max_score:
            max_score = score
            max_ind = i

    max_grid, max_path = result_paths[max_ind]
    max_score = result_paths_score[max_ind]
    return max_path, max_score


def solve(env: Env):
    result_paths, result_paths_score = beam_search((env.grid, []), BEAM_WIDTH, API(K, env))
    return result_paths, result_paths_score


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
