""" 組み合わせ """


# 組み合わせ
def comb(n, k):

    k = min(k, n - k)

    m = 1
    for i in range(n, n - k, -1):
        m *= i

    n = 1
    for i in range(1, k + 1):
        n *= i

    return m // n


# 順列
def perm(n, k):

    ret = 1
    for i in range(n, n - k, -1):
        ret *= i

    return ret


""" mod下での計算 """


# 割り算 (1/x)
pow(x, mod - 2, mod)


# 順列
def perm_mod(n, k, mod=10**9 + 7):

    ret = 1
    for i in range(n, n - k, -1):
        ret *= i
        ret %= mod

    return ret


# 組み合わせ(mod)
def comb_mod(n, k, mod):

    k = min(k, n - k)

    m = 1
    for i in range(n, n - k, -1):
        m = m * i % mod

    n = 1
    for i in range(1, k + 1):
        n = n * i % mod

    return (m * pow(n, mod - 2, mod)) % mod


# クラス
class CombMod:

    def __init__(self, N, MOD=10**9 + 7):

        N = N + 1
        inv = [0] * N
        fact = [0] * N
        fact_inv = [0] * N

        inv[0] = 0
        inv[1] = 1
        for n in range(2, N):
            q, r = divmod(MOD, n)
            inv[n] = inv[r] * (-q) % MOD

        fact[0] = 1
        for n in range(1, N):
            fact[n] = n * fact[n - 1] % MOD

        fact_inv[0] = 1
        for n in range(1, N):
            fact_inv[n] = fact_inv[n - 1] * inv[n] % MOD

        self.fact = fact
        self.fact_inv = fact_inv
        self.inv = inv

    def comb(self, n, r, mod=10**9 + 7):
        return self.fact[n] * self.fact_inv[r] % mod * self.fact_inv[n - r] % mod

    def perm(self, n, r, mod=10**9 + 7):
        return self.fact[n] * self.fact_inv[n - r] % mod


# 組み合わせ(numpy)
def fact_table(N, MOD=10**9 + 7):

    N = N + 1
    inv = np.empty(N, np.int64)
    fact = np.empty(N, np.int64)
    fact_inv = np.empty(N, np.int64)

    inv[0] = 0
    inv[1] = 1
    for n in range(2, N):
        q, r = divmod(MOD, n)
        inv[n] = inv[r] * (-q) % MOD

    fact[0] = 1
    for n in range(1, N):
        fact[n] = n * fact[n - 1] % MOD

    fact_inv[0] = 1
    for n in range(1, N):
        fact_inv[n] = fact_inv[n - 1] * inv[n] % MOD

    return fact, fact_inv, inv
