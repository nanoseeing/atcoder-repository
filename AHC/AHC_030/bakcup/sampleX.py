# read prior information
def query1(x, y):
    print("q 1 {} {}".format(x, y))
    return input()


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


def greedy(N, M):
    # drill every square
    has_oil = []
    for i in range(N):
        for j in range(N):
            resp = query1(i, j)
            if resp != "0":
                has_oil.append((i, j))

    print(
        "a {} {}".format(
            len(has_oil), " ".join(map(lambda x: "{} {}".format(x[0], x[1]), has_oil))
        )
    )
    resp = input()
    assert resp == "1"


def main():
    N, M, ESP, GRID = std_inputs()
    greedy(N, M)
