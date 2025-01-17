from multiprocessing import Process
from typing import Any, Optional

from daemon import DaemonContext
from deciphon_schema import HMMFile

from h3daemon.daemon import Daemon
from h3daemon.ensure_pressed import ensure_pressed
from h3daemon.errors import DaemonAlreadyRunningError
from h3daemon.pidfile import create_pidfile

__all__ = ["daemonize", "spawn"]


def daemonize(
    hmmfile: HMMFile,
    cport: int = 0,
    wport: int = 0,
    stdin: Optional[Any] = None,
    stdout: Optional[Any] = None,
    stderr: Optional[Any] = None,
    detach: Optional[bool] = None,
):
    pidfile = create_pidfile(hmmfile.path)
    assert pidfile.is_locked() is None
    with DaemonContext(
        working_directory=str(hmmfile.path.parent),
        pidfile=pidfile,
        detach_process=detach,
        stdin=stdin,
        stdout=stdout,
        stderr=stderr,
    ):
        x = Daemon.spawn(hmmfile, cport, wport)
        x.join()

    return pidfile


def spawn(
    hmmfile: HMMFile,
    cport: int = 0,
    wport: int = 0,
    stdin: Optional[Any] = None,
    stdout: Optional[Any] = None,
    stderr: Optional[Any] = None,
    detach: Optional[bool] = None,
    force: Optional[bool] = False,
):
    ensure_pressed(hmmfile)
    pidfile = create_pidfile(hmmfile.path)
    if pidfile.is_locked():
        if not force:
            raise DaemonAlreadyRunningError(f"Daemon for {hmmfile} is already running.")
        x = Daemon.possess(pidfile)
        x.shutdown(force=force)

    args = (hmmfile, cport, wport, stdin, stdout, stderr, detach)
    p = Process(target=daemonize, args=args, daemon=False)
    p.start()
    return pidfile
