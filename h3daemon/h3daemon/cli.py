from __future__ import annotations

import importlib.metadata
from pathlib import Path
from typing import Optional

import typer
from deciphon_schema import HMMFile
from typer import echo

from h3daemon.daemon import Daemon
from h3daemon.daemonize import spawn
from h3daemon.pidfile import create_pidfile

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
):
    """
    Start daemon.
    """
    hmm = HMMFile(path=hmmfile)
    spawn(
        hmm,
        cport=port,
        stdin=stdin,
        stdout=stdout,
        stderr=stderr,
        force=force,
    )


@app.command()
def stop(hmmfile: Path, force: bool = O_FORCE):
    """
    Stop daemon.
    """
    hmm = HMMFile(path=hmmfile)
    pidfile = create_pidfile(hmm.path)
    x = Daemon.possess(pidfile)
    x.shutdown(force=force)


@app.command()
def ready(hmmfile: Path, wait: bool = O_WAIT):
    """
    Check if h3daemon is running and ready.
    """
    file = HMMFile(path=hmmfile)
    pidfile = create_pidfile(file.path)
    x = Daemon.possess(pidfile)
    echo(x.healthy())
