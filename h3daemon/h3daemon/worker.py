from __future__ import annotations

import hmmer
import psutil

from h3daemon.polling import Polling
from h3daemon.tcp import tcp_connections

__all__ = ["Worker"]


class Worker:
    def __init__(self, proc: psutil.Process):
        self._proc = proc

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
        connections = [x for x in connections if x.status == "ESTABLISHED"]
        return [x.laddr.port for x in connections if x.laddr.ip == "127.0.0.1"]

    def remote_established_ports(self):
        connections = tcp_connections(self._proc)
        connections = [x for x in connections if x.status == "ESTABLISHED"]
        return [x.raddr.port for x in connections if x.raddr.ip == "127.0.0.1"]
