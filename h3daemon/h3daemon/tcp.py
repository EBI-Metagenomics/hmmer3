from psutil import Process

__all__ = ["tcp_connections"]


def tcp_connections(x: Process):
    # psutil bug: https://github.com/giampaolo/psutil/issues/2116
    with open("/dev/null", "wb"):
        connections = x.net_connections(kind="tcp")
    return connections
