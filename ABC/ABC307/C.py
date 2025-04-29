import sys

input = sys.stdin.buffer.readline
in_n = lambda: int(input())
in_nn = lambda: map(int, input().split())
in_s = lambda: input().rstrip().decode("utf-8")
in_map = lambda: [s == ord(".") for s in input() if s != ord("\n")]

MOD = 10**9 + 7
INF = 8 * 10**18


def main():
    HA, WA = in_nn()
    sheet_A = [list(in_s()) for _ in range(HA)]
    HB, WB = in_nn()
    sheet_B = [list(in_s()) for _ in range(HB)]
    HX, WX = in_nn()
    sheet_X = [list(in_s()) for _ in range(HX)]

    A_count = 0
    for ha in range(HA):
        for wa in range(WA):
            if sheet_A[ha][wa] == "#":
                A_count += 1
    B_count = 0
    for hb in range(HB):
        for wb in range(WB):
            if sheet_B[hb][wb] == "#":
                B_count += 1

    for ha in range(-max(HA, HX), HX + HA + 1):
        for wa in range(-max(WA, WX), WX + WA + 1):
            for hb in range(-max(HB, HX), HX + HB + 1):
                for wb in range(-max(WB, WX), WX + WB + 1):
                    a_count = 0
                    b_count = 0
                    grid = [["."] * WX for _ in range(HX)]
                    for hx in range(HX):
                        for wx in range(WX):
                            if hb + hx < 0 or hb + hx >= HB or wb + wx < 0 or wb + wx >= WB:
                                continue
                            elif sheet_B[hb + hx][wb + wx] == "#":
                                grid[hx][wx] = "#"
                                b_count += 1
                    if b_count != B_count:
                        continue
                    for hx in range(HX):
                        for wx in range(WX):
                            if hx + ha < 0 or hx + ha >= HA or wx + wa < 0 or wx + wa >= WA:
                                continue
                            elif sheet_A[ha + hx][wa + wx] == "#":
                                grid[hx][wx] = "#"
                                a_count += 1
                    if a_count != A_count:
                        continue
                    all_ok = True
                    for hx in range(HX):
                        for wx in range(WX):
                            if grid[hx][wx] != sheet_X[hx][wx]:
                                all_ok = False
                    if all_ok:
                        print("Yes")
                        return
    print("No")


if __name__ == "__main__":
    main()
