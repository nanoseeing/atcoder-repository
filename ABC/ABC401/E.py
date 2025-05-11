# ================
# Template
# ================
import pickle
import sys

input = sys.stdin.buffer.readline
in_n = lambda: int(input())
in_nn = lambda: map(int, input().split())
in_s = lambda: input().rstrip().decode("utf-8")
in_map = lambda: [s == ord(".") for s in input() if s != ord("\n")]


def fast_copy(obj):
    return pickle.loads(pickle.dumps(obj, -1))


def create_array(shape: tuple, fill_value):
    if len(shape) == 1:
        return [fill_value() if callable(fill_value) else fill_value for _ in range(shape[0])]
    else:
        return [create_array(shape[1:], fill_value) for _ in range(shape[0])]


def debug(*args):
    import ast
    import inspect
    import textwrap

    CYAN = "\033[96m"
    RESET = "\033[0m"

    frame = inspect.currentframe().f_back
    code_line = inspect.getframeinfo(frame).code_context[0]
    code_line = textwrap.dedent(code_line).strip()

    try:
        root = ast.parse(code_line)
    except SyntaxError as e:
        print(f"{CYAN}SyntaxError while parsing code line: {e}{RESET}", file=sys.stderr)
        return

    call_node = root.body[0].value
    if not isinstance(call_node, ast.Call):
        print(f"{CYAN}debug_print() not found in the code line.{RESET}", file=sys.stderr)
        return

    arg_names = []
    for arg in call_node.args:
        arg_str = ast.get_source_segment(code_line, arg)
        arg_names.append(arg_str)

    output = []
    for name, val in zip(arg_names, args):
        output.append(f"{name} = {val!r}")

    print(f"{CYAN}" + ", ".join(output) + f"{RESET}", file=sys.stderr)


# ================
# Main
# ================

# import itertools
import heapq

# import bisect
# from collections import Counter
# from collections import defaultdict
# from collections import deque

# sys.setrecursionlimit(10**7)

MOD = 10**9 + 7
INF = 9 * 10**18


def main():
    N, M = in_nn()
    edges = create_array((N,), list)
    for _ in range(M):
        a, b = in_nn()
        edges[a - 1].append(b - 1)
        edges[b - 1].append(a - 1)

    # 1から始める
    # heapに頂点番号をいれる
    # 小さいやつ取り出す
    # 取り出した頂点からいける頂点で、頂点集合に含まれないものを追加
    # 取り出した頂点は削除しておくこと。
    # 現在の頂点集合の最大値 == 追加回数ならOK
    #    そのとき、いける頂点のsetの個数
    ans = []
    q = [0]
    heapq.heapify(q)
    visited = set([0])
    non_visited = set()
    max_visited = 0

    for cnt in range(N):
        if q:
            v = heapq.heappop(q)
            # debug(v, q)
            # debug(v)
            visited.add(v)
            for to in edges[v]:
                # debug(v, to)
                if to not in visited:
                    if to not in non_visited:
                        non_visited.add(to)
                        heapq.heappush(q, to)
            non_visited.discard(v)
            max_visited = max(max_visited, v)
        if cnt == max_visited and cnt == len(visited) - 1:
            ans.append(len(non_visited))
        else:
            ans.append(-1)

    print(*ans, sep="\n")


if __name__ == "__main__":
    main()
