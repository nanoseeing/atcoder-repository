import time

import math
import pickle
import bisect
import numpy as np

DEBUG = False
AREA_MAX = 1000 * 1000
AREA_TH = 0.995
INF = 10**18
MAX_TIME = 2.5


def debug_print(*args, **kwargs):
    if DEBUG:
        debug_print(*args, **kwargs)


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


def check_ans(ans: list[tuple[int]]):
    for d in range(len(ans)):
        for coordinates in ans[d]:
            x1, y1, x2, y2 = coordinates
            if x1 < 0 or x2 > 1000 or y1 < 0 or y2 > 1000:
                return False
    return True


def get_diff_max(target_areas: list[list[int]], env: Env):
    now_target_areas = fastcopy(target_areas)
    target_dw = len(target_areas)

    get_ind_from_area = [dict() for _ in range(target_dw)]
    for d in range(target_dw):
        tmp_dict = dict()
        for tn in range(env.N):
            val = now_target_areas[d][tn]
            tmp_dict[val] = tn
        get_ind_from_area[d] = tmp_dict

    diff_max = []
    for n in range(env.N):
        tmp_diff_max = []
        len_n = len(now_target_areas[0])
        for d in range(target_dw):
            for tn in range(len_n):
                left_area = now_target_areas[d][tn]
                seach_areas = []
                for next_d in range(target_dw):
                    if d == next_d:
                        seach_areas.append(left_area)
                        continue
                    b_ind = bisect.bisect_left(now_target_areas[next_d], left_area)
                    if b_ind == len_n:
                        break
                    seach_areas.append(now_target_areas[next_d][b_ind])
                if len(seach_areas) != target_dw:
                    continue
                diff = max(seach_areas) - min(seach_areas)
                max_area = max(seach_areas)
                inds = []
                for si, area in enumerate(seach_areas):
                    inds.append(get_ind_from_area[si][area])
                tmp_diff_max.append((diff, max_area, inds, seach_areas))
        sorted_diff_max = sorted(tmp_diff_max)
        add_diff, add_max_area, add_inds, del_areas = sorted_diff_max[0]
        diff_max.append((add_diff, add_max_area, add_inds))
        for si, area in enumerate(del_areas):
            now_target_areas[si].remove(area)

    return diff_max


def calc_cost(ans: list[list[int]], env: Env):
    partial_cost = 0
    area_cost = 0

    hs = set()
    vs = set()
    for d in range(env.D):
        hs2 = set()
        vs2 = set()
        for k in range(env.N):
            i0, j0, i1, j1 = ans[d][k]
            area = (i1 - i0) * (j1 - j0)
            if env.a[d][k] > area:
                area_cost += 100 * (env.a[d][k] - area)
            for j in range(j0, j1):
                if i0 > 0:
                    hs2.add((i0, j))
                if i1 < env.W:
                    hs2.add((i1, j))
            for i in range(i0, i1):
                if j0 > 0:
                    vs2.add((j0, i))
                if j1 < env.W:
                    vs2.add((j1, i))

        if d > 0:
            for i, j in hs:
                if (i, j) not in hs2:
                    partial_cost += 1
            for j, i in vs:
                if (j, i) not in vs2:
                    partial_cost += 1

        hs = hs2
        vs = vs2
    return partial_cost + area_cost + 1


def one_day_solve_optim(one_day_area: list[int], env: Env):
    x1 = 0
    y1 = 0
    x2 = 1000
    y2 = 1000

    ans = [None for _ in range(env.N)]
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
            ans[min_i] = (x1, y1, x2, y2)
            break
        if min_direct == "ud":
            ans[min_i] = (x1, y1, x2, y1 + min_len)
            y1 += min_len
        else:
            ans[min_i] = (x2 - min_len, y1, x2, y2)
            x2 -= min_len
        areas.pop(min_now_i)
    return [ans]


