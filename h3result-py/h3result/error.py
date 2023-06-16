from h3result.cffi import ffi, lib

__all__ = ["H3ResultError"]


class H3ResultError(RuntimeError):
    def __init__(self, errno: int):
        msg = ffi.string(lib.h3r_strerror(errno)).decode()
        super().__init__(f"h3r error: {msg}")
