import sys
import math
import pickle
import bisect
import numpy as np
import time

from dataclasses import dataclass
from collections import defaultdict

ONLINE_JUDGE = True if len(sys.argv) >= 2 and sys.argv[1] == "ONLINE_JUDGE" else False
# ONLINE_JUDGE = True
W_SIZE = 1000
AREA_TH = 0.996
INF = 10**18
TIME_MAX = 2.3


@dataclass
class Rect:
    x1: int
    y1: int
    x2: int
    y2: int


# ----- Utils ----- #
def overlap_1d(xl1: int, xr1: int, xl2: int, xr2: int):
    return True if max(xl1, xl2) < min(xr1, xr2) else False


def overlap_1d_ok_just(xl1: int, xr1: int, xl2: int, xr2: int):
    return True if max(xl1, xl2) <= min(xr1, xr2) else False


def overlap_1d_vec_ok_just(vec1: tuple[int, int], vec2: tuple[int, int]):
    return overlap_1d_ok_just(vec1[0], vec1[1], vec2[0], vec2[1])


def overlap_1d_vec(vec1: tuple[int, int], vec2: tuple[int, int]):
    return overlap_1d(vec1[0], vec1[1], vec2[0], vec2[1])


def get_overlap_vec(vec1: tuple[int, int], vec2: tuple[int, int]):
    return (max(vec1[0], vec2[0]), min(vec1[1], vec2[1]))


def concat_1d_vec(vecs: list[tuple[int, int]]) -> list[tuple[int, int]]:
    serched_vecs = []
    for vec in vecs:
        overlap_vecs = [vec]
        non_overlap_vecs = []
        for seached_vec in serched_vecs:
            if overlap_1d_vec_ok_just(vec, seached_vec):
                overlap_vecs.append(seached_vec)
            else:
                non_overlap_vecs.append(seached_vec)
        concat_overlap_vec = (min([v[0] for v in overlap_vecs]), max([v[1] for v in overlap_vecs]))
        serched_vecs = non_overlap_vecs + [concat_overlap_vec]
    return serched_vecs


def evaluate_1d_partition_costs(pre_vecs: list[tuple[int, int]], next_vecs: list[tuple[int, int]]) -> int:
    pre_vecs = concat_1d_vec(pre_vecs)
    next_vecs = concat_1d_vec(next_vecs)

    overlaped_vecs = []
    for pre_vec in pre_vecs:
        for next_vec in next_vecs:
            if overlap_1d_vec(pre_vec, next_vec):
                overlaped_vecs.append(get_overlap_vec(pre_vec, next_vec))

    cost = 0
    cost += sum(vec[1] - vec[0] for vec in pre_vecs)
    cost += sum(vec[1] - vec[0] for vec in next_vecs)

    converged_overlaped_vecs = concat_1d_vec(overlaped_vecs)
    for vec in converged_overlaped_vecs:
        cost -= (vec[1] - vec[0]) * 2
    return cost


def evaluate_partition_rect_cost(pre_rect: list[Rect], next_rect: list[Rect]) -> int:
    pre_rect_x_vecs = defaultdict(list)
    pre_rect_y_vecs = defaultdict(list)
    next_rect_x_vecs = defaultdict(list)
    next_rect_y_vecs = defaultdict(list)

    for rect in pre_rect:
        pre_rect_x_vecs[rect.y1].append((rect.x1, rect.x2))
        pre_rect_x_vecs[rect.y2].append((rect.x1, rect.x2))
        pre_rect_y_vecs[rect.x1].append((rect.y1, rect.y2))
        pre_rect_y_vecs[rect.x2].append((rect.y1, rect.y2))
    for rect in next_rect:
        next_rect_x_vecs[rect.y1].append((rect.x1, rect.x2))
        next_rect_x_vecs[rect.y2].append((rect.x1, rect.x2))
        next_rect_y_vecs[rect.x1].append((rect.y1, rect.y2))
        next_rect_y_vecs[rect.x2].append((rect.y1, rect.y2))

    cost = 0

    x_keys = set(pre_rect_x_vecs.keys()) | set(next_rect_x_vecs.keys())
    for key in x_keys:
        cost += evaluate_1d_partition_costs(pre_rect_x_vecs[key], next_rect_x_vecs[key])

    y_keys = set(pre_rect_y_vecs.keys()) | set(next_rect_y_vecs.keys())
    for key in y_keys:
        cost += evaluate_1d_partition_costs(pre_rect_y_vecs[key], next_rect_y_vecs[key])
    return cost


