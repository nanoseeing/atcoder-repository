import time
import random
import z3
from itertools import product
import time
import random


def is_out_of_range(y, x, N):
    return not ((0 <= x < N) and (0 <= y < N))


# ----- Domain ----- #
class Env:

    def __init__(self):
        _line = input().split()
        N = int(_line[0])
        M = int(_line[1])
        ESP = float(_line[2])
        GRID = []
        for _ in range(M):
            _line = input().split()
            ps = []
            for i in range(int(_line[0])):
                ps.append((int(_line[2 * i + 1]), int(_line[2 * i + 2])))
            GRID.append(ps)

        self.N = N
        self.M = M
        self.esp = ESP
        self.grids = GRID

        self.total_oil_num = sum(len(g) for g in self.grids)
        self.score = 10**9

        self._sum_oil = 0
        self._score = 0

        self.plus_coordinates = []
        self.queries1 = []
        self.queries2 = []
        self.fixed_board = [[-1 for _ in range(N)] for _ in range(N)]

    def is_ok(self):
        return self._sum_oil == self.total_oil_num

    def query1(self, coordinate):
        q = "q 1 {} {}".format(coordinate[0], coordinate[1])
        print(q)
        resp = int(input())

        self.queries1.append((coordinate, resp))

        y, x = coordinate
        self.fixed_board[y][x] = resp
        if resp > 0:
            self._sum_oil += resp
            self.plus_coordinates.append(coordinate)

        return resp

    def query2(self, coordinates):
        coordinate = []
        for y, x in coordinates:
            coordinate.append(y)
            coordinate.append(x)

        q = f"q {len(coordinates)} " + " ".join(map(str, coordinate))
        print(q)
        resp = int(input())

        self._score += 1 / len(coordinates) ** 0.5
        self.queries2.append(resp)
        return resp

    def answer(self, has_oils):
        q = "a {} {}".format(len(has_oils), " ".join(map(lambda k: "{} {}".format(k[0], k[1]), has_oils)))
        print(q)
        resp = input()
        return resp == "1"

    def answer_from_board(self, board):
        has_oils = []
        for y in range(self.N):
            for x in range(self.N):
                if board[y][x] > 0:
                    has_oils.append((y, x))
        return self.answer(has_oils)


