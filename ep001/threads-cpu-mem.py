import concurrent.futures
from utils import time_it

counter = 0


@time_it("work")
def do_work() -> int:
    global counter

    for _ in range(1_000_000):
        counter += 1


def main():
    with time_it("main"):
        with concurrent.futures.ThreadPoolExecutor(5) as pool:
            futures = [pool.submit(do_work) for _ in range(10)]
            concurrent.futures.wait(futures)

    return counter


if __name__ == "__main__":
    c = main()
    print("Total: ", c)
