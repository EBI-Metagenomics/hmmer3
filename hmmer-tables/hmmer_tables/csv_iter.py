import csv
from typing import Iterable

__all__ = ["csv_iter"]


def csv_iter(stream: Iterable[str]):
    return csv.reader(uncomment(stream), delimiter=" ", skipinitialspace=True)


def uncomment(stream: Iterable[str]):
    for line in stream:
        if line.startswith("#"):
            continue
        if len(line) == 0:
            continue
        yield line
