from multiprocessing import Process
from typing import Any, Optional

from daemon import DaemonContext
from deciphon_schema import HMMFile

from h3daemon.daemon import Daemon
from h3daemon.errors import DaemonAlreadyRunningError
from h3daemon.pidfile import create_pidfile
from h3daemon.port import find_free_port
from h3daemon.portfile import create_portfile

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
    pidfile = create_pidfile(hmmfile)
    assert pidfile.is_locked() is None
    with DaemonContext(
        working_directory=str(hmmfile.path.parent),
        pidfile=pidfile,
        detach_process=detach,
        stdin=stdin,
        stdout=stdout,
        stderr=stderr,
    ):
        cport = find_free_port() if cport == 0 else cport
        wport = find_free_port() if wport == 0 else wport
        create_portfile(pidfile, cport, wport)
        x = Daemon.create(hmmfile, cport, wport)
        x.join()


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
    pidfile = create_pidfile(hmmfile)
    if pidfile.is_locked():
        if not force:
            raise DaemonAlreadyRunningError(f"Daemon for {hmmfile} is already running.")
        x = Daemon.possess(pidfile)
        x.shutdown(force=force)

    args = (hmmfile, cport, wport, stdin, stdout, stderr, detach)
    p = Process(target=daemonize, args=args, daemon=False)
    p.start()
    return pidfile
