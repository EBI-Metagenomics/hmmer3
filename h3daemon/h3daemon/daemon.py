import traceback
from contextlib import contextmanager, suppress
from multiprocessing import Process
from typing import Any, Optional

import psutil
from daemon import DaemonContext
from deciphon_schema import HMMFile
from loguru import logger
from pidlockfile import PIDLockFile
from tenacity import retry, stop_after_delay, wait_exponential

from h3daemon.ensure_pressed import ensure_pressed
from h3daemon.errors import (
    ChildNotFoundError,
    DaemonAlreadyRunningError,
    PIDNotFoundError,
)
from h3daemon.fixstreams import fixstreams
from h3daemon.master import Master
from h3daemon.pidfile import create_pidfile
from h3daemon.port import find_free_port
from h3daemon.portfile import create_portfile, read_portfile
from h3daemon.worker import Worker

__all__ = ["Daemon", "context"]


def shutdown(x: psutil.Process, *, force: bool, wait: bool):
    with suppress(psutil.NoSuchProcess):
        if force:
            x.kill()
            if wait:
                x.wait()
        else:
            x.terminate()
            try:
                if wait:
                    x.wait(5)
            except psutil.TimeoutExpired:
                shutdown(x, force=True, wait=wait)


class Daemon:
    def __init__(self, master: Master, worker: Worker, process: psutil.Process | None):
        self._master = master
        self._worker = worker
        self._process = process
        if process is not None:
            logger.info(f"Daemon.__init__ PID: {process.pid}.")

    @classmethod
    @logger.catch(reraise=True)
    def create(cls, hmmfile: HMMFile, cport: int, wport: int):
        ensure_pressed(hmmfile)
        master: Master | None = None
        worker: Worker | None = None

        try:
            logger.info(f"Daemon.spawn cport: {cport}.")
            logger.info(f"Daemon.spawn wport: {wport}.")

            cmd = Master.cmd(cport, wport, str(hmmfile.path))
            master = Master(psutil.Popen(cmd), cport, wport)
            master.wait_for_readiness()

            cmd = Worker.cmd(wport)
            worker = Worker(psutil.Popen(cmd), wport)
            worker.wait_for_readiness()

            logger.info("Daemon.spawn is ready")
        except Exception as exception:
            if worker:
                shutdown(worker.process, force=True, wait=False)
            if master:
                shutdown(master.process, force=True, wait=False)
            raise exception

        return cls(master, worker, None)

    @classmethod
    @retry(stop=stop_after_delay(10), wait=wait_exponential(multiplier=0.001))
    def possess_wait(cls, pidfile: PIDLockFile):
        return Daemon.possess(pidfile)

    @classmethod
    def possess(cls, pidfile: PIDLockFile):
        pid = pidfile.is_locked()
        if not pid:
            raise PIDNotFoundError("PID not in pidfile.")

        process = psutil.Process(pid)
        children = process.children()

        masters = [x for x in children if Master.myself(x)]
        workers = [x for x in children if Worker.myself(x)]

        cport, wport = read_portfile(pidfile)

        if len(masters) > 0:
            assert len(masters) == 1
            master = Master(masters[0], cport, wport)
        else:
            raise ChildNotFoundError("Master not found.")

        if len(workers) > 0:
            assert len(workers) == 1
            worker = Worker(workers[0], wport)
        else:
            raise ChildNotFoundError("Worker not found.")

        logger.info("Daemon.possess finished.")
        return cls(master, worker, process)

    def shutdown(self, force=False):
        if self._master is not None:
            shutdown(self._master.process, force=force, wait=False)

        if self._worker is not None:
            shutdown(self._worker.process, force=force, wait=False)

        if self._process is not None:
            shutdown(self._process, force=force, wait=True)

        logger.info("Daemon.shutdown finished.")

    @retry(stop=stop_after_delay(10), wait=wait_exponential(multiplier=0.001))
    def wait_for_readiness(self):
        assert self.healthy()

    def healthy(self) -> bool:
        logger.info("Checking deamon healthy...")
        try:
            if self._process:
                assert self._process.is_running()
            assert self._master.healthy()
            assert self._worker.healthy()
        except Exception as exception:
            logger.info(f"Exception raised while checking healthy: {str(exception)}.")
            return False
        return True

    def port(self) -> int:
        self.wait_for_readiness()
        return self._master.cport()

    def join(self):
        psutil.wait_procs([self._master.process, self._worker.process])


@contextmanager
def context(hmmfile: HMMFile, port: int = 0):
    port = find_free_port() if port == 0 else port
    x = Daemon.create(hmmfile, port, find_free_port())
    try:
        yield x.port()
    finally:
        x.shutdown()


def daemonize(
    pidfile: PIDLockFile,
    hmmfile: HMMFile,
    port: int = 0,
    stdin: Optional[Any] = None,
    stdout: Optional[Any] = None,
    stderr: Optional[Any] = None,
):
    logfile = f"{str(hmmfile.path)}.log"
    logger.add(logfile, level="TRACE")
    logger.info("Spawning daemon up...")

    # https://pagure.io/python-daemon/issue/89
    with fixstreams():
        ctx = DaemonContext(
            working_directory=str(hmmfile.path.parent),
            pidfile=pidfile,
            detach_process=True,
            stdin=stdin,
            stdout=stdout,
            stderr=stderr,
        )
        try:
            try:
                # We remove it to avoid log duplication due to fork that will happen
                # bellow and to avoid broken sink due to daemon construction.
                logger.remove()
                ctx.open()
            except Exception as exception:
                logger.add(logfile, level="TRACE")
                logger.trace(str(exception) + "\n" + traceback.format_exc())
                raise

            try:
                # We are now the child resulted from the fork. Open a sink for it.
                logger.add(logfile, level="TRACE")
                port = find_free_port() if port == 0 else port
                wport = find_free_port()
                create_portfile(pidfile, port, wport)
                x = Daemon.create(hmmfile, port, wport)
                x.join()
            except Exception as exception:
                logger.trace(str(exception) + "\n" + traceback.format_exc())
                raise
        finally:
            ctx.close()


def spawn(
    hmmfile: HMMFile,
    port: int = 0,
    stdin: Optional[Any] = None,
    stdout: Optional[Any] = None,
    stderr: Optional[Any] = None,
    force: Optional[bool] = False,
):
    pidfile = create_pidfile(hmmfile)
    if pidfile.is_locked():
        if not force:
            raise DaemonAlreadyRunningError(f"Daemon for {hmmfile} is already running.")
        x = Daemon.possess(pidfile)
        x.shutdown(force=force)

    hmmfile = HMMFile(path=hmmfile.path.absolute())
    args = (pidfile, hmmfile, port, stdin, stdout, stderr)

    p = Process(target=daemonize, args=args, daemon=False)
    p.start()
    p.join()
    assert p.exitcode == 0

    return pidfile


def possess(pidfile: PIDLockFile, wait=True):
    if wait:
        return Daemon.possess_wait(pidfile)
    return Daemon.possess(pidfile)
