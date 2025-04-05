import random
import time
from pathlib import Path
import os


def is_out_of_range(y, x, N):
    return not ((0 <= x < N) and (0 <= y < N))


class Env:
    def __init__(self, file_path: str):
        with open(file_path, mode="r") as f:
            lines = f.readlines()
            N, M, ESP = lines[0].split()
            N, M, ESP = int(N), int(M), float(ESP)
            GRID = []
            lines = lines[1:]
            for _line in lines[:M]:
                _line = _line.split()
                ps = []
                for i in range(int(_line[0])):
                    ps.append((int(_line[2 * i + 1]), int(_line[2 * i + 2])))
                GRID.append(ps)
            lines = lines[2 * M :]
            ANS_GRID = []
            for _line in lines[:N]:
                ANS_GRID.append(list(map(int, _line.split())))

        self.N: int = N
        self.M: int = M
        self.esp: float = ESP
        self.grids: list[list[tuple[int, int]]] = GRID
        self.ans_grid: list[list[int]] = ANS_GRID

        self.total_oil_num: int = sum(len(g) for g in self.grids)
        self.score: int = 10**9

        cnt: int = 0
        for y in range(N):
            for x in range(N):
                if self.ans_grid[y][x] > 0:
                    cnt += 1
        self.ans_point_size: int = cnt

        self._sum_oil: int = 0
        self._score: float = 0

        self.plus_coordinates: list[tuple[int, int]] = []
        self.queries1: list[tuple[tuple[int, int], int]] = []
        self.fixed_board: list[list[int]] = [[-1 for _ in range(N)] for _ in range(N)]

        self._out_file_path: Path = Path("out.txt")
        self._is_output_file = True

        if self._is_output_file:
            os.remove(self._out_file_path)
            self._out_file_path.touch()

    def is_ok(self):
        return self._sum_oil == self.total_oil_num

    def query1(self, coordinate) -> int:
        y, x = coordinate
        self._score += 1
        resp: int = self.ans_grid[y][x]
        self.queries1.append((coordinate, resp))
        self.fixed_board[y][x] = resp
        if resp > 0:
            self._sum_oil += resp
            self.plus_coordinates.append(coordinate)
        if self._is_output_file:
            q = "q 1 {} {}".format(coordinate[0], coordinate[1])
            with open(self._out_file_path, mode="+a") as f:
                f.write(q + "\n")
        return resp

    def answer(self, coordinates) -> bool:
        is_ok = self.total_oil_num == sum(self.ans_grid[y][x] for y, x in coordinates)
        if is_ok:
            self.score = self._score
        else:
            self._score += 1
        if self._is_output_file:
            q = "a {} {}".format(len(coordinates), " ".join(map(lambda k: "{} {}".format(k[0], k[1]), coordinates)))
            with open(self._out_file_path, mode="+a") as f:
                f.write(q + "\n")
        return is_ok

    def answer_from_board(self, board: list[list[int]]) -> bool:
        has_oils: list[tuple[int, int]] = []
        for y in range(self.N):
            for x in range(self.N):
                if board[y][x] > 0:
                    has_oils.append((y, x))
        return self.answer(has_oils)

    @property
    def query1_count(self) -> int:
        return len(self.queries1)


# ----- Solver -----
class BoardSimulator:

    SIMULATE_NUM: int = 30
    TIME_OUT: float = 2.0

    def __init__(self, env: Env):
        self.env: Env = env

        self.simulate_avg_num: float = 0
        self.simulate_cnt: int = 0

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
            for y in range(N):
                for x in range(N):
                    flag: bool = True
                    for coordinate in now_grids:
                        ny, nx = y + coordinate[0], x + coordinate[1]
                        if is_out_of_range(ny, nx, N):
                            flag = False
                            break
                        if f_board[ny][nx] == -1:
                            continue
                        if f_board[ny][nx] == 0:
                            flag = False
                            break
                    if flag:
                        now_placement_indexes.append((y, x))
            placement_indexes.append(now_placement_indexes)

        return placement_indexes

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

        placement_indexes: dict[int, list[tuple[int, int]]] = self._search_placement_indexes()
        for simulate_num in range(BoardSimulator.SIMULATE_NUM):
            now_board: list[list[int]] = [[0 for _ in range(N)] for _ in range(N)]
            for m in range(M):
                # sampling_index: tuple[int, int] = random.choice(placement_indexes[m])
                # sampling_index: tuple[int, int] = random.choice(placement_indexes[m])
                random_ind = (simulate_num * 199712 + self.env.query1_count * 3145 + 11) % len(placement_indexes[m])
                sampling_index: tuple[int, int] = placement_indexes[m][random_ind]
                for y, x in grids[m]:
                    ny, nx = y + sampling_index[0], x + sampling_index[1]
                    now_board[ny][nx] += 1
            cost: float = self._calc_cost_from_simulated_board(now_board)
            if cost == 0:
                if ans_board is None:
                    ans_board = now_board
                    only_one_ans_board = True
                else:
                    only_one_ans_board = False
            for y in range(N):
                for x in range(N):
                    if now_board[y][x] > 0:
                        plus_count_board[y][x] += 1 / (cost + 1)
            if time.time() - start_time > BoardSimulator.TIME_OUT / N**2:
                # シミュレート部分に合計3秒以上かけるとまずい
                break
        self.simulate_avg_num = 1 / (1 + self.simulate_cnt) * (self.simulate_cnt + simulate_num)

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

        if only_one_ans_board:
            return max_coordinate, ans_board
        else:
            return max_coordinate, None


def solve(env: Env):

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


def main():
    TEST_NUM = 1
    random.seed(0)
    for test_num in range(TEST_NUM):
        print(f"===== {test_num+1}/{TEST_NUM} =====")
        env = Env(file_path=f"./in/{test_num:0>4}.txt")
        solve(env)
        print(f"score:{env.score}")


if __name__ == "__main__":
    main()
