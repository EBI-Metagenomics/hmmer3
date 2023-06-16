import csv
from io import TextIOWrapper

__all__ = ["csv_iter"]


def csv_iter(stream: TextIOWrapper):
    return csv.reader(uncomment(stream), delimiter=" ", skipinitialspace=True)


def uncomment(stream: TextIOWrapper):
    for line in stream:
        if line.startswith("#"):
            continue
        yield line