def evaluate_area_cost(target_rects: list[Rect], target_areas: list[int]) -> int:
    cost = 0
    for i in range(len(target_rects)):
        rect_area = abs(target_rects[i].x2 - target_rects[i].x1) * abs(target_rects[i].y2 - target_rects[i].y1)
        if rect_area < target_areas[i]:
            cost += target_areas[i] - rect_area
    return cost


def evaluate_one_day_cost(pre_rect: list[Rect] | None, next_rect: list[Rect], target_areas: list[int]) -> int:
    if pre_rect is None:
        return evaluate_area_cost(next_rect, target_areas)
    return evaluate_partition_rect_cost(pre_rect, next_rect) + evaluate_area_cost(next_rect, target_areas)


def evaluate_all_cost(ans: list[list[int]], target_areas: list[list[int]], days: int) -> int:

    all_rects = []
    for day in range(days):
        rects = []
        for vec in ans[day]:
            rects.append(Rect(vec[0], vec[1], vec[2], vec[3]))
        all_rects.append(rects)

    cost = 0
    for day in range(days):
        if day == 0:
            cost += evaluate_one_day_cost(None, all_rects[day], target_areas[day])
        else:
            cost += evaluate_one_day_cost(all_rects[day - 1], all_rects[day], target_areas[day])

    return cost + 1


# ----- Utils ----- #


class Env:
    def __init__(self):
        self.W, self.D, self.N, self.a = self._input()

    def _input(self):
        W, D, N = map(int, input().split())
        a = []
        for d in range(D):
            a.append(list(map(int, input().split())))
        return W, D, N, a


def debug_print(*args, **kwargs):
    if not ONLINE_JUDGE:
        print(*args, **kwargs)


def fastcopy(obj):
    return pickle.loads(pickle.dumps(obj, -1))


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
            if x1 < 0 or x2 > W_SIZE or y1 < 0 or y2 > W_SIZE:
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
            partial_cost += len(hs ^ hs2)
            partial_cost += len(vs ^ vs2)

        hs = hs2
        vs = vs2
    return partial_cost + area_cost + 1


def dicision_pos(one_day_area: list[tuple[int, int]], now_lr: int, now_ud: int, x1: int, y1: int, x2: int, y2: int, how: str):
    area_diffs = []
    for i, area in one_day_area:
        len1 = math.ceil(area / now_lr)
        len2 = math.ceil(area / now_ud)
        if len1 * now_lr < len2 * now_ud:
            direct = "ud"
            if how == "diff":
                score = len1 * now_lr - area
            elif how == "aspect":
                score = 1 - min(len1, now_lr) / max(len1, now_lr)
            area_diffs.append((score, len1, direct, i))
        else:
            direct = "lr"
            if how == "diff":
                score = len2 * now_ud - area
            elif how == "aspect":
                score = 1 - min(len2, now_ud) / max(len2, now_ud)
            area_diffs.append((score, len2, direct, i))
    _, min_len, min_direct, min_i = min(area_diffs)
    if min_direct == "ud":
        ans = (min_i, x1, y1, x2, y1 + min_len)
        assigin_area = (x2 - x1) * min_len
        y1 += min_len
    elif min_direct == "lr":
        ans = (min_i, x2 - min_len, y1, x2, y2)
        assigin_area = min_len * (y2 - y1)
        x2 -= min_len
    else:
        raise ValueError("invalid direct")

    for i, area in one_day_area:
        if i == min_i:
            target_area = area

    remain_diff_area = assigin_area - target_area
    return x1, y1, x2, y2, ans, remain_diff_area


