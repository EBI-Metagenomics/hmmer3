from pidlockfile import PIDLockFile

from h3daemon.daemon import Daemon

__all__ = ["possess"]


def possess(pidfile: PIDLockFile):
    return Daemon.possess(pidfile)
