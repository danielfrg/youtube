import time
import concurrent.futures

from utils import time_it


@time_it("work")
def do_work():
    time.sleep(1)


@time_it("main")
def main():
    with concurrent.futures.ThreadPoolExecutor(3) as pool:
        futures = [pool.submit(do_work) for _ in range(10)]
        concurrent.futures.wait(futures)


if __name__ == "__main__":
    main()
