from __future__ import annotations

import importlib.metadata
from pathlib import Path
from typing import Optional

import typer
from daemon import DaemonContext
from deciphon_schema import HMMFile
from typer import echo

from h3daemon.ensure_pressed import ensure_pressed
from h3daemon.manager import Manager
from h3daemon.pidfile import create_pidfile
from h3daemon.port import find_free_port

__all__ = ["app"]


app = typer.Typer(
    add_completion=False,
    pretty_exceptions_short=True,
    pretty_exceptions_show_locals=False,
)

O_VERSION = typer.Option(None, "--version", is_eager=True)
O_PORT = typer.Option(0, help="Port to listen to.")
O_FORCE = typer.Option(False, "--force")
O_WAIT = typer.Option(False, "--wait")
O_STDIN = typer.Option(None, "--stdin")
O_STDOUT = typer.Option(None, "--stdout")
O_STDERR = typer.Option(None, "--stderr")
O_DETACH = typer.Option(
    None, "--detach/--no-detach", show_default=False, help="[default: None]"
)


@app.callback(invoke_without_command=True)
def cli(version: Optional[bool] = O_VERSION):
    if version:
        echo(importlib.metadata.version(str(__package__)))
        raise typer.Exit()


@app.command()
def start(
    hmmfile: Path,
    port: int = O_PORT,
    stdin: Optional[Path] = O_STDIN,
    stdout: Optional[Path] = O_STDOUT,
    stderr: Optional[Path] = O_STDERR,
    force: bool = O_FORCE,
    detach: Optional[bool] = O_DETACH,
):
    """
    Start daemon.
    """
    hmm = HMMFile(path=hmmfile)
    ensure_pressed(hmm)
    pidfile = create_pidfile(hmm.path)
    if pid := pidfile.is_locked():
        if not force:
            echo(f"Daemon for {hmmfile} is already running.")
            raise typer.Exit(1)
        echo("Cleaning up previous daemon...")
        x = Manager.possess(pid)
        x.shutdown(force=force)

    cport = find_free_port() if port == 0 else port
    wport = find_free_port()
    fin = open(stdin, "r") if stdin else stdin
    fout = open(stdout, "w+") if stdout else stdout
    ferr = open(stderr, "w+") if stderr else stderr

    assert pidfile.is_locked() is None
    with DaemonContext(
        working_directory=str(hmm.path.parent),
        pidfile=pidfile,
        detach_process=detach,
        stdin=fin,
        stdout=fout,
        stderr=ferr,
    ):
        x = Manager.spawn(hmm, cport, wport)
        x.join()


@app.command()
def stop(hmmfile: Path, force: bool = O_FORCE):
    """
    Stop daemon.
    """
    hmm = HMMFile(path=hmmfile)
    pidfile = create_pidfile(hmm.path)
    if pid := pidfile.is_locked():
        x = Manager.possess(pid)
        x.shutdown(force=force)
    else:
        echo("No process associated with the PID file.")
        raise typer.Exit(1)


@app.command()
def ready(hmmfile: Path, wait: bool = O_WAIT):
    """
    Check if h3daemon is running and ready.
    """
    file = HMMFile(path=hmmfile)
    pidfile = create_pidfile(file.path)
    if pid := pidfile.is_locked():
        x = Manager.possess(pid)
        echo(x.healthy())
    else:
        echo(f"Failed to possess {hmmfile}. Have you started the daemon?")
        raise typer.Exit(1)
