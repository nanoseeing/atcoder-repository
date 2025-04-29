import sys
from collections import defaultdict, deque
from dataclasses import dataclass

input = sys.stdin.buffer.readline
in_n = lambda: int(input())
in_nn = lambda: map(int, input().split())
in_s = lambda: input().rstrip().decode("utf-8")
in_map = lambda: [s == ord(".") for s in input() if s != ord("\n")]

MOD = 10**9 + 7
INF = 8 * 10**18


@dataclass
class Node:
    id: int
    next: dict[str, int]
    check: bool = False


class TrieTree:
    def __init__(self):
        self.root = Node(0, dict(), False)
        self.nodes = [self.root]

        self.remain_ids = defaultdict(int)
        self.score = 0

    def add_prefix(self, s):
        now_node = self.root
        for c in s:
            if c not in now_node.next:
                next_node = Node(len(self.nodes), dict(), False)
                self.nodes.append(next_node)
                self.nodes[now_node.id].next[c] = next_node.id
            else:
                next_node = self.nodes[now_node.next[c]]
            if next_node.check:
                return
            now_node = next_node

        q = deque([now_node])
        while q:
            next_node = q.popleft()
            if next_node.check:
                continue
            self.nodes[next_node.id].check = True
            if next_node.id in self.remain_ids:
                self.score -= self.remain_ids[next_node.id]
                self.remain_ids[next_node.id] = 0
            for next_id in next_node.next.values():
                q.append(self.nodes[next_id])

    def add_word(self, s: str):
        now_node = self.root
        is_check = False
        for c in s:
            if c not in now_node.next:
                next_node = Node(len(self.nodes), dict(), False)
                self.nodes.append(next_node)
                self.nodes[now_node.id].next[c] = next_node.id
            else:
                next_node = self.nodes[now_node.next[c]]
            is_check = is_check or next_node.check
            if is_check:
                self.nodes[next_node.id].check = True
            now_node = next_node

        if not is_check:
            self.score += 1
            self.remain_ids[now_node.id] += 1


def main():
    Q = in_n()
    trie = TrieTree()

    ans = []
    for _ in range(Q):
        t, s = in_s().split()
        t = int(t)
        if t == 1:
            trie.add_prefix(s)
        else:
            trie.add_word(s)
        ans.append(trie.score)

    print("\n".join(map(str, ans)))


if __name__ == "__main__":
    main()
