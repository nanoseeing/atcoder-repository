""" 幾何学系 """


# 線分交差判定（点と線分も可）
def is_intersection(P0, P1, Q0, Q1):

    def dot3(O, A, B):
        ox, oy = O
        ax, ay = A
        bx, by = B
        return (ax - ox) * (bx - ox) + (ay - oy) * (by - oy)

    def cross3(O, A, B):
        ox, oy = O
        ax, ay = A
        bx, by = B
        return (ax - ox) * (by - oy) - (bx - ox) * (ay - oy)

    def dist2(A, B):
        ax, ay = A
        bx, by = B
        return (ax - bx) ** 2 + (ay - by) ** 2

    C0 = cross3(P0, P1, Q0)
    C1 = cross3(P0, P1, Q1)
    D0 = cross3(Q0, Q1, P0)
    D1 = cross3(Q0, Q1, P1)
    if C0 == C1 == 0:
        E0 = dot3(P0, P1, Q0)
        E1 = dot3(P0, P1, Q1)
        if not E0 < E1:
            E0, E1 = E1, E0
        return E0 <= dist2(P0, P1) and 0 <= E1
    return C0 * C1 <= 0 and D0 * D1 <= 0
