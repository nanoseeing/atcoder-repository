import math

from src.common import EnvOffline, calc_rectangle_max_dist, calc_rectangle_min_dist, divide_interval

EPS = 1e-9


def test_divide_interval_exclusive():
    def is_close_intervals(intervals1, intervals2):
        if len(intervals1) != len(intervals2):
            return False
        for i in range(len(intervals1)):
            if abs(intervals1[i] - intervals2[i]) > EPS:
                return False
        return True

    assert is_close_intervals(divide_interval(0, 1, 3, is_exclude_edge=True), [0.25, 0.5, 0.75])
    assert is_close_intervals(divide_interval(0, 1, 3, is_exclude_edge=False), [0, 0.5, 1])


def test_calc_rectangle_max_dist():
    r1 = (0, 2, 0, 2)
    r2 = (-2, 0, -2, 0)
    assert calc_rectangle_max_dist(r1, r2) == math.sqrt(16 + 16)

    r1 = (0, 2, 0, 2)
    r2 = (0, 2, 2, 100)
    assert calc_rectangle_max_dist(r1, r2) == math.sqrt(4 + 10000)

    r1 = (0, 2, 0, 2)
    r2 = (1, 3, 5, 100)
    assert calc_rectangle_max_dist(r1, r2) == math.sqrt(9 + 10000)

    r1 = (0, 2, 0, 2)
    r2 = (-1, 3, 5, 100)
    assert calc_rectangle_max_dist(r1, r2) == math.sqrt(9 + 10000)

    r1 = (0, 2, 0, 2)
    r2 = (3, 5, 3, 5)
    assert abs(calc_rectangle_max_dist(r1, r2) - math.sqrt(25 + 25)) < EPS


def test_calc_rectangle_min_dist():
    r1 = (0, 2, 0, 2)
    r2 = (-2, 0, -2, 0)
    assert calc_rectangle_min_dist(r1, r2) == 0

    r1 = (0, 2, 0, 2)
    r2 = (0, 2, 2, 100)
    assert calc_rectangle_min_dist(r1, r2) == 0

    r1 = (0, 2, 0, 2)
    r2 = (1, 3, 5, 100)
    assert calc_rectangle_min_dist(r1, r2) == 3

    r1 = (0, 2, 0, 2)
    r2 = (-1, 3, 5, 100)
    assert calc_rectangle_min_dist(r1, r2) == 3

    r1 = (0, 2, 0, 2)
    r2 = (3, 5, 3, 5)
    assert abs(calc_rectangle_min_dist(r1, r2) - math.sqrt(2)) < EPS
