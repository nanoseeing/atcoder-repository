import math
import pickle
import math
import pickle
from itertools import product

AREA_MAX = 1000 * 1000


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


class Area:
    def __init__(self, id_val: int, self_area: int, target_area: int):
        self.id = id_val
        self.self_area = self_area
        self.target_area = target_area
        self.need_first = False


def check_ans(ans: list[list[int]]):
    for d in range(len(ans)):
        for coordinates in ans[d]:
            x1, y1, x2, y2 = coordinates
            if x1 < 0 or x2 > 1000 or y1 < 0 or y2 > 1000:
                return False
    return True


def one_day_solve(one_day_area: list[int], env: Env):
    x1 = 0
    y1 = 0
    x2 = 1000
    y2 = 1000

    now_ans = [None for _ in range(env.N)]
    areas = [(i, area) for i, area in enumerate(one_day_area)]
    for n in range(env.N):
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
        if n == env.N - 1:
            now_ans[min_i] = (x1, y1, x2, y2)
            break
        if min_direct == "ud":
            now_ans[min_i] = (x1, y1, x2, y1 + min_len)
            y1 += min_len
        else:
            now_ans[min_i] = (x2 - min_len, y1, x2, y2)
            x2 -= min_len
        areas.pop(min_now_i)
    return now_ans


def common_area_solver(target_areas: list[list[int]], env: Env):

    ans = []

    target_dw = len(target_areas)

    # 共通化したときの差分を計算
    combs = list(product(list(range(env.N)), repeat=target_dw))
    diff_max = []
    for comb in combs:
        tmp_arr = []
        for i in range(target_dw):
            tmp_arr.append(target_areas[i][comb[i]])
        diff = max(tmp_arr) - min(tmp_arr)
        diff_max.append((diff, max(tmp_arr), comb))
    diff_max = sorted(diff_max)

    # 差分最小のものから組み合わせを決定
    now_diff_max = diff_max
    ret_all_combs = []
    for _ in range(env.N):
        now_diff = now_diff_max[0]
        ret_all_combs.append(now_diff)

        next_diff_max = []
        pattern1 = now_diff[2]
        for diff in now_diff_max:
            _, _, pattern2 = diff
            if any(pattern1[i] == pattern2[i] for i in range(target_dw)):
                continue
            next_diff_max.append(diff)
        now_diff_max = next_diff_max

    # エリアオブジェクトを作成
    area_objs = []
    for target_area in target_areas:
        area_obj = []
        for i in range(env.N):
            area_obj.append(Area(id_val=i, self_area=target_area[i], target_area=target_area[i]))
        area_objs.append(area_obj)

    # 目標面積を決定
    ret_area_objs = fastcopy(area_objs)
    for _, max_area, pattern in ret_all_combs:
        now_area_objs = fastcopy(ret_area_objs)
        for d in range(target_dw):
            now_area_objs[d][pattern[d]].target_area = max_area
            now_area_objs[d][pattern[d]].need_first = True
        is_ok = True
        for d in range(target_dw):
            sum_area = sum(now_area_objs[d][i].target_area for i in range(env.N))
            if sum_area / AREA_MAX > 0.995:
                is_ok = False
                break
        if is_ok:
            ret_area_objs = fastcopy(now_area_objs)
        else:
            break

    # 共通部を先に決定しながら配置場所を確定
    for d in range(target_dw):
        is_need_areas = [area for area in ret_area_objs[d] if area.need_first]
        is_not_need_areas = [area for area in ret_area_objs[d] if not area.need_first]

        x1 = 0
        y1 = 0
        x2 = 1000
        y2 = 1000

        now_ans = [None for _ in range(env.N)]
        for _ in range(env.N):
            area_diffs = []
            now_lr = x2 - x1
            now_ud = y2 - y1
            if is_need_areas:
                for target_area in is_need_areas:
                    len1 = math.ceil(target_area.target_area / now_lr)
                    len2 = math.ceil(target_area.target_area / now_ud)
                    if len1 * now_lr < len2 * now_ud:
                        direct = "ud"
                        diff = len1 * now_lr - target_area.target_area
                        area_diffs.append((diff, len1, direct, target_area.id))
                    else:
                        direct = "lr"
                        diff = len2 * now_ud - target_area.target_area
                        area_diffs.append((diff, len2, direct, target_area.id))
                min_diff, min_len, min_direct, area_id = sorted(area_diffs)[0]
                if min_direct == "ud":
                    now_ans[area_id] = (x1, y1, x2, y1 + min_len)
                    y1 += min_len
                else:
                    now_ans[area_id] = (x2 - min_len, y1, x2, y2)
                    x2 -= min_len
                is_need_areas = [area for area in is_need_areas if area.id != area_id]
            elif is_not_need_areas:
                for target_area in is_not_need_areas:
                    len1 = math.ceil(target_area.target_area / now_lr)
                    len2 = math.ceil(target_area.target_area / now_ud)
                    if len1 * now_lr < len2 * now_ud:
                        direct = "ud"
                        diff = len1 * now_lr - target_area.target_area
                        area_diffs.append((diff, len1, direct, target_area.id))
                    else:
                        direct = "lr"
                        diff = len2 * now_ud - target_area.target_area
                        area_diffs.append((diff, len2, direct, target_area.id))
                min_diff, min_len, min_direct, area_id = sorted(area_diffs)[0]
                if min_direct == "ud":
                    now_ans[area_id] = (x1, y1, x2, y1 + min_len)
                    y1 += min_len
                else:
                    now_ans[area_id] = (x2 - min_len, y1, x2, y2)
                    x2 -= min_len
                is_not_need_areas = [area for area in is_not_need_areas if area.id != area_id]
            else:
                raise ValueError()
        ans.append(now_ans)

    if not check_ans(ans):
        raise ValueError()
    return ans


def solve2(env: Env):
    ans = []
    for d in range(0, env.D, 2):
        if d + 1 < env.D:
            try:
                ans += common_area_solver(env.a[d : d + 2], env)
            except Exception as e:
                ans.append(one_day_solve(env.a[d], env))
                ans.append(one_day_solve(env.a[d + 1], env))
        else:
            ans.append(one_day_solve(env.a[d], env))
    return ans


def main():
    env = Env()

    ans = solve2(env)
    str_ans = []
    for d in range(env.D):
        for k in range(env.N):
            i0, j0, i1, j1 = ans[d][k]
            str_ans.append(f"{i0} {j0} {i1} {j1}")

    print("\n".join(str_ans))


if __name__ == "__main__":
    main()
