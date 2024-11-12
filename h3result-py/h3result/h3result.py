import os

from h3result._cffi import ffi, lib
from h3result.error import H3ResultError

__all__ = ["H3Result"]


class H3Result:
    def __init__(self, fd: int):
        self._cdata = ffi.NULL

        self._cdata = lib.h3r_new()
        if self._cdata == ffi.NULL:
            raise MemoryError()

        rc = lib.h3r_unpack(self._cdata, fd)
        if rc != 0:
            raise H3ResultError(rc)

    def print_targets(self, fileno: int):
        fd = os.dup(fileno)
        lib.h3r_print_targets(self._cdata, fd)
        os.close(fd)

    def print_domains(self, fileno: int):
        fd = os.dup(fileno)
        lib.h3r_print_domains(self._cdata, fd)
        os.close(fd)

    def print_targets_table(self, fileno: int):
        fd = os.dup(fileno)
        lib.h3r_print_targets_table(self._cdata, fd)
        os.close(fd)

    def print_domains_table(self, fileno: int):
        fd = os.dup(fileno)
        lib.h3r_print_domains_table(self._cdata, fd)
        os.close(fd)

    def __del__(self):
        if getattr(self, "_cdata", ffi.NULL) != ffi.NULL:
            lib.h3r_del(self._cdata)
