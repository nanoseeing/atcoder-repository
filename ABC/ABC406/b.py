# ================
# Main
# ================

# import itertools
# import heapq
# import bisect
# from collections import Counter
# from collections import defaultdict
# from collections import deque

# sys.setrecursionlimit(10**7)

MOD = 10**9 + 7
INF = 9 * 10**18


def main():
    N, K = in_nn()
    A = list(in_nn())

    ans = 1
    for i in range(N):
        ans *= A[i]
        if len(str(ans)) >= K + 1:
            ans = 1

    print(ans)


# ================
# Template
# ================
import pickle
import sys

ONLINE_JUDGE = "Main.py" in sys.argv

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
    if ONLINE_JUDGE:
        return

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


if __name__ == "__main__":
    main()
