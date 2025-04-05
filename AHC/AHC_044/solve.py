import math
import random
import time

DEBUG: bool = True
MAX_TIME: float = 1.8  # sec
INIT_TEMP: float = 5000.0
MIN_TEMP: float = 1.0
MIN_L: int = 5_000
MAX_L: int = 500_000


def cooling_schedule(initial_temp: float, min_temp: float, elapsed_time: float, max_time: float) -> float:
    """
    時間に基づく指数減衰の冷却スケジュール
    T(t) = T_min + (T_max - T_min) * exp(-λ * t)
    λ は max_time の範囲で温度を min_temp にするように調整
    """
    lambda_param = math.log(initial_temp / min_temp) / max_time
    return min_temp + (initial_temp - min_temp) * math.exp(-lambda_param * elapsed_time)


def exponential_schedule(init: float, obj: float, elapsed_time: float, max_time: float) -> float:
    """
    時間に基いて指数関数的に上昇するスケジュール
    """
    lambda_param = math.log(obj / init) / max_time
    return init * math.exp(lambda_param * elapsed_time)


def linear_schedule(init: float, obj: float, elapsed_time: float, max_time: float) -> float:
    """
    時間に基づく線形減衰・増加
    """
    return init + (obj - init) * (elapsed_time / max_time)


def neighbor_function(x: tuple[list[int], list[int]]) -> tuple[list[int], list[int]]:
    an, bn = x
    new_an = an.copy()
    new_bn = bn.copy()
    if random.random() < 0.5:
        i = random.randint(0, N - 1)
        j = random.randint(0, N - 1)
        if random.random() < 0.5:
            new_an[i], new_an[j] = new_an[j], new_an[i]
        else:
            new_bn[i], new_bn[j] = new_bn[j], new_bn[i]
    else:
        weight_t = [t / L for t in T]
        sample_n = random.sample(range(N), weights=weight_t, k=1)[0]
        i = random.randint(0, N - 1)
        if random.random() < 0.5:
            new_an[i] = sample_n
        else:
            new_bn[i] = sample_n
    return new_an, new_bn


def inline_input() -> tuple[int, int, list[int]]:
    N, L = map(int, input().split())
    T = list(map(int, input().split()))
    return N, L, T


def file_input(file_path: str) -> tuple[int, int, list[int]]:
    with open(file_path, "r") as f:
        N, L = map(int, f.readline().split())
        T = list(map(int, f.readline().split()))
    return N, L, T


def answer_output_inline(an: list[int], bn: list[int]):
    ans_text = ""
    for a, b in zip(an, bn):
        ans_text += f"{a} {b}\n"
    print(ans_text, end="")


def answer_output_file(an: list[int], bn: list[int], file_path: str):
    ans_text = ""
    for a, b in zip(an, bn):
        ans_text += f"{a} {b}\n"
    with open(file_path, "w") as f:
        f.write(ans_text)


def calc_score(x: tuple[list[int], list[int]]) -> int:
    an, bn = x
    counts = [0] * N
    pre_n = 0
    for _ in range(L):
        counts[pre_n] += 1
        if counts[pre_n] % 2 != 0:
            next_n = an[pre_n]
        else:
            next_n = bn[pre_n]
        pre_n = next_n
    score = 0
    for i in range(N):
        score += abs(T[i] - counts[i])
    return 10**6 - score


def calc_score_simulate(x, test_l):
    """score計算を早くするためにシミュレート回数を減らす"""
    an, bn = x
    counts = [0] * N
    pre_n = 0
    for _ in range(test_l):
        counts[pre_n] += 1
        if counts[pre_n] % 2 != 0:
            next_n = an[pre_n]
        else:
            next_n = bn[pre_n]
        pre_n = next_n
    score = 0

    rate = test_l / L
    rate_inv = L / test_l
    for i in range(N):
        score += abs(T[i] * rate - counts[i]) * rate_inv
    return 10**6 - score


def get_random_ans(N, L, T):
    mean_t = L / N
    random_list = []
    for i, t in enumerate(T):
        r = max(1, round(t / mean_t * 2))
        random_list.extend([i] * r)

    if len(random_list) < N * 2:
        random_list.extend([random.randint(0, N)] * (N * 2 - len(random_list)))
    elif len(random_list) > N * 2:
        random_list = random_list[: N * 2]

    random.shuffle(random_list)
    an = []
    bn = []
    for i in range(N * 2):
        if i % 2 == 0:
            an.append(random_list[i])
        else:
            bn.append(random_list[i])
    return an, bn


def simulated_annealing(
    initial_x: tuple[list[int], list[int]], initial_temp: float, min_temp: float, max_time: float, display: bool
) -> tuple[tuple[list[int], list[int]], float]:
    """
    焼きなまし法の実装
    :param initial_x: 初期解
    :param initial_temp: 初期温度
    :param min_temp: 最小温度
    :param max_iter: 最大試行回数 (None の場合、時間制限のみで実行)
    :param max_time: 最大実行時間（秒） (None の場合、試行回数のみで実行)
    :return: 最適解
    """

    x = initial_x
    current_cost = -calc_score(x)  # 初期解のコストを計算
    best_x = x
    best_cost = current_cost

    start_time = time.time()  # 開始時刻を記録
    iteration = 0

    while True:
        elapsed_time = time.time() - start_time
        if elapsed_time >= max_time:
            break
        temp = cooling_schedule(initial_temp, min_temp, elapsed_time, max_time)
        if temp < min_temp:
            break

        # 近傍解を生成
        new_x = neighbor_function(x)
        now_l = int(exponential_schedule(MIN_L, MAX_L, elapsed_time, max_time))
        now_l = min(now_l, L)
        new_cost = -calc_score_simulate(new_x, now_l)
        delta_cost = new_cost - current_cost

        # 改善されるなら更新、悪化しても確率的に更新
        if delta_cost < 0 or random.random() < math.exp(-delta_cost / temp):
            x = new_x
            current_cost = new_cost

        # 最良解の更新
        if current_cost < best_cost:
            best_x = x
            best_cost = current_cost

        iteration += 1
        if display and iteration % 100 == 0:
            print(f"Iteration: {iteration}, Best cost: {best_cost}, Current cost: {current_cost}")

    return best_x, best_cost


if __name__ == "__main__":
    if DEBUG:
        N, L, T = file_input("tools/in/0000.txt")
    else:
        N, L, T = inline_input()

    initial_x = get_random_ans(N, L, T)
    best_x, best_cost = simulated_annealing(
        initial_x=initial_x,
        initial_temp=INIT_TEMP,
        min_temp=MIN_TEMP,
        max_time=MAX_TIME,
        display=DEBUG,
    )
    an, bn = best_x

    if DEBUG:
        answer_output_file(an, bn, "./output/random_ans.txt")
        print(calc_score((an, bn)))
    else:
        answer_output_inline(an, bn)