def one_day_solve_aspect(one_day_area: list[int], env: Env):
    x1 = 0
    y1 = 0
    x2 = 1000
    y2 = 1000

    ans = [None for _ in range(env.N)]
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
                aspect = 1 - min(len1, now_lr) / max(len1, now_lr)
                area_diffs.append((aspect, diff, len1, direct, i, now_i))
            else:
                direct = "lr"
                diff = len2 * now_ud - area
                aspect = 1 - min(len2, now_ud) / max(len2, now_ud)
                area_diffs.append((aspect, diff, len2, direct, i, now_i))
        _, _, min_len, min_direct, min_i, min_now_i = sorted(area_diffs)[0]
        if n == env.N - 1:
            ans[min_i] = (x1, y1, x2, y2)
            break
        if min_direct == "ud":
            ans[min_i] = (x1, y1, x2, y1 + min_len)
            y1 += min_len
        else:
            ans[min_i] = (x2 - min_len, y1, x2, y2)
            x2 -= min_len
        areas.pop(min_now_i)

    if not check_ans([ans]):
        raise ValueError("failed check ans")

    return [ans]


def one_day_solve(one_day_area: list[int], env: Env):
    try:
        ans = one_day_solve_aspect(one_day_area, env)
    except Exception as e:
        debug_print(f"Err: one_day_solve_aspect {e}")
        ans = one_day_solve_optim(one_day_area, env)
    return ans


def common_area_solver_core_opt(ret_area_objs, env: Env):

    ans = []
    target_dw = len(ret_area_objs)

    for d in range(target_dw):
        is_need_areas = [area for area in ret_area_objs[d] if area.need_first]
        is_not_need_areas = [area for area in ret_area_objs[d] if not area.need_first]

        x1 = 0
        y1 = 0
        x2 = 1000
        y2 = 1000

        now_ans = [None for _ in range(env.N)]
        for n in range(env.N):
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
            else:
                raise ValueError("target is empty")

            min_diff, min_len, min_direct, area_id = sorted(area_diffs)[0]
            if n == env.N - 1:
                now_ans[area_id] = (x1, y1, x2, y2)
                break
            if min_direct == "ud":
                now_ans[area_id] = (x1, y1, x2, y1 + min_len)
                y1 += min_len
            else:
                now_ans[area_id] = (x2 - min_len, y1, x2, y2)
                x2 -= min_len

            if is_need_areas:
                is_need_areas = [area for area in is_need_areas if area.id != area_id]
            elif is_not_need_areas:
                is_not_need_areas = [area for area in is_not_need_areas if area.id != area_id]
        ans.append(now_ans)

    if not check_ans(ans):
        raise ValueError("failed check ans")

    return ans


def common_area_solver_core_aspect(ret_area_objs, env: Env):

    ans = []
    target_dw = len(ret_area_objs)

    for d in range(target_dw):
        is_need_areas = [area for area in ret_area_objs[d] if area.need_first]
        is_not_need_areas = [area for area in ret_area_objs[d] if not area.need_first]

        x1 = 0
        y1 = 0
        x2 = 1000
        y2 = 1000

        now_ans = [None for _ in range(env.N)]
        for n in range(env.N):
            area_diffs = []
            now_lr = x2 - x1
            now_ud = y2 - y1
            if is_need_areas:
                for target_area in is_need_areas:
                    len1 = math.ceil(target_area.target_area / now_lr)
                    len2 = math.ceil(target_area.target_area / now_ud)
                    if len1 * now_lr < len2 * now_ud:
                        direct = "ud"
                        aspect = 1 - min(len1, now_lr) / max(len1, now_lr)
                        diff = len1 * now_lr - target_area.target_area
                        area_diffs.append((aspect, diff, len1, direct, target_area.id))
                    else:
                        direct = "lr"
                        aspect = 1 - min(len2, now_ud) / max(len2, now_ud)
                        diff = len2 * now_ud - target_area.target_area
                        area_diffs.append((aspect, diff, len2, direct, target_area.id))
            elif is_not_need_areas:
                for target_area in is_not_need_areas:
                    len1 = math.ceil(target_area.target_area / now_lr)
                    len2 = math.ceil(target_area.target_area / now_ud)
                    if len1 * now_lr < len2 * now_ud:
                        direct = "ud"
                        aspect = 1 - min(len1, now_lr) / max(len1, now_lr)
                        diff = len1 * now_lr - target_area.target_area
                        area_diffs.append((aspect, diff, len1, direct, target_area.id))
                    else:
                        direct = "lr"
                        aspect = 1 - min(len2, now_ud) / max(len2, now_ud)
                        diff = len2 * now_ud - target_area.target_area
                        area_diffs.append((aspect, diff, len2, direct, target_area.id))
            else:
                raise ValueError("target is empty")

            _, _, min_len, min_direct, area_id = sorted(area_diffs)[0]
            if n == env.N - 1:
                now_ans[area_id] = (x1, y1, x2, y2)
                break
            if min_direct == "ud":
                now_ans[area_id] = (x1, y1, x2, y1 + min_len)
                y1 += min_len
            else:
                now_ans[area_id] = (x2 - min_len, y1, x2, y2)
                x2 -= min_len

            if is_need_areas:
                is_need_areas = [area for area in is_need_areas if area.id != area_id]
            elif is_not_need_areas:
                is_not_need_areas = [area for area in is_not_need_areas if area.id != area_id]
        ans.append(now_ans)

    if not check_ans(ans):
        raise ValueError("failed check ans")

    return ans


