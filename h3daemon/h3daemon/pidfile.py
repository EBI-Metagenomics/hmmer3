from deciphon_schema import HMMFile
from pidlockfile import PIDLockFile

__all__ = ["create_pidfile"]


def create_pidfile(hmmfile: HMMFile, timeout=5):
    return PIDLockFile(f"{str(hmmfile.path.absolute())}.pid", timeout=timeout)
