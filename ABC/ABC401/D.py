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
    S = list(in_s())

    o_cnt = 0
    for i in range(N):
        if S[i] == "o":
            if i - 1 >= 0:
                S[i - 1] = "."
            if i + 1 < N:
                S[i + 1] = "."
            o_cnt += 1
    if o_cnt == K:
        for i in range(N):
            if S[i] == "?":
                S[i] = "."
        print("".join(S))
        return

    question_inds = []
    question_ind = -1
    for i in range(N):
        if S[i] == "?":
            if question_ind == -1:
                question_ind = i
        else:
            if question_ind != -1:
                question_inds.append((question_ind, i))
                question_ind = -1
    else:
        if question_ind != -1:
            question_inds.append((question_ind, N))

    remain_o = K - o_cnt
    buffer = sum((next_ind - question_ind + 1) // 2 for question_ind, next_ind in question_inds)

    if buffer > remain_o:
        print("".join(S))
    else:
        for question_ind, next_ind in question_inds:
            t = next_ind - question_ind
            if t % 2 == 1:
                for j in range(question_ind, next_ind):
                    if (j - question_ind) % 2 == 0:
                        S[j] = "o"
                    else:
                        S[j] = "."

        print("".join(S))


if __name__ == "__main__":
    main()
