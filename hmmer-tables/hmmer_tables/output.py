from io import TextIOBase
from typing import Iterable

import more_itertools
from more_itertools import split_at
from pydantic import BaseModel

from hmmer_tables.cleanup import (
    rstrip_newlines,
    strip_empty_lines,
)
from hmmer_tables.path_like import PathLike
from hmmer_tables.query import QueryAnnotList, parse_query

__all__ = ["read_output", "Output"]


class Output(BaseModel):
    head: str
    queries: QueryAnnotList


def _consume_run_params(stream: Iterable[str]):
    x, tail = split_at(stream, lambda x: len(x) == 0, maxsplit=1)
    return x, tail


def _split_queries(stream: Iterable[str]):
    return list(split_at(stream, lambda x: x == "//"))


def _read_output_stream(stream: Iterable[str]):
    rows = rstrip_newlines(stream)
    rows = list(more_itertools.rstrip(rows, lambda x: x == "[ok]"))
    rows = list(more_itertools.rstrip(rows, lambda x: x == "//"))
    head, rows = _consume_run_params(strip_empty_lines(rows))
    rows = strip_empty_lines(rows)
    queries = _split_queries(rows)
    query_annotations = QueryAnnotList.model_validate(parse_query(x) for x in queries)
    return Output(head="\n".join(head), queries=query_annotations)


def read_output(filename: PathLike | None = None, stream: TextIOBase | None = None):
    """
    Read output file type.
    """
    if filename is not None:
        assert stream is None
        with open(filename, "r") as stream:
            return _read_output_stream(stream)
    else:
        assert stream is not None
        return _read_output_stream(stream)
