import socket
from contextlib import closing

__all__ = ["find_free_port"]


def _find_free_port():
    with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as s:
        s.bind(("", 0))
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        return s.getsockname()[1]


def find_free_port():
    port = _find_free_port()
    while not (49152 <= port and port <= 65535):
        port = _find_free_port()
    return port
