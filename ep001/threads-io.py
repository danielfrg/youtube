import time
import concurrent.futures

from utils import time_it


@time_it("work")
def do_work() -> int:
    time.sleep(1)


def main():
    with time_it("main"):
        with concurrent.futures.ThreadPoolExecutor(3) as pool:
            futures = [pool.submit(do_work) for _ in range(10)]
            concurrent.futures.wait(futures)

    return 0


if __name__ == "__main__":
    main()
