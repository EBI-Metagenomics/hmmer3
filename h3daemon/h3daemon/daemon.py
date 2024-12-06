import platform
from contextlib import contextmanager, suppress

import psutil
from deciphon_schema import HMMFile
from pidlockfile import PIDLockFile

from h3daemon.debug import debug_exception, debug_message
from h3daemon.ensure_pressed import ensure_pressed
from h3daemon.errors import ChildNotFoundError, PIDNotFoundError
from h3daemon.healthy import assert_peers_healthy
from h3daemon.master import Master
from h3daemon.polling import polling
from h3daemon.port import find_free_port
from h3daemon.worker import Worker

__all__ = ["Daemon", "daemon_context"]


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


class Daemon:
    def __init__(self, master: Master, worker: Worker, process: psutil.Process | None):
        self._master = master
        self._worker = worker
        self._process = process
        if process is not None:
            debug_message(f"Daemon.__init__ PID: {process.pid}")

    @classmethod
    def spawn(cls, hmmfile: HMMFile, cport: int = 0, wport: int = 0):
        master: Master | None = None
        worker: Worker | None = None

        try:
            cport = find_free_port() if cport == 0 else cport
            wport = find_free_port() if wport == 0 else wport
            debug_message(f"Daemon.spawn cport: {cport}")
            debug_message(f"Daemon.spawn wport: {wport}")

            cmd = Master.cmd(cport, wport, str(hmmfile.path))
            master = Master(psutil.Popen(cmd))
            master.wait_for_readiness()

            cmd = Worker.cmd(wport)
            worker = Worker(psutil.Popen(cmd))
            worker.wait_for_readiness()

            if platform.system() != "Darwin":
                assert_peers_healthy(master, worker)
            debug_message("Daemon.spawn is ready")
        except Exception as exception:
            debug_exception(exception)
            if worker:
                shutdown(worker.process, force=True)
            if master:
                shutdown(master.process, force=True)
            raise exception

        return cls(master, worker, None)

    @classmethod
    @polling
    def possess(cls, pidfile: PIDLockFile):
        pid = pidfile.is_locked()
        if not pid:
            raise PIDNotFoundError("PID not in pidfile.")

        process = psutil.Process(pid)
        children = process.children()

        masters = [x for x in children if Master.myself(x)]
        workers = [x for x in children if Worker.myself(x)]

        if len(masters) > 0:
            assert len(masters) == 1
            master = Master(masters[0])
        else:
            raise ChildNotFoundError("Master not found.")

        if len(workers) > 0:
            assert len(workers) == 1
            worker = Worker(workers[0])
        else:
            raise ChildNotFoundError("Worker not found.")

        debug_message("Daemon.possess finished")
        return cls(master, worker, process)

    def shutdown(self, force=False):
        if self._worker is not None:
            shutdown(self._worker.process, True)

        if self._master is not None:
            shutdown(self._master.process, True)

        if self._process is not None:
            shutdown(self._process, force)

        debug_message("Daemon.shutdown finished")

    @polling
    def wait_for_readiness(self):
        assert self.healthy

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
        self.wait_for_readiness()
        master_ports = set(self._master.local_listening_ports())
        worker_ports = list(set(self._worker.remote_established_ports()))
        debug_message(f"Daemon.port master_ports: {master_ports}")
        debug_message(f"Daemon.port worker_ports: {worker_ports}")
        if len(worker_ports) != 1:
            raise RuntimeError(
                f"Expected one remote port ({worker_ports}). Worker might have died."
            )
        master_ports.remove(worker_ports[0])
        if len(master_ports) != 1:
            raise RuntimeError(
                f"Expected one remaining master port ({master_ports}). Master might have died."
            )
        return int(list(master_ports)[0])

    def join(self):
        psutil.wait_procs([self._master.process, self._worker.process])


@contextmanager
def daemon_context(hmmfile: HMMFile, cport: int = 0, wport: int = 0):
    ensure_pressed(hmmfile)
    x = Daemon.spawn(hmmfile, cport, wport)
    try:
        x.wait_for_readiness()
        yield x
    finally:
        x.shutdown()
