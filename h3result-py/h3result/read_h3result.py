import os
from pathlib import Path

from h3result.cffi import ffi, lib
from h3result.h3result import H3Result
from h3result.path_like import PathLike

__all__ = ["read_h3result"]


def read_h3result(filename: PathLike | None = None, fileno: int | None = None):
    if filename is not None:
        assert fileno is None
    if fileno is not None:
        assert filename is None

    if filename:
        fp = lib.fopen(bytes(Path(filename)), b"rb")
    else:
        fp = lib.fdopen(os.dup(fileno), b"rb")

    if fp == ffi.NULL:
        raise RuntimeError()

    try:
        x = H3Result(fp)
    finally:
        lib.fclose(fp)

    return x