class BoardSimulator:

    SIMULATE_TIME_OUT = 2.0
    SIMULATE_MAX = 100000
    SIMULATE_MIN = 50
    MIN_INDEX_CANDIDATES = 100

    def __init__(self, env: Env):
        self.env: Env = env

        self.simulate_avg_num: float = 0
        self.simulate_cnt: int = 0

        self._indexes_candidate: list[tuple[int]] = []
        self._placement_indexes: list[list[tuple[int, int]]] = self._search_placement_indexes()

    def _search_placement_indexes(self) -> list[list[tuple[int, int]]]:
        """現在の確定盤面に対して配置可能なインデックスを計算する"""

        N: int = self.env.N
        M: int = self.env.M
        f_board: list[list[int]] = self.env.fixed_board
        grids: list[list[tuple[int, int]]] = self.env.grids

        placement_indexes: list[list[tuple[int, int]]] = []
        for m in range(M):
            now_grids: list[tuple[int, int]] = grids[m]
            now_placement_indexes: list[tuple[int, int]] = []
            is_m_indexes: list[tuple[int, int]] = []
            for y in range(N):
                for x in range(N):
                    flag: bool = True
                    max_diff = -1
                    for coordinate in now_grids:
                        ny, nx = y + coordinate[0], x + coordinate[1]
                        if is_out_of_range(ny, nx, N) or f_board[ny][nx] == 0:
                            flag = False
                            break
                        max_diff = max(max_diff, f_board[ny][nx])
                    if flag:
                        now_placement_indexes.append((y, x))
                        if max_diff == M:
                            is_m_indexes.append((y, x))
            if len(is_m_indexes) > 0:
                placement_indexes.append(is_m_indexes)
            else:
                placement_indexes.append(now_placement_indexes)

        return placement_indexes

    def update(self, coordinate, resp):
        if resp == 0 or resp == self.env.M:
            self._placement_indexes = self._search_placement_indexes()

    def _get_indexes_candidate(self):
        if len(self._indexes_candidate) > 0:
            return self._indexes_candidate

        M: int = self.env.M
        for _ in range(BoardSimulator.MIN_INDEX_CANDIDATES):
            sampling_indexes: list[tuple[int, int]] = []
            for m in range(M):
                sampling_index: tuple[int, int] = random.choice(self._placement_indexes[m])
                sampling_indexes.append(sampling_index)
            self._indexes_candidate.append(sampling_indexes)
        return self._indexes_candidate

    def _update_indexes_candidate(self, stock_indexes: list[list[tuple[int, int]]], stock_costs: list[float]):
        sorted_costs, sorted_indexes = zip(*sorted(zip(stock_costs, stock_indexes)))
        self._indexes_candidate = sorted_indexes[: BoardSimulator.MIN_INDEX_CANDIDATES]

    def _calc_cost_from_simulated_board(self, simulated_board: list[list[int]]) -> float:

        N: int = self.env.N
        f_board: list[list[int]] = self.env.fixed_board

        cost: float = 0
        for y in range(N):
            for x in range(N):
                if f_board[y][x] == -1:
                    continue
                v: int = abs(simulated_board[y][x] - f_board[y][x])
                cost += v
        return cost

    def get_next_coordinate(self) -> tuple[tuple[int, int], None | list[list[int]]]:
        """
        現在の盤面と最も一致している（コストの低い）盤面で、最も正の数である確率が高い座標を返す。
        """

        start_time: float = time.time()

        N: int = self.env.N
        M: int = self.env.M
        f_board: list[list[int]] = self.env.fixed_board
        grids: list[list[tuple[int, int]]] = self.env.grids

        plus_count_board: list[list[float]] = [[0.0 for _ in range(N)] for _ in range(N)]
        ans_board: list[list[int]] = None
        only_one_ans_board: bool = False

        indexes_candidate = self._get_indexes_candidate()

        stock_indexes: list[list[tuple[int, int]]] = []
        stock_costs: list[float] = []

        random_index_prob = max(1.0 / M * 1.74, 1.0 - (self.simulate_cnt / (N**2 / 2.83)))
        for simulate_num in range(BoardSimulator.SIMULATE_MAX):
            now_board: list[list[int]] = [[0 for _ in range(N)] for _ in range(N)]
            sampling_indexes = list(random.choice(indexes_candidate))
            for m in range(M):
                if random.random() < random_index_prob:
                    sampling_indexes[m] = random.choice(self._placement_indexes[m])
            for m in range(M):
                sampling_index = sampling_indexes[m]
                for y, x in grids[m]:
                    ny, nx = y + sampling_index[0], x + sampling_index[1]
                    now_board[ny][nx] += 1
            cost: float = self._calc_cost_from_simulated_board(now_board)
            stock_indexes.append(sampling_indexes)
            stock_costs.append(cost)

            if cost == 0:
                if ans_board is None:
                    ans_board = now_board
                    only_one_ans_board = True
                else:
                    only_one_ans_board = False
            for y in range(N):
                for x in range(N):
                    if now_board[y][x] > 0:
                        plus_count_board[y][x] += now_board[y][x] / (cost + 1)
            if simulate_num < BoardSimulator.SIMULATE_MIN:
                continue
            if (time.time() - start_time) > BoardSimulator.SIMULATE_TIME_OUT / N**2:
                break

        self._update_indexes_candidate(stock_indexes, stock_costs)

        max_coordinate: tuple[int, int] = (-1, -1)
        max_cnt: int = -1
        for y in range(N):
            for x in range(N):
                if f_board[y][x] != -1:
                    continue
                cnt = plus_count_board[y][x]
                if cnt > max_cnt:
                    max_cnt = cnt
                    max_coordinate = (y, x)

        self.simulate_cnt += 1

        if only_one_ans_board:
            return max_coordinate, ans_board
        else:
            return max_coordinate, None


