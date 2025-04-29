from collections import deque
from dataclasses import dataclass


@dataclass
class Node:
    id: int
    next: dict[str, int]


class TrieTree:
    def __init__(self):
        self.root = Node(0, dict(), False)
        self.nodes = [self.root]

    def add_word(self, s):
        now_node = self.root
        for c in s:
            if c not in now_node.next:
                next_node = Node(len(self.nodes), dict(), False)
                self.nodes.append(next_node)
                self.nodes[now_node.id].next[c] = next_node.id
            else:
                next_node = self.nodes[now_node.next[c]]
            now_node = next_node

        q = deque([now_node])
        while q:
            next_node = q.popleft()
            self.nodes[next_node.id].check = True
            for next_id in next_node.next.values():
                q.append(self.nodes[next_id])
