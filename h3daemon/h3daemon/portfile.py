from pathlib import Path

from pidlockfile import PIDLockFile

__all__ = ["create_portfile", "read_portfile"]


def create_portfile(pidfile: PIDLockFile, cport: int, wport: int):
    portfile = _portfile(pidfile)
    with open(portfile, "w") as f:
        f.write(f"{cport} {wport}\n")
    return portfile


def read_portfile(pidfile: PIDLockFile):
    portfile = _portfile(pidfile)
    with open(portfile, "r") as f:
        cport, wport = f.readline().strip().split(" ")
        return int(cport), int(wport)


def _portfile(pidfile: PIDLockFile):
    return f"{str(Path(pidfile.path).absolute())}.port"
