import io
import os
import sys
from contextlib import contextmanager


@contextmanager
def fixstreams():
    oldstdin = sys.stdin
    old__stdin__ = sys.__stdin__

    oldstdout = sys.stdout
    old__stdout__ = sys.__stdout__

    oldstderr = sys.stderr
    old__stderr__ = sys.__stderr__

    if not working_fileno(sys.stdin):
        devnull = open(os.devnull, "r")
        sys.stdin = sys.__stdin__ = devnull
        try:
            os.stat(0)
        except OSError:
            os.dup2(devnull.fileno(), 0)

    if not working_fileno(sys.stdout):
        devnull = open(os.devnull, "w+")
        sys.stdout = sys.__stdout__ = devnull
        try:
            os.stat(1)
        except OSError:
            os.dup2(devnull.fileno(), 1)

    if not working_fileno(sys.stderr):
        devnull = open(os.devnull, "w+")
        sys.stderr = sys.__stderr__ = devnull
        try:
            os.stat(2)
        except OSError:
            os.dup2(devnull.fileno(), 2)

    try:
        yield
    finally:
        sys.stdin = oldstdin
        sys.__stdin__ = old__stdin__

        sys.stdout = oldstdout
        sys.__stdout__ = old__stdout__

        sys.stderr = oldstderr
        sys.__stderr__ = old__stderr__


def working_fileno(stream):
    if not hasattr(stream, "fileno"):
        return False
    try:
        stream.fileno()
    except io.UnsupportedOperation:
        return False
    return True
