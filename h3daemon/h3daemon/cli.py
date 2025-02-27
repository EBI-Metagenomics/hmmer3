from __future__ import annotations

import importlib.metadata
from pathlib import Path
from typing import Optional

import typer
from deciphon_schema import HMMFile
from typer import echo

from h3daemon import possess, spawn
from h3daemon.daemon import Daemon
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
O_STDIN = typer.Option(None, "--stdin")
O_STDOUT = typer.Option(None, "--stdout")
O_STDERR = typer.Option(None, "--stderr")
O_WAIT = typer.Option(False, "--wait")


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
    fin = open(stdin, "r") if stdin else stdin
    fout = open(stdout, "w+") if stdout else stdout
    ferr = open(stderr, "w+") if stderr else stderr
    spawn(
        hmm,
        port=port,
        stdin=fin,
        stdout=fout,
        stderr=ferr,
        force=force,
    )


@app.command()
def stop(hmmfile: Path, force: bool = O_FORCE):
    """
    Stop daemon.
    """
    hmm = HMMFile(path=hmmfile)
    pidfile = create_pidfile(hmm)
    x = Daemon.possess(pidfile)
    x.shutdown(force=force)


@app.command()
def ready(
    hmmfile: Path,
    wait: bool = O_WAIT,
):
    """
    Check if h3daemon is running and ready.
    """
    file = HMMFile(path=hmmfile)
    pidfile = create_pidfile(file)
    x = possess(pidfile, wait=wait)
    echo(x.healthy())
