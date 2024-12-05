import functools

from tenacity import Retrying, retry, stop_after_delay, wait_fixed, wait_random

STOP = stop_after_delay(10)
WAIT = wait_fixed(0.1) + wait_random(0, 0.1)


def polling(func):
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        cb = retry(
            func,
            stop=STOP,
            wait=WAIT,
        )
        return cb(*args, **kwargs)

    return wrapper


def Polling():
    return Retrying(stop=STOP, wait=WAIT)