def one_day_greedy_solve(one_day_area: list[int], env: Env):
    x1 = 0
    y1 = 0
    x2 = W_SIZE
    y2 = W_SIZE
    remain_area = W_SIZE * W_SIZE - sum(one_day_area)

    ans = [None for _ in range(env.N)]
    one_day_area_with_index = [(i, area) for i, area in enumerate(one_day_area)]
    for n in range(env.N):
        remain_n = env.N - n
        now_lr = x2 - x1
        now_ud = y2 - y1
        if n == env.N - 1:
            # 最後の一個は残り全部
            last_ind = one_day_area_with_index[0][0]
            ans[last_ind] = (x1, y1, x2, y2)
            break

        if remain_n * (W_SIZE - 1) <= remain_area:
            # 残り面積が広いときはアスペクト比を貪欲探索
            x1, y1, x2, y2, now_ans, assigin_area = dicision_pos(one_day_area_with_index, now_lr, now_ud, x1, y1, x2, y2, how="aspect")
        else:
            # 残り面積が狭いときは面積を有効活用
            x1, y1, x2, y2, now_ans, assigin_area = dicision_pos(one_day_area_with_index, now_lr, now_ud, x1, y1, x2, y2, how="diff")
        ans_ind, *ans_tuple = now_ans
        for i, area in one_day_area_with_index:
            if i == ans_ind:
                one_day_area_with_index.remove((i, area))

        ans[ans_ind] = ans_tuple
        remain_area -= assigin_area

    return ans


