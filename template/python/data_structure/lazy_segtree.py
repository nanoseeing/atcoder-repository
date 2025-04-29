import sys

INF = 8 * 10 ** 18
MOD = 998244353
MASK = 32


# ----- seg tree設定 ----- #
e = INF
id = 0


def op(a, b):
    return min(a, b)


def mapping(f, a):
    return a + f


def composition(f, g):
    return f + g
# ----- seg tree設定 ここまで ----- #


class LazySegTree:
    def __init__(self, op, e, mapping, composition, id, n: int = -1, v: list = []):
        assert (len(v) > 0) | (n > 0)
        if(len(v) == 0):
            v = [e] * n
        self.__n = len(v)
        self.__log = (self.__n - 1).bit_length()
        self.__size = 1 << self.__log
        self.__d = [e] * (2 * self.__size)
        self.__lz = [id] * self.__size
        self.__op = op
        self.__e = e
        self.__mapping = mapping
        self.__composition = composition
        self.__id = id

        for i in range(self.__n):
            self.__d[self.__size + i] = v[i]
        for i in range(self.__size - 1, 0, -1):
            self.__update(i)

    def __update(self, k: int):
        self.__d[k] = self.__op(self.__d[2 * k], self.__d[2 * k + 1])

    def __all_apply(self, k: int, f):
        self.__d[k] = self.__mapping(f, self.__d[k])
        if(k < self.__size):
            self.__lz[k] = self.__composition(f, self.__lz[k])

    def __push(self, k: int):
        self.__all_apply(2 * k, self.__lz[k])
        self.__all_apply(2 * k + 1, self.__lz[k])
        self.__lz[k] = self.__id

    def set(self, p: int, x):
        assert (0 <= p) & (p < self.__n)
        p += self.__size
        for i in range(self.__log, 0, -1):
            self.__push(p >> i)
        self.__d[p] = x
        for i in range(1, self.__log + 1):
            self.__update(p >> i)

    def get(self, p: int):
        assert (0 <= p) & (p < self.__n)
        p += self.__size
        for i in range(self.__log, 0, -1):
            self.__push(p >> i)
        return self.__d[p]

    def prod(self, l: int, r: int):
        assert (0 <= l) & (l <= r) & (r <= self.__n)
        if(l == r):
            return self.__e

        l += self.__size
        r += self.__size

        for i in range(self.__log, 0, -1):
            if((l >> i) << i) != l:
                self.__push(l >> i)
            if((r >> i) << i) != r:
                self.__push(r >> i)

        sml = self.__e
        smr = self.__e
        while(l < r):
            if(l & 1):
                sml = self.__op(sml, self.__d[l])
                l += 1
            if(r & 1):
                r -= 1
                smr = self.__op(self.__d[r], smr)
            l //= 2
            r //= 2

        return self.__op(sml, smr)

    def all_prod(self):
        return self.__d[1]

    def apply(self, p: int, f):
        assert (0 <= p) & (p < self.__n)
        p += self.__size
        for i in range(self.__log, 0, -1):
            self.__push(p >> i)
        self.__d[p] = self.__mapping(f, self.__d[p])
        for i in range(1, self.__log + 1):
            self.__update(p >> i)

    def apply_range(self, l: int, r: int, f):
        assert (0 <= l) & (l <= r) & (r <= self.__n)
        if(l == r):
            return

        l += self.__size
        r += self.__size

        for i in range(self.__log, 0, -1):
            if((l >> i) << i) != l:
                self.__push(l >> i)
            if((r >> i) << i) != r:
                self.__push((r - 1) >> i)

        l2, r2 = l, r
        while(l < r):
            if(l & 1):
                self.__all_apply(l, f)
                l += 1
            if(r & 1):
                r -= 1
                self.__all_apply(r, f)
            l //= 2
            r //= 2
        l, r = l2, r2

        for i in range(1, self.__log + 1):
            if((l >> i) << i) != l:
                self.__update(l >> i)
            if((r >> i) << i) != r:
                self.__update((r - 1) >> i)

    def max_right(self, l: int, g):
        assert (0 <= l) & (l <= self.__n)
        assert g(self.__e)
        if(l == self.__n):
            return self.__n
        l += self.__size
        for i in range(self.__log, 0, -1):
            self.__push(l >> i)
        sm = self.__e
        while(True):
            while(l % 2 == 0):
                l //= 2
            if(not g(self.__op(sm, self.__d[l]))):
                while(l < self.__size):
                    self.__push(l)
                    l *= 2
                    if(g(self.__op(sm, self.__d[l]))):
                        sm = self.__op(sm, self.__d[l])
                        l += 1
                return l - self.__size
            sm = self.__op(sm, self.__d[l])
            l += 1
            if(l & -l) == l:
                break
        return self.__n

    def min_left(self, r: int, g):
        assert (0 <= r) & (r <= self.__n)
        assert g(self.__e)
        if(r == 0):
            return 0
        r += self.__size
        for i in range(self.__log, 0, -1):
            self.__push((r - 1) >> i)
        sm = self.__e
        while(True):
            r -= 1
            while(r > 1) & (r % 2):
                r //= 2
            if(not g(self.__op(self.__d[r], sm))):
                while(r < self.__size):
                    self.__push(r)
                    r = 2 * r + 1
                    if(g(self.__op(self.__d[r], sm))):
                        sm = self.__op(self.__d[r], sm)
                        r -= 1
                return r + 1 - self.__size
            sm = self.__op(self.__d[r], sm)
            if(r & -r) == r:
                break
        return 0

    def all_push(self):
        for i in range(1, self.__size):
            self.__push(i)

    def get_all(self):
        self.all_push()
        return self.__d[self.__size:self.__size + self.__n]

    def print(self):
        print(list(map(lambda x: divmod(x, (1 << 30)), self.__d)))
        print(self.__lz)
        print('------------------')