def common_area_solver(target_areas: list[list[int]], env: Env):

    ans = []

    target_dw = len(target_areas)
    diff_max = get_diff_max(target_areas, env)

    # エリアオブジェクトを作成
    area_objs = []
    for target_area in target_areas:
        area_obj = []
        for i in range(env.N):
            area_obj.append(Area(id_val=i, self_area=target_area[i], target_area=target_area[i]))
        area_objs.append(area_obj)

    # 目標面積を決定
    ret_area_objs = fastcopy(area_objs)
    for _, max_area, pattern in diff_max:
        now_area_objs = fastcopy(ret_area_objs)
        for d in range(target_dw):
            now_area_objs[d][pattern[d]].target_area = max_area
            now_area_objs[d][pattern[d]].need_first = True
        is_ok = True
        for d in range(target_dw):
            sum_area = sum(now_area_objs[d][i].target_area for i in range(env.N))
            if sum_area / AREA_MAX > AREA_TH:
                is_ok = False
                break
        if is_ok:
            ret_area_objs = fastcopy(now_area_objs)
        else:
            break

    try:
        ans = common_area_solver_core_aspect(ret_area_objs, env)
        return ans
    except Exception as e:
        debug_print(f"Err: common_area_solver_core_aspect {e}")
        ans = common_area_solver_core_opt(ret_area_objs, env)
        return ans


def fin_solver(target_areas: list[list[int]], env: Env):
    ret = []
    if len(target_areas) == 1:
        ret = one_day_solve(target_areas[0], env)
    else:
        try:
            ret = common_area_solver(target_areas, env)
        except Exception as e:
            debug_print(f"Err: common_area_solver {e}")
            for d in range(len(target_areas)):
                ret += one_day_solve(env.a[d : d + 1], env)
    return ret


def solve(env: Env):
    best_cost = INF
    best_ans = None
    best_shift = None

    start_time = time.perf_counter()
    shift_list = list(set([1, 2] + [int(x) for x in np.linspace(3, env.D, 8)]))
    for shift in shift_list:
        now_time = time.perf_counter()
        ans = []
        for d in range(0, env.D, shift):
            ans += fin_solver(env.a[d : d + shift], env)
        cost = calc_cost(ans, env)
        elapsed_time = time.perf_counter() - now_time
        elapsed_time_total = time.perf_counter() - start_time

        if cost < best_cost:
            best_cost = cost
            best_ans = ans
            best_shift = shift
        if elapsed_time + elapsed_time_total > MAX_TIME:
            # print("over!!!")
            break

    # print(f"best:{best_cost} shift:{best_shift}/{env.D}")
    return best_ans


def main():
    env = Env()
    ans = solve(env)

    str_ans = []
    for d in range(env.D):
        for k in range(env.N):
            i0, j0, i1, j1 = ans[d][k]
            str_ans.append(f"{i0} {j0} {i1} {j1}")

    print("\n".join(str_ans))


if __name__ == "__main__":
    main()