def common_area_solver(target_areas: list[list[int]], env: Env):

    TARGET_DW = len(target_areas)
    diff_max = get_diff_max(target_areas, env)

    # エリアオブジェクトを作成
    area_objs = []
    for target_area in target_areas:
        area_obj = []
        for i in range(env.N):
            area_obj.append(Area(id_val=i, self_area=target_area[i], target_area=target_area[i]))
        area_objs.append(area_obj)

    # 目標面積を決定
    ret_area_objs: list[list[Area]] = fastcopy(area_objs)
    for _, max_area, pattern in diff_max:
        now_area_objs = fastcopy(ret_area_objs)
        for d in range(TARGET_DW):
            now_area_objs[d][pattern[d]].target_area = max_area
            now_area_objs[d][pattern[d]].need_first = True
        is_ok = True
        for d in range(TARGET_DW):
            sum_area = sum(now_area_objs[d][i].target_area for i in range(env.N))
            # if W_SIZE * W_SIZE - sum_area < (W_SIZE - 1) * env.N:
            if sum_area / (W_SIZE * W_SIZE) > AREA_TH:
                is_ok = False
                break
        if is_ok:
            ret_area_objs = fastcopy(now_area_objs)
        else:
            break

    MAX_SUM_AREA = max(sum(x.target_area for x in ret_area_objs[d]) for d in range(TARGET_DW))
    INIT_REMAIN_AREA = W_SIZE * W_SIZE - MAX_SUM_AREA
    ans_all_day = []
    for d in range(TARGET_DW):
        is_need_areas = [(area_obj.id, area_obj.target_area) for area_obj in ret_area_objs[d] if area_obj.need_first]
        is_not_need_areas = [(area_obj.id, area_obj.target_area) for area_obj in ret_area_objs[d] if not area_obj.need_first]
        x1 = 0
        y1 = 0
        x2 = W_SIZE
        y2 = W_SIZE
        sum_remain_area = 0
        NOW_SUM_AREA = W_SIZE * W_SIZE - sum(x.target_area for x in ret_area_objs[d])

        ans_one_day = [None for _ in range(env.N)]
        for n in range(env.N):
            remain_n = env.N - n
            now_lr = x2 - x1
            now_ud = y2 - y1
            if is_need_areas:
                if n == env.N - 1:
                    # 最後の一個は残り全部
                    last_ind = is_need_areas[0][0]
                    ans_one_day[last_ind] = (x1, y1, x2, y2)
                    break
                if remain_n * (W_SIZE - 1) <= (INIT_REMAIN_AREA - sum_remain_area):
                    # 残り面積が広いときはアスペクト比を貪欲探索
                    x1, y1, x2, y2, now_ans, assigin_area = dicision_pos(is_need_areas, now_lr, now_ud, x1, y1, x2, y2, how="aspect")
                else:
                    # 残り面積が狭いときは面積を有効活用
                    x1, y1, x2, y2, now_ans, assigin_area = dicision_pos(is_need_areas, now_lr, now_ud, x1, y1, x2, y2, how="diff")
                ans_ind, *ans_tuple = now_ans

                for i, area in is_need_areas:
                    if i == ans_ind:
                        is_need_areas.remove((i, area))

                ans_one_day[ans_ind] = ans_tuple
                sum_remain_area += assigin_area
            elif is_not_need_areas:
                if n == env.N - 1:
                    # 最後の一個は残り全部
                    last_ind = is_not_need_areas[0][0]
                    ans_one_day[last_ind] = (x1, y1, x2, y2)
                    break
                if remain_n * (W_SIZE - 1) <= (NOW_SUM_AREA - sum_remain_area):  # 今日エリアでなくなったら残り面積は独自計算
                    # 残り面積が広いときはアスペクト比を貪欲探索
                    x1, y1, x2, y2, now_ans, assigin_area = dicision_pos(is_not_need_areas, now_lr, now_ud, x1, y1, x2, y2, how="aspect")
                else:
                    # 残り面積が狭いときは面積を有効活用
                    x1, y1, x2, y2, now_ans, assigin_area = dicision_pos(is_not_need_areas, now_lr, now_ud, x1, y1, x2, y2, how="diff")
                ans_ind, *ans_tuple = now_ans

                for i, area in is_not_need_areas:
                    if i == ans_ind:
                        is_not_need_areas.remove((i, area))

                ans_one_day[ans_ind] = ans_tuple
                sum_remain_area += assigin_area
            else:
                raise ValueError("invalid area")
        ans_all_day.append(ans_one_day)

    return ans_all_day


def fin_solver(target_areas: list[list[int]], env: Env):
    ret = []
    if len(target_areas) == 1:
        ret = [one_day_greedy_solve(target_areas[0], env)]
    else:
        ret = common_area_solver(target_areas, env)
    return ret


def solve(env: Env):
    best_cost = INF
    best_ans = None
    best_shift = None

    start_time = time.perf_counter()
    shift_list = list(set([1, 2] + [int(x) for x in np.linspace(3, env.D, 8)]))
    for shift in range(1, env.D + 1):
        ans = []
        for d in range(0, env.D, shift):
            ans += fin_solver(env.a[d : d + shift], env)

        if not check_ans(ans):
            raise ValueError("invalid ans")
        cost = evaluate_all_cost(ans, env.a, env.D)

        if cost < best_cost:
            best_cost = cost
            best_ans = ans
            best_shift = shift
        if time.perf_counter() - start_time > TIME_MAX:
            debug_print("over!!!")
            break
        print(f"shift:{shift} cost:{cost}")

    debug_print(f"best:{best_cost} shift:{best_shift}/{env.D}")
    return best_ans


def main():
    env = Env()
    ans = solve(env)

    str_ans = []
    for d in range(env.D):
        for k in range(env.N):
            i0, j0, i1, j1 = ans[d][k]
            str_ans.append(f"{i0} {j0} {i1} {j1}")

    # print("\n".join(str_ans))


if __name__ == "__main__":
    main()
