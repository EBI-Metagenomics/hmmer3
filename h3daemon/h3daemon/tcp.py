import socket

__all__ = ["can_connect"]


def can_connect(host: str, port: int):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((host, port))
        return True
    except ConnectionRefusedError:
        return False
