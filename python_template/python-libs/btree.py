import bisect


class BTreeNode:
    def __init__(self):
        self.key = []
        self.child = []


class BTree:
    def __init__(self):
        self.root = BTreeNode()

    def search_higher(self, key):
        ptr = self.root
        ret = None
        while ptr.child:
            i = bisect.bisect_right(ptr.key, key)
            if i != len(ptr.key):
                ret = ptr.key[i]
            ptr = ptr.child[i]
        i = bisect.bisect_right(ptr.key, key)
        if i != len(ptr.key):
            ret = ptr.key[i]
        return ret

    def search_lower(self, key):
        ptr = self.root
        ret = None
        while ptr.child:
            i = bisect.bisect_left(ptr.key, key)
            if i != 0:
                ret = ptr.key[i - 1]
            ptr = ptr.child[i]
        i = bisect.bisect_left(ptr.key, key)
        if i != 0:
            ret = ptr.key[i - 1]
        return ret

    def insert(self, key):
        def insert_rec(ptr):
            b_size = 512
            if not ptr.child:
                bisect.insort(ptr.key, key)
                if len(ptr.key) == b_size * 2 - 1:
                    ret = BTreeNode()
                    ret.key = ptr.key[:b_size]
                    ptr.key = ptr.key[b_size:]
                    return ret
            else:
                i = bisect.bisect(ptr.key, key)
                temp = insert_rec(ptr.child[i])
                if temp is not None:
                    ptr.key.insert(i, temp.key.pop(-1))
                    ptr.child.insert(i, temp)
                    if len(ptr.child) == b_size * 2:
                        ret = BTreeNode()
                        ret.child = ptr.child[:b_size]
                        ptr.child = ptr.child[b_size:]
                        ret.key = ptr.key[:b_size]
                        ptr.key = ptr.key[b_size:]
                        return ret
            return None
        temp = insert_rec(self.root)
        if temp is not None:
            root = BTreeNode()
            root.key = [temp.key.pop(-1)]
            root.child = [temp, self.root]
            self.root = root

    def dump(self):
        def dump_rec(ptr, dep):
            for _ in range(0, dep):
                print("  ", end="")
            print(ptr.key)
            for c in ptr.child:
                dump_rec(c, dep + 1)
        dump_rec(self.root, 0)
        print("")


def main():

    a = [1, 3, 5, 2, 6, 9, 11, 15, 13]

    bt = BTree()
    for i in range(len(a)):
        bt.insert(a[i])

    print(bt.search_higher(2))
    print(bt.search_lower)


if __name__ == '__main__':
    main()
