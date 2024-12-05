import os
import time
import traceback
from contextlib import suppress

import psutil
from deciphon_schema import HMMFile
from tenacity import Retrying, stop_after_delay, wait_exponential

from h3daemon.errors import ChildNotFoundError
from h3daemon.master import Master
from h3daemon.port import find_free_port
from h3daemon.worker import Worker

__all__ = ["Manager"]


def shutdown(x: psutil.Process, force: bool):
    with suppress(psutil.NoSuchProcess):
        if force:
            x.kill()
            x.wait()
        else:
            x.terminate()
            try:
                x.wait(3)
            except psutil.TimeoutExpired:
                shutdown(x, True)


def assert_healthy(master: Master, worker: Worker):
    try:
        assert master.is_running()
        assert worker.is_running()
        master_listen = master.local_listening_ports()
        master_lport = master.local_established_ports()
        master_rport = master.remote_established_ports()
        worker_lport = worker.local_established_ports()
        worker_rport = worker.remote_established_ports()

        assert len(master_lport) == 1
        assert len(worker_rport) == 1
        assert master_lport[0] == worker_rport[0]
        assert len(master_rport) == 1
        assert len(worker_lport) == 1
        assert master_rport[0] == worker_lport[0]
        assert len(master_listen) == 2
        master_ports = set(master_listen)
        assert len(master_ports) == 2
        master_ports.remove(worker_rport[0])
        assert len(master_ports) == 1
    except RuntimeError as exception:
        if not exception.args[0] == "proc_pidinfo(PROC_PIDLISTFDS) 2/2 syscall failed":
            raise exception
        # psutil bug: https://github.com/giampaolo/psutil/issues/2116
        time.sleep(0.1)


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


class Manager:
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

            assert_healthy(master, worker)
        except Exception as exception:
            debug_exception(exception)
            if master:
                shutdown(master.process, force=True)
            if worker:
                shutdown(worker.process, force=True)
            raise exception

        return cls(master, worker, None)

    @classmethod
    def possess(cls, pid: int):
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
        if self._master is not None:
            shutdown(self._master.process, force)

        if self._worker is not None:
            shutdown(self._worker.process, force)

        if self._process is not None:
            shutdown(self._process, force)

    def healthy(self) -> bool:
        try:
            assert_healthy(self._master, self._worker)
        except Exception as exception:
            debug_exception(exception)
            return False
        return True

    def port(self) -> int:
        for attempt in Retrying(stop=stop_after_delay(10), wait=wait_exponential()):
            with attempt:
                assert self.healthy
        master_ports = set(self._master.local_listening_ports())
        master_ports.remove(self._worker.remote_established_ports()[0])
        return int(list(master_ports)[0])

    def join(self):
        psutil.wait_procs([self._master.process, self._worker.process])
