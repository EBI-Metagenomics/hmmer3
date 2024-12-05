from __future__ import annotations

import hmmer
import psutil

from h3daemon.polling import Polling
from h3daemon.tcp import tcp_connections

__all__ = ["Master"]


class Master:
    def __init__(self, proc: psutil.Process):
        self._proc = proc

    @property
    def process(self):
        return self._proc

    @staticmethod
    def cmd(cport: int, wport: int, hmmfile: str):
        cmd = [hmmer.path(hmmer.hmmpgmd), "--master", "--hmmdb", hmmfile]
        cmd += ["--cport", str(cport), "--wport", str(wport)]
        return cmd

    def healthy(self):
        if not self._proc.is_running():
            return False
        lports = self.local_listening_ports()
        return len(lports) > 1

    def wait_for_readiness(self):
        for attempt in Polling():
            with attempt:
                assert self.healthy()

    def local_listening_ports(self):
        connections = tcp_connections(self._proc)
        connections = [x for x in connections if x.status == "LISTEN"]
        return [x.laddr.port for x in connections if x.laddr.ip == "0.0.0.0"]

    def local_established_ports(self):
        connections = tcp_connections(self._proc)
        connections = [x for x in connections if x.status == "ESTABLISHED"]
        return [x.laddr.port for x in connections if x.laddr.ip == "127.0.0.1"]

    def remote_established_ports(self):
        connections = tcp_connections(self._proc)
        connections = [x for x in connections if x.status == "ESTABLISHED"]
        return [x.raddr.port for x in connections if x.raddr.ip == "127.0.0.1"]
