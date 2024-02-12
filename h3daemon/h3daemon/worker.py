from __future__ import annotations

import time
from pathlib import Path

import hmmer
import psutil

__all__ = ["Worker"]


def has_connected(pid: int):
    try:
        for x in psutil.Process(pid).connections(kind="tcp"):
            if x.status == "ESTABLISHED":
                return True
    except RuntimeError:
        # Bug to be fixed: https://github.com/giampaolo/psutil/issues/2116
        time.sleep(0.1)
        return True
    return False


class Worker:
    def __init__(self, proc: psutil.Process):
        self._proc = proc

    @property
    def process(self):
        return self._proc

    @staticmethod
    def cmd(wport: int):
        hmmpgmd = str(Path(hmmer.BIN_DIR) / "hmmpgmd")
        return [hmmpgmd, "--worker", "127.0.0.1", "--cpu", "1", "--wport", str(wport)]

    def is_ready(self):
        if not self.is_running():
            return False
        try:
            lports = self.local_established_ports()
            rports = self.remote_established_ports()
        except (ProcessLookupError, psutil.ZombieProcess):
            return False
        except RuntimeError:
            # psutil bug: https://github.com/giampaolo/psutil/issues/2116
            lports = [0]
            rports = [0]
        return len(lports) > 0 and len(rports) > 0

    def is_running(self):
        return self._proc.is_running()

    def local_established_ports(self):
        connections = self._proc.connections(kind="tcp")
        connections = [x for x in connections if x.status == "ESTABLISHED"]
        return [x.laddr.port for x in connections if x.laddr.ip == "127.0.0.1"]

    def remote_established_ports(self):
        connections = self._proc.connections(kind="tcp")
        connections = [x for x in connections if x.status == "ESTABLISHED"]
        return [x.raddr.port for x in connections if x.raddr.ip == "127.0.0.1"]
