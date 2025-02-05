import threading
import concurrent.futures

from utils import time_it


counter = 0
lock = threading.Lock()


@time_it("work")
def work():
    global counter

    x = 0
    for _ in range(10_000_000):
        x += 1

    with lock:
        counter += x


@time_it("-- main")
def main():
    with concurrent.futures.ThreadPoolExecutor(2) as pool:
        futures = [pool.submit(work) for _ in range(10)]
        concurrent.futures.wait(futures)

    return counter


if __name__ == "__main__":
    c = main()
    print("Total: ", c)
