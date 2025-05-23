import random
import time


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
        self.fixed_board = [[None for _ in range(N)] for _ in range(N)]

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
        q = "a {} {}".format(
            len(has_oils), " ".join(map(lambda k: "{} {}".format(k[0], k[1]), has_oils))
        )
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


# ----- Solver -----
class BoardSimulator:

    SIMULATE_NUM = 10**8
    TIME_OUT = 2.0

    def __init__(self, env: Env):
        self.env: Env = env

        self.simulate_avg_num = 0
        self.simulate_cnt = 0

    def _search_placement_indexes(self):
        """現在の確定盤面に対して配置可能なインデックスを計算する"""

        N = self.env.N
        M = self.env.M
        f_board = self.env.fixed_board
        grids = self.env.grids

        placement_indexes = {}
        for m in range(M):
            now_grids = grids[m]
            now_placement_indexes = set()
            for y in range(N):
                for x in range(N):
                    flag = True
                    for coordinate in now_grids:
                        ny, nx = y + coordinate[0], x + coordinate[1]
                        if is_out_of_range(ny, nx, N):
                            flag = False
                            break
                        if f_board[ny][nx] is None:
                            continue
                        if f_board[ny][nx] == 0:
                            flag = False
                            break
                    if flag:
                        now_placement_indexes.add((y, x))
            placement_indexes[m] = list(now_placement_indexes)

        return placement_indexes

    def _calc_cost_from_simulated_board(self, simulated_board):

        N = self.env.N
        f_board = self.env.fixed_board

        cost = 0
        for y in range(N):
            for x in range(N):
                if f_board[y][x] is None:
                    continue
                v = abs(simulated_board[y][x] - f_board[y][x])
                cost += v
        return cost

    def _simulate_board(self):
        """
        現在の盤面と最も一致している（コストの低い）盤面で、最も正の数である確率が高い座標を返す。
        """

        start_time = time.time()

        N = self.env.N
        M = self.env.M
        f_board = self.env.fixed_board
        grids = self.env.grids

        plus_count_board = [[0 for _ in range(N)] for _ in range(N)]

        ans_board = None
        only_one_ans_board = False

        placement_indexes = self._search_placement_indexes()
        for simulate_num in range(BoardSimulator.SIMULATE_NUM):
            now_board = [[0 for _ in range(N)] for _ in range(N)]
            for m in range(M):
                sampling_index = random.choice(placement_indexes[m])
                for y, x in grids[m]:
                    ny, nx = y + sampling_index[0], x + sampling_index[1]
                    now_board[ny][nx] += 1
            cost = self._calc_cost_from_simulated_board(now_board)
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
        self.simulate_avg_num = (
            1 / (1 + self.simulate_cnt) * (self.simulate_cnt + simulate_num)
        )

        max_coordinate = None
        max_cnt = -1
        for y in range(N):
            for x in range(N):
                if f_board[y][x] is not None:
                    continue
                cnt = plus_count_board[y][x]
                if cnt > max_cnt:
                    max_cnt = cnt
                    max_coordinate = (y, x)

        if only_one_ans_board:  # and (self.env.query1_count > (N**2 * 0.15)):
            return max_coordinate, ans_board
        else:
            return max_coordinate, None

    def get_next_coordinate(self):
        return self._simulate_board()


# ----- Run -----
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
    random.seed(0)
    env = Env()
    solve(env)


if __name__ == "__main__":
    main()
