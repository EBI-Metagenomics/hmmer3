from typing import Iterable

import more_itertools
from more_itertools import split_at


def _strip_empty_lines(items: Iterable[str]):
    return list(more_itertools.strip(items, lambda x: len(x) == 0))


def _split_queries(stream: Iterable[str]):
    return list(split_at(stream, lambda x: x == "//"))


def _split_at_consecutive_empty_lines(stream: Iterable[str], count=-1):
    rows = "\n".join(stream).replace("\n\n\n", "\n@@\n", count).split("\n")
    return list(split_at(rows, lambda x: x == "@@"))


def _rstrip_newlines(stream: Iterable[str]):
    return [x.rstrip("\n") for x in stream]


def remove_multispace(data: str):
    return " ".join(data.split())
