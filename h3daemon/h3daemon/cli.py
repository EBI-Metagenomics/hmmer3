from __future__ import annotations

import importlib.metadata
from functools import partial
from pathlib import Path
from typing import Optional

import typer
from typer import echo

from h3daemon.connect import find_free_port
from h3daemon.errors import CouldNotPossessError, ParentNotAliveError
from h3daemon.hmmfile import HMMFile
from h3daemon.pidfile import create_pidfile
from h3daemon.sched import Sched
from h3daemon.polling import wait_until

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
    file = HMMFile(hmmfile)
    file.ensure_pressed()
    pidfile = create_pidfile(file.path)
    if pidfile.is_locked():
        if force:
            sched = Sched.possess(file, pidfile)
            sched.kill_children()
            sched.kill_parent()
            file = HMMFile(hmmfile)
        else:
            typer.echo(f"Daemon for {hmmfile} is already running.")
            raise typer.Exit(1)
    cport = find_free_port() if port == 0 else port
    wport = find_free_port()
    fin = open(stdin, "r") if stdin else stdin
    fout = open(stdout, "w+") if stdout else stdout
    ferr = open(stderr, "w+") if stderr else stderr
    Sched.daemonize(file, pidfile, cport, wport, fin, fout, ferr, detach)


@app.command()
def stop(hmmfile: Path, force: bool = O_FORCE):
    """
    Stop daemon.
    """
    try:
        sched = Sched.possess(HMMFile(hmmfile))
    except CouldNotPossessError as x:
        typer.echo(str(x))
        raise typer.Exit(1)
    if force:
        sched.kill_children()
        sched.kill_parent()
    else:
        sched.terminate_children()
        sched.terminate_parent()


def try_possess_sched(hmmfile: HMMFile):
    try:
        Sched.possess(hmmfile)
    except CouldNotPossessError:
        return False
    return True


@app.command()
def ready(hmmfile: Path, wait: bool = O_WAIT):
    """
    Check if h3daemon is running and ready.
    """
    try:
        file = HMMFile(hmmfile)
        if wait:
            wait_until(partial(try_possess_sched, file))

        sched = Sched.possess(file)
        if wait:
            wait_until(sched.is_ready)

        try:
            if sched.is_ready():
                typer.echo("Daemon is ready!")
                raise typer.Exit(0)
            else:
                typer.echo("Daemon is NOT ready...")
                raise typer.Exit(1)
        except ParentNotAliveError:
            typer.echo("Daemon is NOT ready...")
            raise typer.Exit(1)

    except CouldNotPossessError as x:
        typer.echo(str(x))
        raise typer.Exit(1)
