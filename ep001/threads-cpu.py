import concurrent.futures

from utils import time_it


@time_it("work")
def do_work() -> int:
    x = 0
    for _ in range(10_000_000):
        x += 1


def main():
    with time_it("main"):
        with concurrent.futures.ThreadPoolExecutor(4) as pool:
            futures = [pool.submit(do_work) for _ in range(10)]
            concurrent.futures.wait(futures)

    return 0


if __name__ == "__main__":
    main()
