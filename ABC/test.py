import sys

input = sys.stdin.buffer.readline
in_n = lambda: int(input())
in_nn = lambda: map(int, input().split())
in_s = lambda: input().rstrip().decode('utf-8')
in_map = lambda: [s == ord('.') for s in input() if s != ord('\n')]

def create_array(shape, fill_value=0):
    if len(shape) == 1:
        return [fill_value] * shape[0]
    else:
        return [create_array(shape[1:], fill_value) for _ in range(shape[0])]

MOD = 10**9 + 7
INF = 9 * 10**18


def main():
    arr = create_array((3, 4), 0)


if __name__ == '__main__':
    main()
