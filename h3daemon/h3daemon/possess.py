from pidlockfile import PIDLockFile
from h3daemon.polling import polling

from h3daemon.daemon import Daemon

__all__ = ["possess"]


@polling
def possess(pidfile: PIDLockFile):
    return Daemon.possess(pidfile)
