from io import TextIOBase
from typing import Iterable, List

from pydantic import BaseModel, RootModel

from hmmer_tables.csv_iter import csv_iter
from hmmer_tables.path_like import PathLike

__all__ = ["TBLScore", "TBLRow", "TBLIndex", "TBLDom", "read_tbl", "TBL"]


class TBLIndex(BaseModel):
    name: str
    accession: str


class TBLScore(BaseModel):
    e_value: float
    score: float
    bias: float


class TBLDom(BaseModel):
    exp: float
    reg: int
    clu: int
    ov: int
    env: int
    dom: int
    rep: int
    inc: int


class TBLRow(BaseModel):
    target: TBLIndex
    query: TBLIndex
    full_sequence: TBLScore
    best_1_domain: TBLScore
    domain_numbers: TBLDom
    description: str


class TBL(RootModel):
    root: List[TBLRow]

    def __iter__(self):
        return iter(self.root)

    def __getitem__(self, item) -> TBLRow:
        return self.root[item]

    def __len__(self):
        return len(self.root)


def _read_tbl_stream(stream: Iterable[str]) -> TBL:
    rows: list[TBLRow] = []
    for x in csv_iter(stream):
        seq = TBLScore(e_value=float(x[4]), score=float(x[5]), bias=float(x[6]))
        dom = TBLScore(e_value=float(x[7]), score=float(x[8]), bias=float(x[9]))
        row = TBLRow(
            target=TBLIndex(name=x[0], accession=x[1]),
            query=TBLIndex(name=x[2], accession=x[3]),
            full_sequence=seq,
            best_1_domain=dom,
            domain_numbers=TBLDom(
                exp=float(x[10]),
                reg=int(x[11]),
                clu=int(x[12]),
                ov=int(x[13]),
                env=int(x[14]),
                dom=int(x[15]),
                rep=int(x[16]),
                inc=int(x[17]),
            ),
            description=" ".join(x[18:]),
        )
        rows.append(row)
    return TBL.model_validate(rows)


def read_tbl(filename: PathLike | None = None, stream: TextIOBase | None = None) -> TBL:
    """
    Read tbl file type.
    """
    if filename is not None:
        assert stream is None
        with open(filename, "r") as stream:
            return _read_tbl_stream(stream)
    else:
        assert stream is not None
        return _read_tbl_stream(stream)
