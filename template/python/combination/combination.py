import itertools

# 順列
list(itertools.permutations([1, 2, 3, 4], 2))
# 組み合わせ
list(itertools.combinations([1, 2, 3, 4], 2))
# 重複順列（bit全探索）
list(itertools.product([0, 1], repeat=10))
# 重複組み合わせ
list(itertools.combinations_with_replacement(nums, 2))


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


def divmod(a, b, mod):
    """a // b % mod"""
    return (a * pow(b, mod - 2, mod)) % mod


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
