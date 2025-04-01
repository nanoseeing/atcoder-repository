from collections import deque
from collections import defaultdict
from collections import Counter
from heapq import heappush, heappop, heapify
import itertools
import bisect
import math
from fractions import Fraction
import re
import numpy as np
from numba import njit
import Python.ACL.atcoder as ac

sys.setrecursionlimit(10**7)


""" numpy関連 """


# 2次元入力 ([.##.] [####] [.#.#])
in_map2 = lambda: np.array([s == ord('.') for s in read() if s != ord('\n')])


# JITコンパイル
@njit('(i8[:],)', cache=True)
def solve():
    pass


# AOTコンパイル
def cc_export():
    from numba.pycc import CC
    cc = CC('my_module')
    cc.export('solve', '(i4,)')(solve)
    cc.compile()


if __name__ == '__main__':
    import sys
    if sys.argv[-1] == 'ONLINE_JUDGE':
        cc_export()
        exit(0)
    from my_module import solve
    main()


""" itertools """

import itertools


# 順列
list(itertools.permutations([1, 2, 3, 4], 2))
# 組み合わせ
list(itertools.combinations([1, 2, 3, 4], 2))
# 重複順列（bit全探索）
list(itertools.product([0, 1], repeat=10))
# 重複組み合わせ
list(itertools.combinations_with_replacement(nums, 2))
# 累積和
list(itertools.accumulate(ary))
