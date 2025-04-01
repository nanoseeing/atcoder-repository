from dataclasses import dataclass

INF = 10**9
N = 5


def problem_input():
    lines = []
    for _ in range(N + 1):
        lines.append(input())
    a = []
    for line in lines[1:]:
        arr = list(map(int, line.strip().split()))
        a.append(arr)

    return a


@dataclass
class Clane:
    clane_id: int
    pos_x: int
    pos_y: int
    have_container: int | None
    is_exploded: bool


class Game:

    def __init__(self, input_containers: list[list[int]]):
        self.input_containers = input_containers
        self.output_containers = [[] for _ in range(N)]
        self.container_maps = [[None for _ in range(N)] for _ in range(N)]
        self.commands = []
        self.clanes: list[Clane] = []
        for n in range(N):
            self.clanes.append(Clane(n, 0, n, None, False))

        self.ouput_container_nums = 0

    def is_finished(self):
        return self.ouput_container_nums == N * N

    def get_ans_commands(self):
        ans_command = ["" for _ in range(N)]
        for command in self.commands:
            for n in range(N):
                ans_command[n] += command[n]
        return ans_command

    def print_ans(self):
        ans_command = self.get_ans_commands()
        for command in ans_command:
            print(command)

    def _turn_first(self):
        for n in range(N):
            if self.container_maps[n][0] is None and self.input_containers[n]:
                self.container_maps[n][0] = self.input_containers[n].pop(0)

    def _turn_end(self):
        for n in range(N):
            target_container = self.container_maps[n][N - 1]
            if target_container is not None:
                self.output_containers[n].append(target_container)
                self.container_maps[n][N - 1] = None
                self.ouput_container_nums += 1

    def get_next_output_containernums(self):
        ret = []
        for n in range(N):
            if 0 < len(self.output_containers[n]) < N:
                next_container = self.output_containers[n][-1] + 1
            elif len(self.output_containers[n]) == N:
                next_container = None
            elif len(self.output_containers[n]) == 0:
                next_container = n * N
            else:
                raise ValueError("invalid output container")
            ret.append(next_container)
        return ret

    def search_next_output_container_pos(self):
        next_output_container = self.get_next_output_containernums()
        next_output_container_nums = set([x for x in next_output_container if x is not None])

        for row in range(N):
            for col in range(N):
                if self.container_maps[row][col] is None:
                    continue
                if self.container_maps[row][col] in next_output_container_nums:
                    target_container = self.container_maps[row][col]
                    target_row = next_output_container.index(target_container)
                    return (col, row, N - 1, target_row)
        return None

    def search_shallow_next_output(self):
        next_output_container = self.get_next_output_containernums()
        next_output_container = set([x for x in next_output_container if x is not None])

        ans_row = None
        now_min = INF
        for n in range(N):
            for i, container in enumerate(self.input_containers[n]):
                if container in next_output_container:
                    if i < now_min:
                        now_min = i
                        ans_row = n

        if ans_row is None:
            raise ValueError("not found")

        return (0, ans_row)

    def search_nearest_empty_pos(self, now_x, now_y):

        ans_pos = None
        min_d = INF
        for row in range(N):
            for col in range(N):
                if self.container_maps[row][col] is not None:
                    continue
                d = abs(row - now_y) + abs(col - now_x)
                if d < min_d:
                    min_d = d
                    ans_pos = (col, row)

        if ans_pos is None:
            raise ValueError("not found")

        return ans_pos

    def _turn(self, command: str):
        if len(command) != N:
            raise ValueError("invalid command length")

        for n in range(N):
            now_command = command[n]
            px = self.clanes[n].pos_x
            py = self.clanes[n].pos_y
            if now_command == "P":
                if self.clanes[n].have_container is not None:
                    raise ValueError("clane have container")
                if self.container_maps[py][px] is None:
                    raise ValueError("container is not exist")
                self.clanes[n].have_container = self.container_maps[py][px]
                self.container_maps[py][px] = None
            elif now_command == "Q":
                if self.clanes[n].have_container is None:
                    raise ValueError("clane not have container")
                if self.container_maps[py][px] is not None:
                    raise ValueError("container is exist")
                self.container_maps[py][px] = self.clanes[n].have_container
                self.clanes[n].have_container = None
            elif now_command == "B":
                if self.clanes[n].have_container is not None:
                    raise ValueError("clane have container")
                self.clanes[n].is_exploded = True
            elif now_command in ["U", "D", "L", "R"]:
                if now_command == "U":
                    self.clanes[n].pos_y -= 1
                elif now_command == "D":
                    self.clanes[n].pos_y += 1
                elif now_command == "L":
                    self.clanes[n].pos_x -= 1
                elif now_command == "R":
                    self.clanes[n].pos_x += 1
                if self.clanes[n].pos_x < 0 or self.clanes[n].pos_x >= N:
                    raise ValueError("clane out of range")
                if self.clanes[n].pos_y < 0 or self.clanes[n].pos_y >= N:
                    raise ValueError("clane out of range")
            elif now_command == ".":
                # 何もしない
                pass
            else:
                raise ValueError("invalid command")
        self.commands.append(command)

    def turn(self, command: str):
        self._turn_first()
        self._turn(command)
        self._turn_end()


def move_help(nx, ny, dx, dy):
    diff_x = dx - nx
    diff_y = dy - ny

    tmp_commands = ""
    if diff_x > 0:
        tmp_commands += "R" * abs(diff_x)
    if diff_x < 0:
        tmp_commands += "L" * abs(diff_x)
    if diff_y > 0:
        tmp_commands += "D" * abs(diff_y)
    if diff_y < 0:
        tmp_commands += "U" * abs(diff_y)

    turn_commands = []
    for tmp_command in tmp_commands:
        turn_commands.append(tmp_command + "....")

    return turn_commands


def solve():
    A = problem_input()

    game = Game(A)
    game.turn(".BBBB")  # クレーン0以外のクレーンを爆破

    while not game.is_finished():
        # print("---")
        # for n in range(N):
        #     print(game.container_maps[n])
        # print(game.output_containers)
        # print(game.clanes[0])

        ret = game.search_next_output_container_pos()
        if ret is not None:
            # map内に次のcontainerがある場合、そのcontainerを取りに行って、次のcontainerを置く場所に移動する
            nx, ny, dx, dy = ret
            commands = move_help(game.clanes[0].pos_x, game.clanes[0].pos_y, nx, ny)
            for command in commands:
                game.turn(command)
            game.turn("P....")
            commands = move_help(nx, ny, dx, dy)
            for command in commands:
                game.turn(command)
            game.turn("Q....")
        else:
            nx, ny = game.search_shallow_next_output()
            dx, dy = game.search_nearest_empty_pos(nx, ny)
            commands = move_help(game.clanes[0].pos_x, game.clanes[0].pos_y, nx, ny)
            for command in commands:
                game.turn(command)
            game.turn("P....")
            commands = move_help(nx, ny, dx, dy)
            for command in commands:
                game.turn(command)
            game.turn("Q....")

    game.print_ans()


if __name__ == "__main__":
    solve()
