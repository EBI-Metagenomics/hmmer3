import os
import traceback

__all__ = ["debug_exception", "debug_message"]

_should_debug: bool | None = None
_debug_file: str | None = None


def should_debug():
    global _should_debug
    if _should_debug is None:
        _should_debug = bool(os.environ.get("H3DAEMON_DEBUG", 0))
    return _should_debug


def debug_file():
    global _debug_file
    if _debug_file is None:
        _debug_file = os.environ.get("H3DAEMON_DEBUG_FILE", "h3daemon_debug.txt")
    return _debug_file


def debug_exception(exception: Exception):
    if should_debug():
        with open(debug_file(), "a") as f:
            f.write(f"{type(exception)}\n")
            f.write(f"{exception.args}\n")
            f.write(traceback.format_exc())


def debug_message(msg: str):
    if should_debug():
        with open(debug_file(), "a") as f:
            f.write(f"{msg}\n")
