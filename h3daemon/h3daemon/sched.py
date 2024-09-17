from __future__ import annotations

import atexit
import os
import sys
from typing import Optional, Union

import psutil
from daemon import DaemonContext
from pidlockfile import PIDLockFile

from h3daemon.connect import find_free_port
from h3daemon.errors import (
    ChildNotFoundError,
    CouldNotPossessError,
    ParentNotAliveError,
)
from h3daemon.hmmfile import HMMFile
from h3daemon.master import Master
from h3daemon.pidfile import create_pidfile
from h3daemon.polling import wait_until
from h3daemon.worker import Worker

__all__ = ["Sched", "SchedContext"]


def spawn_master(hmmfile: str, cport: int, wport: int):
    cmd = Master.cmd(cport, wport, hmmfile)
    master = Master(psutil.Popen(cmd))
    wait_until(master.is_ready)
    return master


def spawn_worker(wport: int):
    cmd = Worker.cmd(wport)
    worker = Worker(psutil.Popen(cmd))
    wait_until(worker.is_ready)
    return worker


def entry_point(hmmfile: str, cport: int, wport: int):
    sched = Sched(psutil.Process(os.getpid()))
    sched.run(hmmfile, cport, wport)


class SchedContext:
    def __init__(
        self, hmmfile: HMMFile, cport: int = 0, wport: int = 0, stdout=None, stderr=None
    ):
        hmmfile.ensure_pressed()
        self._hmmfile = hmmfile
        self._cport = find_free_port() if cport == 0 else cport
        self._wport = find_free_port() if wport == 0 else wport
        self._exe = sys.executable
        self._scr = os.path.realpath(__file__)
        self._sched: Optional[Sched] = None
        self._stdout = stdout
        self._stderr = stderr

    def open(self):
        cmd = [self._exe, self._scr, str(self._hmmfile)]
        cmd += [str(self._cport), str(self._wport)]
        self._sched = Sched(psutil.Popen(cmd, stdout=self._stdout, stderr=self._stderr))
        atexit.register(self.close)

    def close(self):
        if not self._sched:
            return
        self._sched.kill_children()
        self._sched.kill_parent()
        self._sched = None

    @property
    def sched(self) -> Sched:
        assert self._sched
        return self._sched

    def __enter__(self):
        self.open()
        return self.sched

    def __exit__(self, *_):
        self.close()


class Sched:
    def __init__(self, proc: psutil.Process):
        self._proc = proc

    @classmethod
    def possess(cls, hmmfile: HMMFile, pidfile: Optional[PIDLockFile] = None):
        if not pidfile:
            pidfile = create_pidfile(hmmfile.path)

        pid = pidfile.is_locked()
        if pid:
            return cls(psutil.Process(pid))
        raise CouldNotPossessError(
            f"Failed to possess {hmmfile}. Have you started the daemon?"
        )

    @staticmethod
    def daemonize(
        hmmfile: HMMFile,
        pidfile: PIDLockFile,
        cport: int,
        wport: int,
        stdin,
        stdout,
        stderr,
        detach: Optional[bool] = None,
    ):
        assert pidfile.is_locked() is None
        ctx = DaemonContext(
            working_directory=str(hmmfile.path.parent),
            pidfile=pidfile,
            detach_process=detach,
            stdin=stdin,
            stdout=stdout,
            stderr=stderr,
        )
        with ctx:
            entry_point(str(hmmfile), cport, wport)

    def run(self, hmmfile: str, cport: int, wport: int):
        try:
            master = spawn_master(hmmfile, cport, wport)
            worker = spawn_worker(wport)

            def shutdown(_):
                try:
                    self.terminate_children(15)
                except psutil.TimeoutExpired:
                    self.kill_children()

            psutil.wait_procs([master.process, worker.process], callback=shutdown)
        finally:
            try:
                self.terminate_children(15)
            except psutil.TimeoutExpired:
                self.kill_children()

    def kill_children(self):
        for x in self._proc.children():
            x.kill()
            x.wait()

    def kill_parent(self):
        self._proc.kill()
        self._proc.wait()

    def terminate_children(self, timeout: Union[float, None] = None):
        for x in self._proc.children():
            try:
                x.terminate()
                x.wait(timeout)
            except psutil.NoSuchProcess:
                pass

    def terminate_parent(self, timeout: Union[float, None] = None):
        try:
            self._proc.terminate()
            self._proc.wait(timeout)
        except psutil.NoSuchProcess:
            pass

    def is_alive(self):
        if not psutil.pid_exists(self._proc.pid):
            return False
        return self._proc.status() not in (psutil.STATUS_DEAD, psutil.STATUS_ZOMBIE)

    def is_ready(self):
        if not self.is_alive():
            raise ParentNotAliveError()
        try:
            master = self.master
            worker = self.worker
        except ChildNotFoundError:
            return False
        return master.is_ready() and worker.is_ready() and self._is_healthy()

    @property
    def master(self) -> Master:
        children = self._proc.children()
        if len(children) > 0:
            return Master(children[0])
        raise ChildNotFoundError("Master not found.")

    @property
    def worker(self) -> Worker:
        children = self._proc.children()
        if len(children) > 1:
            return Worker(children[1])
        raise ChildNotFoundError("Worker not found.")

    def get_cport(self) -> int:
        wait_until(self.is_ready)
        return self._get_cport()

    def _is_healthy(self):
        try:
            self._assert_healthy()
        except AssertionError:
            return False
        return True

    def _assert_healthy(self):
        assert self.master.is_running()
        assert self.worker.is_running()
        master_listen = self.master.local_listening_ports()
        master_lport = self.master.local_established_ports()
        master_rport = self.master.remote_established_ports()
        worker_lport = self.worker.local_established_ports()
        worker_rport = self.worker.remote_established_ports()

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

    def _get_cport(self):
        master_ports = set(self.master.local_listening_ports())
        master_ports.remove(self.worker.remote_established_ports()[0])
        return int(list(master_ports)[0])


if __name__ == "__main__":
    entry_point(sys.argv[1], int(sys.argv[2]), int(sys.argv[3]))
