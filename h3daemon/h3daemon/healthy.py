from h3daemon.master import Master
from h3daemon.worker import Worker


def assert_peers_healthy(master: Master, worker: Worker):
    assert master.healthy()
    assert worker.healthy()
    master_listen = master.local_listening_ports()
    master_lport = master.local_established_ports()
    master_rport = master.remote_established_ports()
    worker_lport = worker.local_established_ports()
    worker_rport = worker.remote_established_ports()

    assert len(master_lport) >= 1
    assert len(worker_rport) == 1
    assert worker_rport[0] in master_lport
    assert len(master_rport) >= 1
    assert len(worker_lport) == 1
    assert worker_lport[0] in master_rport
    assert len(master_listen) == 2
    master_ports = set(master_listen)
    assert len(master_ports) == 2
    master_ports.remove(worker_rport[0])
    assert len(master_ports) == 1