# --------------------  #
class SMTSolver:

    def __init__(self, env: Env):
        placements_grid_set = {}
        for m in range(env.M):
            now_grid_set = [[set() for _ in range(env.N)] for _ in range(env.N)]
            target_grids = env.grids[m]
            for x in range(env.N):
                for y in range(env.N):
                    tmp_grids = []
                    is_ok = True
                    for coordinate in target_grids:
                        ny, nx = coordinate[0] + y, coordinate[1] + x
                        if is_out_of_range(ny, nx, env.N):
                            is_ok = False
                            break
                        tmp_grids.append((ny, nx))
                    if not is_ok:
                        continue
                    for coordinate in tmp_grids:
                        ty, tx = coordinate
                        now_grid_set[ty][tx].add((y, x))
            placements_grid_set[m] = now_grid_set

        self.placements_grid_set = placements_grid_set
        self.env = env

    def solve(self):
        z3_solver = z3.Solver()
        env = self.env

        # 配列外参照にならないインデックスを追加
        var_x = {}
        for m in range(env.M):
            for y, x in product(list(range(env.N)), list(range(env.N))):
                target_grids = env.grids[m]
                flag = True
                for coordinate in target_grids:
                    ny, nx = coordinate[0] + y, coordinate[1] + x
                    if is_out_of_range(ny, nx, env.N):
                        flag = False
                        break
                if flag:
                    var_x[m, y, x] = z3.Bool(f"var_x{m, y, x}")

        # 配置はどれか1つを選択しなければならない
        for m in range(env.M):
            now_vars = []
            for y, x in product(list(range(env.N)), list(range(env.N))):
                ind = y * env.N + x
                if (m, y, x) in var_x:
                    now_vars.append(var_x[m, y, x])
            z3_solver.add(z3.Sum(now_vars) == 1)

        # 配置した箇所に対応するグリッドが加算される
        set_coordinates = set([coordinate for coordinate, resp in env.queries1])
        for y, x in product(list(range(env.N)), list(range(env.N))):
            bool_vars = []
            for m in range(env.M):
                for ty, tx in self.placements_grid_set[m][y][x]:
                    if (m, ty, tx) in var_x:
                        bool_vars.append(var_x[m, ty, tx])
            if (y, x) in set_coordinates:
                for query in env.queries1:
                    coordinate, resp = query
                    if coordinate == (y, x):
                        z3_solver.add(z3.Sum(bool_vars) == resp)

        # 解を求める
        z3_solver.set("timeout", 350)
        if not z3_solver.check() == z3.sat:
            return None
        model = z3_solver.model()

        placement_ans = []
        for m, y, x in product(list(range(env.M)), list(range(env.N)), list(range(env.N))):
            if (m, y, x) in var_x:
                if model.eval(var_x[m, y, x]):
                    placement_ans.append((m, y, x))

        point_ans = set()
        for m, y, x in placement_ans:
            target_grids = env.grids[m]
            for coordinate in target_grids:
                ny, nx = coordinate[0] + y, coordinate[1] + x
                point_ans.add((ny, nx))

        return point_ans


def solve1(env: Env):

    SMT_TIME_OUT = 1.5
    smt_level = [0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.4, 0.5, 1.0]
    # smt_level = [0.04, 0.9, 0.14, 0.18, 0.24, 0.3, 0.4, 0.5, 1.0]
    # smt_level = [0.03, 0.065, 0.10, 0.15, 0.2, 0.25, 0.3, 0.4, 0.5, 1.0]

    BoardSimulator.SIMULATE_TIME_OUT = 0.5
    BoardSimulator.SIMULATE_MIN = 10

    start_time = time.time()
    board_simulator = BoardSimulator(env)
    for i in range(500):
        if env.is_ok():
            env.answer(env.plus_coordinates)
            break
        coordinate, ans_board = board_simulator.get_next_coordinate()
        if ((time.time() - start_time) < SMT_TIME_OUT) and i >= env.N**2 * smt_level[0]:
            smt_solver = SMTSolver(env)
            ans_points = smt_solver.solve()
            if ans_points:
                if env.answer(ans_points):
                    break
            smt_level.pop(0)
        elif ans_board is not None:
            if env.answer_from_board(ans_board):
                break
        resp = env.query1(coordinate)
        board_simulator.update(coordinate, resp)


def solve2(env: Env):

    board_simulator = BoardSimulator(env)
    for i in range(500):
        if env.is_ok():
            env.answer(env.plus_coordinates)
            break
        coordinate, ans_board = board_simulator.get_next_coordinate()
        if ans_board is not None:
            if env.answer_from_board(ans_board):
                break
        resp = env.query1(coordinate)
        board_simulator.update(coordinate, resp)


def solve(env: Env):
    if env.M <= 2 or (env.N <= 15 and env.M <= 3):
        solve1(env)
    else:
        solve2(env)


def main():
    random.seed(0)
    env = Env()
    solve(env)


if __name__ == "__main__":
    main()
