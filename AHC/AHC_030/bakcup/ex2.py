# RESULT: 182.50078546
# ESP == 0.01のときのみ3つ探索する

import bisect
import numpy as np
import itertools


def calc_mu_sigma(k, e, vs):
    mu = (k - vs) * e + vs * (1 - e)
    sigma = k * e * (1 - e)
    return mu, sigma


class ResponseSimulator:

    def __init__(self, n: int, esp: float):
        self.esp = esp
        self.n = n

    def calc_best_query_num(self):
        INF = 10**9

        best_cost = INF
        best_q = 1
        for q in range(1, 15):
            cost = self.inference_score(q)
            if cost < best_cost:
                best_q = q
                best_cost = cost
        return best_q, best_cost

    def inference_correct_prob(self, query_num):

        mus = []
        for vs in range(201):
            mu, sigma = calc_mu_sigma(query_num, self.esp, vs)
            mus.append(mu)

        mu_mids = []
        for i in range(len(mus) - 1):
            mu_mids.append((mus[i] + mus[i + 1]) / 2)

        SAMPLE_N = 10**5
        model_vs = query_num  # 代表正解値。なんでもよいはず。

        mu, sigma = calc_mu_sigma(query_num, self.esp, model_vs)
        rng = np.random.default_rng()
        samples = rng.normal(mu, sigma, SAMPLE_N)

        correct_cnt = 0
        for sample in samples:
            if model_vs == bisect.bisect_left(mu_mids, sample):
                correct_cnt += 1
        prob = correct_cnt / SAMPLE_N
        return prob

    def inference_score(self, query_num: int):
        if query_num == 1:
            return self.n**2
        grid_size = self.n**2
        prob = self.inference_correct_prob(query_num) ** grid_size
        expected_cost = (grid_size / np.sqrt(query_num)) + grid_size * (1 - prob)
        return expected_cost

    def env_setting(self, query_num: int):
        mus = []
        for vs in range(201):
            mu, sigma = calc_mu_sigma(query_num, self.esp, vs)
            mus.append(mu)

        mu_mids = []
        for i in range(len(mus) - 1):
            mu_mids.append((mus[i] + mus[i + 1]) / 2)

        self.query_num = query_num
        self.mu_mids = mu_mids

    def calc_true_val_from_response(self, resp_val):
        return bisect.bisect_left(self.mu_mids, resp_val)


def custom_search_inds(N):
    ret = []
    ind = [N // 2]
    for i in range(1, N // 2 + 2):
        left = N // 2 + i
        right = N // 2 - i
        if left < N:
            ind.append(N // 2 + i)
        if 0 <= right:
            ind.append(N // 2 - i)
    for x in ind:
        for y in ind:
            ret.append((x, y))

    return ret


def query1(coordinate):
    q = "q 1 {} {}".format(coordinate[0], coordinate[1])
    print(q)
    resp = int(input())

    return resp


def answer(has_oils):
    q = "a {} {}".format(
        len(has_oils), " ".join(map(lambda k: "{} {}".format(k[0], k[1]), has_oils))
    )
    print(q)
    resp = input()

    return resp == "1"


def query2(coordinates):
    coordinate = []
    for x, y in coordinates:
        coordinate.append(x)
        coordinate.append(y)

    q = f"q {len(coordinates)} " + " ".join(map(str, coordinate))
    print(q)
    resp = int(input())

    return resp


def std_inputs():
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

    return N, M, ESP, GRID


def greedy(N, M, ESP, GRID):

    total = 0
    total_grid = sum(len(grid) for grid in GRID)
    coordinates = custom_search_inds(N)

    has_oils = []
    for x, y in coordinates[:-1]:
        resp = query1((x, y))
        total += resp
        if resp != 0:
            has_oils.append((x, y))
        if total == total_grid:
            return answer(has_oils)

    last_grid = total_grid - total
    if last_grid > 0:
        has_oils.append(coordinates[-1])
    return answer(has_oils)


def my_greedy(N, M, ESP, GRID, response_simulator: ResponseSimulator):

    query_num = response_simulator.query_num

    total = 0
    total_grid = sum(len(grid) for grid in GRID)

    coordinates = []
    for x, y in custom_search_inds(N):
        coordinates.append((x, y))

    search_n = len(coordinates) - 1
    not_greedy_n = search_n % (query_num + 1)
    greedy_n = search_n - not_greedy_n

    has_oils = []

    def solving_simultaneous(query_indexs, coordinate_combs):
        query_vals = {}
        for comb in coordinate_combs:
            resp_val = response_simulator.calc_true_val_from_response(
                query2([coordinates[c] for c in comb])
            )
            query_vals[comb] = resp_val

        ans = {}
        abcde = sum(query_vals.values()) // query_num
        for comb in coordinate_combs:
            not_index = list(set(query_indexs) - set(comb))[0]
            v = abcde - query_vals[comb]
            if v < 0:
                return False, None
            ans[not_index] = v

        return True, ans

    for i in range(0, greedy_n, query_num + 1):
        query_indexs = list(range(i, i + query_num + 1))
        coordinate_combs = list(itertools.combinations(query_indexs, query_num))

        for _ in range(2):
            is_ok, ans = solving_simultaneous(query_indexs, coordinate_combs)
            if is_ok:
                break
        else:
            return False

        for ind, v in ans.items():
            total += v
            if v > 0:
                has_oils.append(coordinates[ind])

        if total == total_grid:
            return answer(has_oils)
        if total > total_grid:
            return False

    for i in range(not_greedy_n):
        target_coordinate = coordinates[greedy_n + i]
        num = query1(target_coordinate)
        if num > 0:
            has_oils.append(target_coordinate)
        total += num

    last_grid = total_grid - total
    if last_grid > 0:
        has_oils.append(coordinates[-1])

    return answer(has_oils)


def main():
    N, M, ESP, GRID = std_inputs()

    if ESP <= 0.01:
        response_simulator = ResponseSimulator(n=N, esp=ESP)
        response_simulator.env_setting(query_num=3)
        if not my_greedy(N, M, ESP, GRID, response_simulator):
            greedy(N, M, ESP, GRID)
    else:
        greedy(N, M, ESP, GRID)


main()
