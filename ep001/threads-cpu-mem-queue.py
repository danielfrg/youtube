import queue
import threading
import concurrent.futures

from utils import time_it

counter = 0
q = queue.Queue()


@time_it("work")
def do_work() -> int:
    x = 0
    for _ in range(1_000_000):
        x += 1

    q.put(x)


def update_counter():
    global counter
    while True:
        value = q.get()
        if value is None:
            break
        counter += value
        q.task_done()


def main():
    with time_it("main"):
        # Start a thread to consume from the queue and update the counter
        consumer_thread = threading.Thread(target=update_counter)
        consumer_thread.start()

        with concurrent.futures.ThreadPoolExecutor(5) as pool:
            futures = [pool.submit(do_work) for _ in range(10)]
            concurrent.futures.wait(futures)

        # Signal the consumer thread to stop
        q.put(None)

        # Wait for the consumer thread to finish
        consumer_thread.join()

    return counter


if __name__ == "__main__":
    c = main()
    print("Total: ", c)