# ----- 区間加算・区間最小 ----- #

e = INF
id = 0


def op(a, b):
    return min(a, b)


def mapping(f, a):
    return a + f


def composition(f, g):
    return f + g


# ----- 区間加算・区間和 ----- #

e = 0
id = 0


def op(a, b):
    a1, a2 = divmod(a, 1 << MASK)
    b1, b2 = divmod(b, 1 << MASK)
    c1 = (a1 + b1)
    c2 = (a2 + b2)
    return (c1 << MASK) + c2


def mapping(f, a):
    a1, a2 = divmod(a, 1 << MASK)
    c1 = f * a2
    c2 = a2
    return (c1 << MASK) + c2


def composition(f, g):
    return f + g


# ----- 区間変更・区間最小 ----- #

e = INF
id = 8 * 10**18


def op(a, b):
    return min(a, b)


def mapping(f, a):
    if f == id:
        return a
    else:
        return f


def composition(f, g):
    if f == id:
        return g
    else:
        return f


# ----- 区間変更・区間和 ----- #

e = 0
id = 8 * 10**18


def op(a, b):
    a1, a2 = divmod(a, 1 << MASK)
    b1, b2 = divmod(b, 1 << MASK)
    c1 = (a1 + b1)
    c2 = (a2 + b2)
    return (c1 << MASK) + c2


def mapping(f, a):
    if f == id:
        return a
    else:
        a1, a2 = divmod(a, 1 << MASK)
        c1 = f * a2
        c2 = a2
        return (c1 << MASK) + c2


def composition(f, g):
    if f == id:
        return g
    else:
        return f


# ----- テスト ----- #

def main():
    N, Q = map(int, input().split())
    A = [(a << MASK) + 1 for a in map(int, input().split())]
    lst = LazySegTree(N, op, e, mapping, composition, id, v=A)

    res = []

    for _ in range(Q):
        t, *q = tuple(map(int, input().split()))
        if t == 0:
            l, r, b, c = q
            lst.range_apply(l, r, (b << MASK) + c)
        else:
            l, r = q
            res.append(lst.prod(l, r) >> MASK)

    print('\n'.join(map(str, res)))


if __name__ == '__main__':
    main()
