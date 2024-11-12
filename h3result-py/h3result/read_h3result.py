import os
from pathlib import Path

from h3result._cffi import ffi
from h3result.h3result import H3Result
from h3result.path_like import PathLike

__all__ = ["read_h3result"]


def read_h3result(filename: PathLike | None = None, fileno: int | None = None):
    if filename is not None:
        assert fileno is None
    if fileno is not None:
        assert filename is None

    if filename:
        fd = os.open(bytes(Path(str(filename))), os.O_RDONLY)
    else:
        assert fileno is not None
        fd = os.dup(fileno)

    if fd == ffi.NULL:
        raise RuntimeError()

    try:
        x = H3Result(fd)
    finally:
        os.close(fd)

    return x
