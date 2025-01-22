from __future__ import annotations

import hmmer
import psutil
from tenacity import retry, stop_after_delay, wait_exponential

from h3daemon.debug import debug_message
from h3daemon.tcp import can_connect

__all__ = ["Worker"]


class Worker:
    def __init__(self, process: psutil.Process, wport: int):
        self._proc = process
        self._wport = wport
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
        return self._proc.is_running() and can_connect("127.0.0.1", self._wport)

    @retry(stop=stop_after_delay(10), wait=wait_exponential(multiplier=0.001))
    def wait_for_readiness(self):
        assert self.healthy()
