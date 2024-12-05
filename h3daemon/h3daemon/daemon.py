import os
import platform
import traceback
from contextlib import contextmanager, suppress

import psutil
from deciphon_schema import HMMFile
from pidlockfile import PIDLockFile

from h3daemon.ensure_pressed import ensure_pressed
from h3daemon.errors import ChildNotFoundError, PIDNotFoundError
from h3daemon.healthy import assert_peers_healthy
from h3daemon.master import Master
from h3daemon.port import find_free_port
from h3daemon.worker import Worker

__all__ = ["Daemon", "daemon_context"]


def shutdown(x: psutil.Process, force: bool):
    with suppress(psutil.NoSuchProcess):
        if force:
            x.kill()
        else:
            x.terminate()
            try:
                x.wait(3)
            except psutil.TimeoutExpired:
                shutdown(x, True)


def debug_exception(exception: Exception):
    if os.environ.get("H3DAEMON_DEBUG", 0):
        with open("h3daemon_debug.txt", "a") as f:
            f.write(f"{type(exception)}\n")
            f.write(f"{exception.args}\n")
            f.write(traceback.format_exc())


def debug_msg(msg: str):
    if os.environ.get("H3DAEMON_DEBUG", 0):
        with open("h3daemon_debug.txt", "a") as f:
            f.write(f"{msg}\n")


class Daemon:
    def __init__(self, master: Master, worker: Worker, process: psutil.Process | None):
        self._master = master
        self._worker = worker
        self._process = process

    @classmethod
    def spawn(cls, hmmfile: HMMFile, cport: int = 0, wport: int = 0):
        master: Master | None = None
        worker: Worker | None = None

        try:
            cport = find_free_port() if cport == 0 else cport
            wport = find_free_port() if wport == 0 else wport

            cmd = Master.cmd(cport, wport, str(hmmfile.path))
            master = Master(psutil.Popen(cmd))
            master.wait_for_readiness()

            cmd = Worker.cmd(wport)
            worker = Worker(psutil.Popen(cmd))
            worker.wait_for_readiness()

            if platform.system() != "Darwin":
                assert_peers_healthy(master, worker)
        except Exception as exception:
            debug_exception(exception)
            if worker:
                shutdown(worker.process, force=True)
            if master:
                shutdown(master.process, force=True)
            raise exception

        return cls(master, worker, None)

    @classmethod
    def possess(cls, pidfile: PIDLockFile):
        pid = pidfile.is_locked()
        if not pid:
            raise PIDNotFoundError("PID not in pidfile.")
        process = psutil.Process(pid)
        children = process.children()

        if len(children) > 0:
            master = Master(children[0])
        else:
            raise ChildNotFoundError("Master not found.")

        if len(children) > 1:
            worker = Worker(children[1])
        else:
            raise ChildNotFoundError("Worker not found.")

        return cls(master, worker, process)

    def shutdown(self, force=False):
        if self._worker is not None:
            shutdown(self._worker.process, True)

        if self._master is not None:
            shutdown(self._master.process, True)

        if self._process is not None:
            shutdown(self._process, force)

    def healthy(self) -> bool:
        try:
            if self._process:
                assert self._process.is_running()
            assert_peers_healthy(self._master, self._worker)
        except Exception as exception:
            debug_exception(exception)
            return False
        return True

    def port(self) -> int:
        master_ports = set(self._master.local_listening_ports())
        master_ports.remove(self._worker.remote_established_ports()[0])
        return int(list(master_ports)[0])

    def join(self):
        psutil.wait_procs([self._master.process, self._worker.process])


@contextmanager
def daemon_context(hmmfile: HMMFile, cport: int = 0, wport: int = 0):
    ensure_pressed(hmmfile)
    x = Daemon.spawn(hmmfile, cport, wport)
    try:
        yield x
    finally:
        x.shutdown()
