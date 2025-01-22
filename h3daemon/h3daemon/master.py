from __future__ import annotations

import hmmer
import psutil
from tenacity import retry, stop_after_delay, wait_exponential

from h3daemon.debug import debug_message
from h3daemon.tcp import can_connect

__all__ = ["Master"]


class Master:
    def __init__(self, process: psutil.Process, cport: int, wport: int):
        self._proc = process
        self._cport = cport
        self._wport = wport
        debug_message(f"Master.__init__ PID: {process.pid}")

    def cport(self):
        return self._cport

    @staticmethod
    def myself(process: psutil.Process):
        return "--master" in process.cmdline()

    @property
    def process(self):
        return self._proc

    @staticmethod
    def cmd(cport: int, wport: int, hmmfile: str):
        cmd = [hmmer.path(hmmer.hmmpgmd), "--master", "--hmmdb", hmmfile]
        cmd += ["--cport", str(cport), "--wport", str(wport)]
        return cmd

    def healthy(self):
        return (
            self._proc.is_running()
            and can_connect("127.0.0.1", self._cport)
            and can_connect("127.0.0.1", self._wport)
        )

    @retry(stop=stop_after_delay(10), wait=wait_exponential(multiplier=0.001))
    def wait_for_readiness(self):
        assert self.healthy()
