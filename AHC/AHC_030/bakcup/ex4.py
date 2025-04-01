from collections import deque


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

        self.total_size = sum(len(g) for g in self.grids)
        self._score = 0
        self.score = 10**9

    def query1(self, coordinate):
        q = "q 1 {} {}".format(coordinate[0], coordinate[1])
        print(q)
        resp = int(input())
        return resp

    def query2(self, coordinates):
        coordinate = []
        for x, y in coordinates:
            coordinate.append(x)
            coordinate.append(y)

        q = f"q {len(coordinates)} " + " ".join(map(str, coordinate))
        print(q)
        resp = int(input())
        return resp

    def answer(self, has_oils):
        q = "a {} {}".format(
            len(has_oils), " ".join(map(lambda k: "{} {}".format(k[0], k[1]), has_oils))
        )
        print(q)
        resp = input()
        return resp == "1"


# ----------------------- #


class BFSSolver:
    """
    幅優先探索で貪欲的に確定する
    RESULT: 127.69
    """

    def __init__(self, N, total_oil_num):
        self.N = N
        self.grid_count = [[-1 for _ in range(N)] for _ in range(N)]
        self.grid_response = [[False for _ in range(N)] for _ in range(N)]
        self.grid_distance = [[0 for _ in range(N)] for _ in range(N)]
        for x in range(N):
            for y in range(N):
                self.grid_distance[y][x] = min(x + 1, y + 1, N - x, N - y)

        self.total_count = 0
        self.total_oil_nums = total_oil_num

        self.next_grids = set()

    def is_ok(self):
        return self.total_count == self.total_oil_nums

    def get_plus_grids(self):
        ret = []
        for x in range(self.N):
            for y in range(self.N):
                if self.grid_count[y][x] == -1:
                    continue
                if self.grid_count[y][x] > 0:
                    ret.append((x, y))
        return ret

    def response(self, x, y, val):
        self.grid_count[y][x] = val
        self.grid_response[y][x] = True
        if val > 0:
            self.total_count += val
            for dx, dy in [[0, 1], [1, 0], [0, -1], [-1, 0]]:
                nx, ny = x + dx, y + dy
                if not (0 <= nx < self.N and 0 <= ny < self.N):
                    continue
                if not self.grid_response[ny][nx]:
                    self.next_grids.add((nx, ny))
        self._update_distance(x, y)

    def get_next_search_point(self):
        if self.next_grids:
            return self.next_grids.pop()
        else:
            return self._get_far_point_from_wall()

    def _update_distance(self, x, y):
        searched = [[False for _ in range(self.N)] for _ in range(self.N)]
        self.grid_distance[y][x] = 0
        start = [(x, y)]
        search_q = deque(start)
        while search_q:
            px, py = search_q.popleft()
            searched[py][px] = True
            if searched[py][px]:
                continue
            for dx, dy in [[0, 1], [1, 0], [0, -1], [-1, 0]]:
                nx, ny = px + dx, py + dy
                if not (0 <= nx < self.N and 0 <= ny < self.N):
                    continue
                if self.grid_distance[ny][nx] == 0 or searched[ny][nx]:
                    continue
                search_q.append((nx, ny))
                self.grid_distance[ny][nx] = min(
                    self.grid_distance[ny][nx], self.grid_distance[py][px] + 1
                )

    def _get_far_point_from_wall(self):
        max_val = max(max(l for l in self.grid_distance))
        for x in range(self.N):
            for y in range(self.N):
                if self.grid_response[y][x]:
                    continue
                if self.grid_distance[y][x] == max_val:
                    return x, y


# ----------------------- #
def main():
    env = Env()

    solver = BFSSolver(env.N, total_oil_num=env.total_size)
    for _ in range(env.N**2 + 1):
        x, y = solver.get_next_search_point()
        resp = env.query1([x, y])
        solver.response(x, y, resp)
        if solver.is_ok():
            break
    env.answer(solver.get_plus_grids())


if __name__ == "__main__":
    main()
