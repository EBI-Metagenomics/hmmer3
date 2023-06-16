import socket
from contextlib import closing

__all__ = ["can_connect", "find_free_port"]


def can_connect(cport: int):
    with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as sock:
        return sock.connect_ex(("127.0.0.1", cport)) == 0


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
