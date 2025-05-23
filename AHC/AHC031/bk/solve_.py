from pathlib import Path
import random
import math
import pickle
import time
import numpy as np

INF = 10**9
T0 = 1000
T_MIN = 1
C = 0.8


def fastcopy(obj):
    return pickle.loads(pickle.dumps(obj, -1))


class Env:
    def __init__(self, input_txt_path: Path):
        self.W, self.D, self.N, self.a = self._input(input_txt_path)

    def _input(self, txt_path):
        with open(txt_path, mode="r") as file:
            lines = file.readlines()
        W, D, N = map(int, lines[0].split())
        a = []
        for line, d in zip(lines[1:], range(D)):
            d = list(map(int, line.split()))
            a.append(d)
        return W, D, N, a


def judge_overlap(x1, y1, x2, y2, x3, y3, x4, y4):
    return (max(x1, x3) < min(x2, x4)) and (max(y1, y3) < min(y2, y4))


def judge_overlap_1d(x1, x2, x3, x4):
    return max(x1, x3) < min(x2, x4)


class Rectangle:
    def __init__(self, target_area=None, width=None, height=None, x1=None, y1=None, x2=None, y2=None, area=None):
        self.target_area = target_area
        self.area = area
        self.width = width
        self.height = height
        self.x1 = x1
        self.y1 = y1
        self.x2 = x2
        self.y2 = y2

    def set_xywh(self, x1, y1, w, h):
        self.x1 = x1
        self.y1 = y1
        self.width = w
        self.height = h
        self.x2 = x1 + w
        self.y2 = y1 + h
        self.area = w * h
        self.rect_check()

    def scaling(self, direct: str, num: int):
        if direct == "R+":
            self._x_right_plus(num)
        elif direct == "L+":
            self._x_left_plus(num)
        elif direct == "U+":
            self._y_up_plus(num)
        elif direct == "D+":
            self._y_bottom_plus(num)
        elif direct == "R-":
            self._x_right_plus(-num)
        elif direct == "L-":
            self._x_left_plus(-num)
        elif direct == "U-":
            self._y_up_plus(-num)
        elif direct == "D-":
            self._y_bottom_plus(-num)

    def shift(self, direct: str, num: int):
        if direct == "R":
            self._x_shift(num)
        elif direct == "L":
            self._x_shift(-num)
        elif direct == "U":
            self._y_shift(num)
        elif direct == "D":
            self._y_shift(-num)

    def _x_right_plus(self, plus_w):
        self.x2 += plus_w
        self.width += plus_w
        self.area = self.width * self.height

    def _x_left_plus(self, plus_w):
        self.x1 -= plus_w
        self.width += plus_w
        self.area = self.width * self.height

    def _y_up_plus(self, plus_h):
        self.y2 += plus_h
        self.height += plus_h
        self.area = self.width * self.height

    def _y_bottom_plus(self, plus_h):
        self.y1 -= plus_h
        self.height += plus_h
        self.area = self.width * self.height
        self.rect_check()

    def _x_shift(self, shift):
        self.x1 += shift
        self.x2 += shift

    def _y_shift(self, shift):
        self.y1 += shift
        self.y2 += shift

    def rect_check(self):
        assert 0 <= self.x1 <= self.x2 <= 1000
        assert 0 <= self.y1 <= self.y2 <= 1000
        assert self.x1 + self.width == self.x2
        assert self.y1 + self.height == self.y2

    def judge_overlap(self, other: "Rectangle"):
        return judge_overlap(self.x1, self.y1, self.x2, self.y2, other.x1, other.y1, other.x2, other.y2)

    def __str__(self):
        return f"area: {self.area}, width: {self.width}, height: {self.height}, x1: {self.x1}, y1: {self.y1}, x2: {self.x2}, y2: {self.y2}"

    def __eq__(self, other: "Rectangle"):
        return self.x1 == other.x1 and self.y1 == other.y1 and self.x2 == other.x2 and self.y2 == other.y2


def evaluate_cost(now_rects: list[Rectangle]):
    cost = 0
    for rect in now_rects:
        if rect.area < rect.target_area:
            cost += (rect.target_area - rect.area) * 100
    return cost


