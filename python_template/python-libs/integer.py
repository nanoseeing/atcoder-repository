""" 整数関連（素数・約数など） """

from collections import defaultdict


# 約数列挙 0(√N)
def make_divisors(n):
    lower_divisors, upper_divisors = [], []
    i = 1
    while i * i <= n:
        if n % i == 0:
            lower_divisors.append(i)
            if i != n // i:
                upper_divisors.append(n // i)
        i += 1
    return lower_divisors + upper_divisors[::-1]


# 素数列挙（エラトステネスの篩）O(NloglogN)
def primes(n):
    is_prime = [True] * (n + 1)
    is_prime[0] = False
    is_prime[1] = False
    for i in range(2, int(n**0.5) + 1):
        if not is_prime[i]:
            continue
        for j in range(i * 2, n + 1, i):
            is_prime[j] = False
    return [i for i in range(n + 1) if is_prime[i]]


# 素数判定
def is_prime(n):
    if n == 1:
        return False

    for k in range(2, int(n**0.5 + 1)):
        if n % k == 0:
            return False

    return True


# 素因数分解
def factorize(n):
    b = 2
    fct = defaultdict(lambda: 0)
    while b ** 2 <= n:
        while n % b == 0:
            n //= b
            fct[b] += 1
        b += 1
    if n > 1:
        fct[n] += 1
    return fct


# 素因数分解（複数回）
class Factorize(object):

    def __init__(self, maxnum):
        self.primes = self.__smallest_prime_factors(maxnum)

    # 素因数分解 O(logN)
    def factorize(self, n):
        fct = defaultdict(lambda: 0)
        while n != 1:
            fct[self.primes[n]] += 1
            n //= self.primes[n]
        return fct

    # n以下の最小の素因数を列挙する O(NlogN)
    def __smallest_prime_factors(self, n):

        prime = list(range(n + 1))

        for i in range(2, int(n**0.5) + 1):
            if prime[i] != i:
                continue
            for j in range(i * 2, n + 1, i):
                prime[j] = min(prime[j], i)
        return prime
