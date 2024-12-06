from __future__ import annotations

import hmmer
import psutil

from h3daemon.debug import debug_message
from h3daemon.polling import Polling
from h3daemon.tcp import tcp_connections

__all__ = ["Worker"]


class Worker:
    def __init__(self, process: psutil.Process):
        self._proc = process
        debug_message(f"Worker.__init__ PID: {process.pid}")

    @staticmethod
    def myself(process: psutil.Process):
        return "--worker" in process.cmdline()

    @property
    def process(self):
        return self._proc

    @staticmethod
    def cmd(wport: int):
        hmmpgmd = hmmer.path(hmmer.hmmpgmd)
        return [hmmpgmd, "--worker", "127.0.0.1", "--cpu", "1", "--wport", str(wport)]

    def healthy(self):
        if not self._proc.is_running():
            return False
        try:
            lports = self.local_established_ports()
            rports = self.remote_established_ports()
            debug_message(f"Worker.healthy lports: {lports}")
            debug_message(f"Worker.healthy rports: {rports}")
        except psutil.ZombieProcess:
            # psutil bug: https://github.com/giampaolo/psutil/issues/2116
            return True
        return len(lports) > 0 and len(rports) > 0

    def wait_for_readiness(self):
        for attempt in Polling():
            with attempt:
                assert self.healthy()

    def local_established_ports(self):
        connections = tcp_connections(self._proc)
        debug_message(f"Worker.local_established_ports connections: {connections}")
        connections = [x for x in connections if x.status == "ESTABLISHED"]
        return [x.laddr.port for x in connections if x.laddr.ip == "127.0.0.1"]

    def remote_established_ports(self):
        connections = tcp_connections(self._proc)
        debug_message(f"Worker.remote_established_ports connections: {connections}")
        connections = [x for x in connections if x.status == "ESTABLISHED"]
        return [x.raddr.port for x in connections if x.raddr.ip == "127.0.0.1"]
