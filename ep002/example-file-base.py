import time

N = 10


def work():
    for i in range(N):
        # do work
        time.sleep(0.5)

        with open("example-file.txt", "a") as f:
            f.write(f"{i}\n")
            print("Wrote to file:", i)

    print("Work done")


if __name__ == "__main__":
    work()
