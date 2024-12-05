from __future__ import annotations

import time

import hmmer
import psutil
from tenacity import Retrying, stop_after_delay, wait_exponential

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

    def is_ready(self):
        if not self.is_running():
            return False
        try:
            lports = self.local_listening_ports()
        except RuntimeError:
            # psutil bug: https://github.com/giampaolo/psutil/issues/2116
            time.sleep(1)
            lports = [-1, -1]
        return len(lports) > 1

    def wait_for_readiness(self):
        for attempt in Retrying(stop=stop_after_delay(10), wait=wait_exponential()):
            with attempt:
                assert self.is_ready()

    def is_running(self):
        return self._proc.is_running()

    def local_listening_ports(self):
        connections = self._proc.net_connections(kind="tcp")
        connections = [x for x in connections if x.status == "LISTEN"]
        return [x.laddr.port for x in connections if x.laddr.ip == "0.0.0.0"]

    def local_established_ports(self):
        connections = self._proc.net_connections(kind="tcp")
        connections = [x for x in connections if x.status == "ESTABLISHED"]
        return [x.laddr.port for x in connections if x.laddr.ip == "127.0.0.1"]

    def remote_established_ports(self):
        connections = self._proc.net_connections(kind="tcp")
        connections = [x for x in connections if x.status == "ESTABLISHED"]
        return [x.raddr.port for x in connections if x.raddr.ip == "127.0.0.1"]
