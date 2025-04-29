import bisect
import itertools
import math
import re
from collections import Counter, defaultdict, deque
from fractions import Fraction
from heapq import heapify, heappop, heappush

import numpy as np
from numba import njit

sys.setrecursionlimit(10**7)


""" numpy関連 """


# 2次元入力 ([.##.] [####] [.#.#])
in_map2 = lambda: np.array([s == ord(".") for s in read() if s != ord("\n")])


# JITコンパイル
@njit("(i8[:],)", cache=True)
def solve():
    pass


# AOTコンパイル
def cc_export():
    from numba.pycc import CC

    cc = CC("my_module")
    cc.export("solve", "(i4,)")(solve)
    cc.compile()


if __name__ == "__main__":
    import sys

    if sys.argv[-1] == "ONLINE_JUDGE":
        cc_export()
        exit(0)
    from my_module import solve

    main()


""" itertools """

import itertools
