import time
from typing import Callable

__all__ = ["wait_until"]


def wait_until(ok: Callable[[], bool], n=50):
    for i in range(n):
        if ok():
            return
        time.sleep(0.1 + (i / n) * 3.0)
    raise TimeoutError()
