import importlib.metadata
from pathlib import Path
from typing import Optional

from typer import Argument, Exit, Option, Typer, echo

from hmmer_tables.output import read_output

__all__ = ["app"]

app = Typer(
    add_completion=False,
    pretty_exceptions_short=True,
    pretty_exceptions_show_locals=False,
)


@app.callback(invoke_without_command=True)
def cli(version: Optional[bool] = Option(None, "--version", is_eager=True)):
    if version:
        assert __package__ is not None
        echo(importlib.metadata.version(__package__))
        raise Exit(0)


@app.command()
def query(
    hmmfile: Path = Argument(
        ...,
        exists=True,
        file_okay=True,
        dir_okay=False,
        readable=True,
        help="HMMER output file.",
    ),
    profile: str = Option("", help="Profile name."),
    query: str = Option("", help="Query name."),
):
    """
    Fetch and display hits.
    """

    output = read_output(filename=hmmfile)

    for q in output.queries:
        for domain in q.domains:
            for align in domain.aligns:
                if len(profile) > 0 and align.align.profile != profile:
                    continue
                if len(query) > 0 and align.align.query_name != query:
                    continue
                print(q.raw)


@app.command()
def domain(
    hmmfile: Path = Argument(
        ...,
        exists=True,
        file_okay=True,
        dir_okay=False,
        readable=True,
        help="HMMER output file.",
    ),
    profile: str = Option("", help="Profile name."),
    query: str = Option("", help="Query name."),
):
    """
    Fetch and display hits.
    """

    output = read_output(filename=hmmfile)

    for q in output.queries:
        for domain in q.domains:
            if len(domain.aligns) == 0:
                continue
            x = domain.aligns[0]
            if len(profile) > 0 and x.align.profile != profile:
                continue
            if len(query) > 0 and x.align.query_name != query:
                continue
            print(x.raw)
