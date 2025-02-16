import time

from tenacity import retry

from checkpoint import checkpoint, Checkpoint

N = 10


@retry
@checkpoint(output="example-file.txt")
def work(ckpt: Checkpoint):
    for i in range(ckpt.start_from, N):
        # do work
        time.sleep(0.5)

        yield i
        print("Wrote to file:", i)

        if i == 4:
            raise Exception("Error while processing")

    print("Work done")


if __name__ == "__main__":
    work()  # type: ignore