def change_action(now_rects: list[Rectangle], W: int):
    ret_rects: list[Rectangle] = fastcopy(now_rects)
    N = len(ret_rects)
    random_ind = random.randint(0, N - 1)
    now_rect = ret_rects[random_ind]

    min_x_right = W - now_rect.x2
    min_x_left = now_rect.x1
    min_y_up = W - now_rect.y2
    min_y_bottom = now_rect.y1
    for i in range(N):
        if i == random_ind:
            continue
        tmp_rect = ret_rects[i]
        if judge_overlap_1d(tmp_rect.y1, tmp_rect.y2, now_rect.y1, now_rect.y2):
            if now_rect.x2 <= tmp_rect.x1:
                min_x_right = min(min_x_right, tmp_rect.x1 - now_rect.x2)
            if tmp_rect.x2 <= now_rect.x1:
                min_x_left = min(min_x_left, now_rect.x1 - tmp_rect.x2)
        if judge_overlap_1d(tmp_rect.x1, tmp_rect.x2, now_rect.x1, now_rect.x2):
            if now_rect.y2 <= tmp_rect.y1:
                min_y_up = min(min_y_up, tmp_rect.y1 - now_rect.y2)
            if tmp_rect.y2 <= now_rect.y1:
                min_y_bottom = min(min_y_bottom, now_rect.y1 - tmp_rect.y2)

    shift_max = 50
    if random.random() < 0.5:
        # 拡大・縮小
        limits = [min_x_right, min_x_left, min_y_up, min_y_bottom, now_rect.width - 1, now_rect.width - 1, now_rect.height - 1, now_rect.height - 1]
        directs = ["R+", "L+", "U+", "D+", "R-", "L-", "U-", "D-"]
        select = random.randint(0, 7)

        now_limit = limits[select]
        now_direct = directs[select]
        if min(now_limit, shift_max) == 0:
            return ret_rects
        random_shift = random.randint(1, min(shift_max, now_limit))
        ret_rects[random_ind].scaling(now_direct, random_shift)
    else:
        # 平行移動
        limits = [min_x_right, min_x_left, min_y_up, min_y_bottom]
        directs = ["R", "L", "U", "D"]
        select_direct = random.randint(0, 3)
        now_limit = limits[select_direct]
        now_direct = directs[select_direct]
        if min(now_limit, shift_max) == 0:
            return ret_rects
        random_shift = random.randint(1, min(shift_max, now_limit))
        ret_rects[random_ind].shift(now_direct, random_shift)

    return ret_rects


def SA(T0, T_MIN, C, simulate_n, rectangles, W: int):
    t = T0

    x_best: list[Rectangle] = fastcopy(rectangles)
    fx_best = evaluate_cost(x_best)

    x_current: list[Rectangle] = fastcopy(rectangles)
    fx_current = fx_best

    total_time = 0
    while t > T_MIN:
        start = time.perf_counter()
        not_changed_cnt = 0
        for _ in range(1, simulate_n + 1):
            x = change_action(x_current, W)
            if x == x_current:
                not_changed_cnt += 1
            fx: float = evaluate_cost(x)
            d: float = fx - fx_current
            # plot(x)
            if d <= 0:
                x_current = x
                fx_current = fx
                if fx < fx_best:
                    x_best = x
                    fx_best = fx_current
            elif random.random() <= np.exp(-d / t):
                x_current = x
                fx_current = fx

        time_sec = time.perf_counter() - start
        total_time += time_sec
        t = C * t
    return x_best, fx_best, total_time


def solve(env: Env):
    div_n = math.ceil(env.N**0.5) + 1
    div_coordinates = []
    for d in range(div_n):
        div_coordinates.append(round(env.W / div_n * d))
    div_coordinates = div_coordinates[1:]
    init_coordinates = []
    for x1 in div_coordinates:
        for y1 in div_coordinates:
            init_coordinates.append((x1, y1))

    now_rects = []
    for area, init_coordinate in zip(env.a[0], init_coordinates):
        x1, y1 = init_coordinate
        rect = Rectangle(target_area=area)
        rect.set_xywh(x1, y1, 10, 10)
        now_rects.append(rect)

    target_time = 2.5 / env.D
    simulate_n = 100
    pre_best = None
    ans: list[list[Rectangle]] = []
    for reservesion in env.a:
        if pre_best:
            now_rects = pre_best
            for i, reserve in enumerate(reservesion):
                now_rects[i].target_area = reserve
        x_best, fx_best, total_time = SA(T0, T_MIN, C, simulate_n, now_rects, env.W)
        x_best = sorted(x_best, key=lambda x: x.area)
        ans.append(x_best)
        pre_best = x_best

        simulate_n = max(1, int(target_time / total_time))

    return ans


def main():
    env = Env()
    ans = solve(env)
    ans_str_list = []
    for rects in ans:
        for rect in rects:
            ans_str_list.append(f"{rect.x1} {rect.y1} {rect.x2} {rect.y2}")
    print("\n".join(ans_str_list))


if __name__ == "__main__":
    main()
