from heapq import heappush, heappop
from collections import defaultdict


class HeapSet:
    def __init__(self):
        self.minQue = []
        self.maxQue = []
        self.counter = defaultdict(lambda: 0)

    def insert(self, x):
        heappush(self.minQue, x)
        heappush(self.maxQue, -x)
        self.counter[x] += 1

    def delete(self, x):
        self.counter[x] = max(0, self.counter[x] - 1)

    def get_max(self):
        while self.maxQue and self.counter[-self.maxQue[0]] == 0:
            heappop(self.maxQue)
        return -self.maxQue[0] if self.maxQue else None

    def get_min(self):
        while self.minQue and self.counter[self.minQue[0]] == 0:
            heappop(self.minQue)
        return self.minQue[0] if self.minQue else None

    def __str__(self):

        min = []
        sl = sorted(list(set(self.minQue)))
        for i in range(len(sl)):
            if self.counter[sl[i]]:
                min += [sl[i]] * self.counter[sl[i]]

        return min.__str__()
