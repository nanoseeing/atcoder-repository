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


def main():
    N = in_n()
    A = list(in_nn())

    s = 0
    for i in range(N - 1):
        if A[i] == A[i + 1]:
            s = i + 1
        else:
            break

    if s == N - 1:
        print(0)
        return

    is_upper = A[s] < A[s + 1]
    if is_upper:
        inds = [s]
    else:
        inds = []

    for i in range(s, N - 2):
        if is_upper:
            if A[i] < A[i + 1] and A[i + 1] > A[i + 2]:
                inds.append(i + 1)
                is_upper = False
        else:
            if A[i] > A[i + 1] and A[i + 1] < A[i + 2]:
                inds.append(i + 1)
                is_upper = True
    inds.append(N - 1)

    ans = 0
    for i in range(0, len(inds) - 3, 2):
        x = inds[i + 1] - inds[i]
        y = inds[i + 3] - inds[i + 2]
        ans += x * y

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
