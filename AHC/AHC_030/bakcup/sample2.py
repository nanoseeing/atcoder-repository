from loguru import logger
import bisect

logger.add("log.txt")
mu_mids = []


def calc_mu_sigma(k, e, vs):
    mu = (k - vs) * e + vs * (1 - e)
    sigma = k * e * (1 - e)
    return mu, sigma


def query1(x, y):
    q = "q 1 {} {}".format(x, y)
    print(q)
    resp = int(input())

    logger.info(q)
    logger.info(resp)

    return resp


def answer(has_oils):
    q = "a {} {}".format(
        len(has_oils), " ".join(map(lambda k: "{} {}".format(k[0], k[1]), has_oils))
    )
    print(q)
    resp = input()

    logger.info(q)
    logger.info(resp)
    return resp


def query2(coordinates):
    coordinate = []
    for x, y in coordinates:
        coordinate.append(x)
        coordinate.append(y)

    q = f"q {len(coordinates)} " + " ".join(map(str, coordinate))
    print(q)
    resp = int(input())

    logger.info(q)
    logger.info(resp)

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


def greedy(N):
    has_oil = []
    for x in range(N):
        for y in range(N):
            resp = query1(x, y)
            if resp != 0:
                has_oil.append((x, y))
    resp = answer(has_oil)
    assert resp == "1"


def my_greedy(N, ESP):
    if ESP > 0.05:
        return False

    coordinates = []
    for x in range(N):
        for y in range(N):
            coordinates.append((x, y))

    not_greedy_n = len(coordinates) % 3
    greedy_n = len(coordinates) - not_greedy_n

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
        if b == 0:
            has_oils.append(coordinates[i + 1])
        if c > 0:
            has_oils.append(coordinates[i + 2])

    for i in range(not_greedy_n):
        if query1(coordinates[-1 - i]) > 0:
            has_oils.append(coordinates[-1 - i])

    resp = answer(has_oils)
    return resp == "1"


def main():
    logger.info("abc")
    N, M, ESP, GRID = std_inputs()

    mus = []
    for vs in range(N**2 * M + 1):
        mu, sigma = calc_mu_sigma(2, ESP, vs)
        mus.append(mu)

    global mu_mids
    for i in range(len(mus) - 1):
        mu_mids.append((mus[i] + mus[i + 1]) / 2)

    if my_greedy(N, ESP):
        exit()
    else:
        greedy(N)


main()
