from pathlib import Path

from pidlockfile import PIDLockFile

__all__ = ["create_pidfile"]


def create_pidfile(file: Path, timeout=5):
    return PIDLockFile(f"{str(file.absolute())}.pid", timeout=timeout)
