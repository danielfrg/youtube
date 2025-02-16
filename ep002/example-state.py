import time
from dataclasses import dataclass

from checkpoint import checkpoint, Checkpoint


@dataclass
class State:
    count: int = 0


N = 10


@checkpoint(State)
def work(state: State, ckpt: Checkpoint):
    if ckpt.status == "done":
        print("Work already done")
        return state

    print("Starting work from", ckpt.start_from)
    for i in range(ckpt.start_from, N):
        # do work
        time.sleep(0.5)

        state.count = state.count + 1
        print("Processed:", i)
        yield

        if i == 4:
            raise Exception("Error while processing")

    return state


if __name__ == "__main__":
    value = work()  # type: ignore
    print("Final value:", value)
