import threading
import concurrent.futures

from utils import time_it


counter = 0
lock = threading.Lock()


@time_it("work")
def do_work():
    global counter

    x = 0
    for _ in range(1_000_000):
        x += 1

    with lock:
        counter += x


@time_it("main")
def main():
    with concurrent.futures.ThreadPoolExecutor(5) as pool:
        futures = [pool.submit(do_work) for _ in range(10)]
        concurrent.futures.wait(futures)

    return counter


if __name__ == "__main__":
    c = main()
    print("Total: ", c)
