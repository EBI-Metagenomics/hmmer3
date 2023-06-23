from typing import Iterable

import more_itertools
from more_itertools import split_at
from pydantic import BaseModel

from hmmer_tables.cleanup import (
    _rstrip_newlines,
    _strip_empty_lines,
)
from hmmer_tables.path_like import PathLike
from hmmer_tables.query import QueryAnnotList, read_queries

__all__ = ["read_output", "Output"]


class Output(BaseModel):
    head: str
    queries: QueryAnnotList


def _consume_run_params(stream: Iterable[str]):
    x, tail = split_at(stream, lambda x: len(x) == 0, maxsplit=1)
    return x, tail


def _read_output_stream(stream: Iterable[str]):
    rows = _rstrip_newlines(stream)
    rows = list(more_itertools.rstrip(rows, lambda x: x == "[ok]"))
    rows = list(more_itertools.rstrip(rows, lambda x: x == "//"))
    head, rows = _consume_run_params(_strip_empty_lines(rows))
    return Output(head="\n".join(head), queries=read_queries(stream=rows))


def read_output(filename: PathLike | None = None, stream: Iterable[str] | None = None):
    """
    Read output file type.
    """
    if filename is not None:
        assert stream is None
        with open(filename, "r") as stream:
            return _read_output_stream(stream)
    else:
        return _read_output_stream(stream)
