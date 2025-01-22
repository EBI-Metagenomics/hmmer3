from pidlockfile import PIDLockFile

from h3daemon.daemon import Daemon

__all__ = ["possess"]


def possess(pidfile: PIDLockFile, wait=True):
    if wait:
        return Daemon.possess_wait(pidfile)
    return Daemon.possess(pidfile)
