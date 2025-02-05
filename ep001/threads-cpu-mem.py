import concurrent.futures

from utils import time_it

counter = 0


@time_it("work")
def work():
    global counter

    for _ in range(10_000_000):
        counter += 1


@time_it("-- main")
def main():
    with concurrent.futures.ThreadPoolExecutor(2) as pool:
        futures = [pool.submit(work) for _ in range(10)]
        concurrent.futures.wait(futures)

    return counter


if __name__ == "__main__":
    c = main()
    print("Total: ", c)
