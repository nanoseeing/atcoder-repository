import math
import pickle


def fastcopy(obj):
    return pickle.loads(pickle.dumps(obj, -1))


class Env:

    def __init__(self):
        self.W, self.D, self.N, self.a = self._input()

    def _input(self):
        W, D, N = map(int, input().split())
        a = []
        for d in range(D):
            a.append(list(map(int, input().split())))
        return W, D, N, a


env = Env()

ans = []
for d in range(env.D):
    x1 = 0
    y1 = 0
    x2 = 1000
    y2 = 1000

    now_ans = [None for _ in range(env.N)]
    areas = env.a[d]
    areas = [(i, area) for i, area in enumerate(areas)]
    for _ in range(env.N):
        area_diffs = []
        now_lr = x2 - x1
        now_ud = y2 - y1
        for now_i, (i, area) in enumerate(areas):
            len1 = math.ceil(area / now_lr)
            len2 = math.ceil(area / now_ud)
            if len1 * now_lr < len2 * now_ud:
                direct = "ud"
                diff = len1 * now_lr - area
                area_diffs.append((diff, len1, direct, i, now_i))
            else:
                direct = "lr"
                diff = len2 * now_ud - area
                area_diffs.append((diff, len2, direct, i, now_i))
        min_diff, min_len, min_direct, min_i, min_now_i = sorted(area_diffs)[0]
        if min_direct == "ud":
            now_ans[min_i] = (x1, y1, x2, y1 + min_len)
            y1 += min_len
        else:
            now_ans[min_i] = (x2 - min_len, y1, x2, y2)
            x2 -= min_len
        areas.pop(min_now_i)
    ans.append(now_ans)

for d in range(env.D):
    for k in range(env.N):
        i0, j0, i1, j1 = ans[d][k]
        print(i0, j0, i1, j1)
