from __future__ import annotations

import hmmer
import psutil

from h3daemon.debug import debug_exception, debug_message
from h3daemon.polling import polling
from h3daemon.tcp import tcp_connections

__all__ = ["Master"]


class Master:
    def __init__(self, process: psutil.Process):
        self._proc = process
        debug_message(f"Master.__init__ PID: {process.pid}")

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
        if not self._proc.is_running():
            return False
        try:
            lports = self.local_listening_ports()
            debug_message(f"Master.healthy lports: {lports}")
        except Exception as exception:
            debug_exception(exception)
            return False
        return len(lports) > 1

    @polling
    def wait_for_readiness(self):
        assert self.healthy()

    def local_listening_ports(self):
        connections = tcp_connections(self._proc)
        debug_message(f"Master.local_listening_ports connections: {connections}")
        connections = [x for x in connections if x.status == "LISTEN"]
        return [x.laddr.port for x in connections if x.laddr.ip == "0.0.0.0"]

    def local_established_ports(self):
        connections = tcp_connections(self._proc)
        debug_message(f"Master.local_established_ports connections: {connections}")
        connections = [x for x in connections if x.status == "ESTABLISHED"]
        return [x.laddr.port for x in connections if x.laddr.ip == "127.0.0.1"]

    def remote_established_ports(self):
        connections = tcp_connections(self._proc)
        debug_message(f"Master.remote_established_ports connections: {connections}")
        connections = [x for x in connections if x.status == "ESTABLISHED"]
        return [x.raddr.port for x in connections if x.raddr.ip == "127.0.0.1"]
