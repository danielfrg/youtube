import time
from contextlib import contextmanager


@contextmanager
def time_it(id):
    start_time = time.time()
    yield
    elapsed_time = time.time() - start_time
    print(f"{id} took: {elapsed_time:.6f} s")
