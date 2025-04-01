# from loguru import logger
import bisect

# logger.add("log.txt")
mu_mids = []


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


def calc_mu_sigma(k, e, vs):
    mu = (k - vs) * e + vs * (1 - e)
    sigma = k * e * (1 - e)
    return mu, sigma


def query1(coordinate):
    q = "q 1 {} {}".format(coordinate[0], coordinate[1])
    print(q)
    resp = int(input())

    # logger.info(q)
    # logger.info(resp)

    return resp


def answer(has_oils):
    q = "a {} {}".format(
        len(has_oils), " ".join(map(lambda k: "{} {}".format(k[0], k[1]), has_oils))
    )
    print(q)
    resp = input()

    # logger.info(q)
    # logger.info(resp)

    return resp == "1"


def query2(coordinates):
    coordinate = []
    for x, y in coordinates:
        coordinate.append(x)
        coordinate.append(y)

    q = f"q {len(coordinates)} " + " ".join(map(str, coordinate))
    print(q)
    resp = int(input())

    # logger.info(q)
    # logger.info(resp)

    return resp


def resp_to_correct_num(search_resp):
    global mu_mids
    return bisect.bisect_left(mu_mids, search_resp)


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


def my_greedy(N, M, ESP, GRID):
    if not ESP <= 0.01:
        return False

    total = 0
    total_grid = sum(len(grid) for grid in GRID)

    coordinates = []
    for x, y in custom_search_inds(N):
        coordinates.append((x, y))

    search_n = len(coordinates) - 1
    not_greedy_n = search_n % 3
    greedy_n = search_n - not_greedy_n

    has_oils = []
    for i in range(0, greedy_n, 3):
        n1 = resp_to_correct_num(query2([coordinates[i], coordinates[i + 1]]))
        n2 = resp_to_correct_num(query2([coordinates[i], coordinates[i + 2]]))
        n3 = resp_to_correct_num(query2([coordinates[i + 1], coordinates[i + 2]]))
        a = (n1 + n2 - n3) // 2
        b = (n1 - n2 + n3) // 2
        c = (-n1 + n2 + n3) // 2
        if a > 0:
            has_oils.append(coordinates[i])
        if b > 0:
            has_oils.append(coordinates[i + 1])
        if c > 0:
            has_oils.append(coordinates[i + 2])
        total += a + b + c
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

    mus = []
    for vs in range(N**2 * M + 1):
        mu, sigma = calc_mu_sigma(2, ESP, vs)
        mus.append(mu)

    global mu_mids
    for i in range(len(mus) - 1):
        mu_mids.append((mus[i] + mus[i + 1]) / 2)

    if my_greedy(N, M, ESP, GRID):
        exit()
    else:
        greedy(N, M, ESP, GRID)


main()
