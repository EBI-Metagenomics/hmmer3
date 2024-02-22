import dataclasses
from io import TextIOBase
from typing import Iterable, List

from deciphon_intervals import PyInterval, RInterval
from pydantic import BaseModel, RootModel

from hmmer_tables.csv_iter import csv_iter
from hmmer_tables.path_like import PathLike

__all__ = [
    "DomTBLCoord",
    "DomTBLDomScore",
    "DomTBLIndex",
    "DomTBLRow",
    "DomTBLSeqScore",
    "DomTBL",
    "read_domtbl",
]


class DomTBLIndex(BaseModel):
    name: str
    accession: str
    length: int


class DomTBLSeqScore(BaseModel):
    e_value: float
    score: float
    bias: float


class DomTBLDomScore(BaseModel):
    id: int
    size: int
    c_value: float
    i_value: float
    score: float
    bias: float


class DomTBLCoord(BaseModel):
    """
    Coordinates.

    Parameters
    ----------
    start
        Start coordinate, starting from 1. Consider using :method:`.interval` instead.
    stop
        Stop coordinate. `(start, stop)` form a closed interval. Consider using
        :method:`.interval` instead.
    """

    start: int
    stop: int

    @property
    def interval(self) -> PyInterval:
        """
        0-start, half-open interval.

        Returns
        -------
        PyInterval
            Interval.
        """
        rinterval = RInterval(start=self.start, stop=self.stop)
        return rinterval.py


class DomTBLRow(BaseModel):
    target: DomTBLIndex
    query: DomTBLIndex
    full_sequence: DomTBLSeqScore
    domain: DomTBLDomScore
    hmm_coord: DomTBLCoord
    ali_coord: DomTBLCoord
    env_coord: DomTBLCoord
    acc: float
    description: str

    def _asdict(self):
        return dataclasses.asdict(self)

    def _field_types(self):
        return {f.name: f.type for f in dataclasses.fields(self)}


class DomTBL(RootModel):
    root: List[DomTBLRow]

    def __iter__(self):
        return iter(self.root)

    def __getitem__(self, item) -> DomTBLRow:
        return self.root[item]

    def __len__(self):
        return len(self.root)


def _read_domtbl_stream(stream: Iterable[str]):
    rows = []
    for x in csv_iter(stream):
        seq_score = DomTBLSeqScore(
            e_value=float(x[6]), score=float(x[7]), bias=float(x[8])
        )
        domain = DomTBLDomScore(
            id=int(x[9]),
            size=int(x[10]),
            c_value=float(x[11]),
            i_value=float(x[12]),
            score=float(x[13]),
            bias=float(x[14]),
        )
        row = DomTBLRow(
            target=DomTBLIndex(name=x[0], accession=x[1], length=int(x[2])),
            query=DomTBLIndex(name=x[3], accession=x[4], length=int(x[5])),
            full_sequence=seq_score,
            domain=domain,
            hmm_coord=DomTBLCoord(start=int(x[15]), stop=int(x[16])),
            ali_coord=DomTBLCoord(start=int(x[17]), stop=int(x[18])),
            env_coord=DomTBLCoord(start=int(x[19]), stop=int(x[20])),
            acc=float(x[21]),
            description=" ".join(x[22:]),
        )
        rows.append(row)
    return DomTBL.model_validate(rows)


def read_domtbl(
    filename: PathLike | None = None,
    stream: TextIOBase | None = None,
) -> DomTBL:
    """
    Read domtbl file type.
    """
    if filename is not None:
        assert stream is None
        with open(filename, "r") as stream:
            return _read_domtbl_stream(stream)
    else:
        assert stream is not None
        return _read_domtbl_stream(stream)
