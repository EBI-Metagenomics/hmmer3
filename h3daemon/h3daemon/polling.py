import functools

from tenacity import Retrying, retry, stop_after_delay, wait_exponential

STOP = stop_after_delay(10)
WAIT = wait_exponential()


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
