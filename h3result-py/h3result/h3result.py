import os
from pathlib import Path

from h3result.cffi import ffi, lib
from h3result.error import H3ResultError
from h3result.path_like import PathLike

__all__ = ["H3Result"]


class H3Result:
    def __init__(self, filename: PathLike):
        self._cdata = ffi.NULL

        self._cdata = lib.h3result_new()
        if self._cdata == ffi.NULL:
            raise MemoryError()

        filename = Path(filename)

        fp = lib.fopen(bytes(filename), b"rb")
        if fp == ffi.NULL:
            raise RuntimeError()

        rc = lib.h3result_unpack(self._cdata, fp)
        lib.fclose(fp)
        if rc != 0:
            raise H3ResultError(rc)

    def print_targets(self, fileno: int):
        fd = os.dup(fileno)
        fp = lib.fdopen(fd, b"w")
        lib.h3result_print_targets(self._cdata, fp)
        lib.fclose(fp)

    def print_domains(self, fileno: int):
        fd = os.dup(fileno)
        fp = lib.fdopen(fd, b"w")
        lib.h3result_print_domains(self._cdata, fp)
        lib.fclose(fp)

    def print_targets_table(self, fileno: int):
        fd = os.dup(fileno)
        fp = lib.fdopen(fd, b"w")
        lib.h3result_print_targets_table(self._cdata, fp)
        lib.fclose(fp)

    def print_domains_table(self, fileno: int):
        fd = os.dup(fileno)
        fp = lib.fdopen(fd, b"w")
        lib.h3result_print_domains_table(self._cdata, fp)
        lib.fclose(fp)

    def __del__(self):
        if getattr(self, "_cdata", ffi.NULL) != ffi.NULL:
            lib.h3result_del(self._cdata)
