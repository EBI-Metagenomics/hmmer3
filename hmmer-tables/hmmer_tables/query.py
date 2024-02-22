from functools import reduce
from io import TextIOBase
from itertools import dropwhile
from typing import Iterable, List

from deciphon_intervals import RInterval
from more_itertools import split_at, split_before
from pydantic import BaseModel, RootModel

from hmmer_tables.cleanup import (
    remove_multispace,
    rstrip_newlines,
    strip_empty_lines,
)
from hmmer_tables.path_like import PathLike

__all__ = [
    "Align",
    "DomAlign",
    "DomAlignList",
    "DomAnnot",
    "DomAnnotList",
    "QueryAnnot",
    "QueryAnnotList",
    "parse_query",
]


class Align(BaseModel):
    core_interval: RInterval
    query_interval: RInterval
    profile: str
    query_name: str
    hmm_cs: str
    hmm_rf: str
    query_cs: str
    match: str
    query: str
    score: str

    @property
    def core_positions(self):
        positions: list[int] = []
        offset = self.core_interval.start - 1
        for i, c in enumerate(self.query_cs):
            if c != ".":
                offset += 1
            positions.append(offset)
        assert positions[-1] == self.core_interval.stop
        return positions


class DomAlign(BaseModel):
    raw: str
    head: str
    align: Align


class DomAlignList(RootModel):
    root: List[DomAlign]

    def __iter__(self):
        return iter(self.root)

    def __getitem__(self, item) -> DomAlign:
        return self.root[item]

    def __len__(self):
        return len(self.root)


class DomAnnot(BaseModel):
    head: str
    aligns: DomAlignList


class DomAnnotList(RootModel):
    root: List[DomAnnot]

    def __iter__(self):
        return iter(self.root)

    def __getitem__(self, item) -> DomAnnot:
        return self.root[item]

    def __len__(self):
        return len(self.root)


class QueryAnnot(BaseModel):
    raw: str
    head: str
    stat: str
    domains: DomAnnotList


class QueryAnnotList(RootModel):
    root: List[QueryAnnot]

    def __iter__(self):
        return iter(self.root)

    def __getitem__(self, item) -> QueryAnnot:
        return self.root[item]

    def __len__(self):
        return len(self.root)


ALIGN_ANNOUNCE = "  Alignments for each domain:"
DOM_ANNOT_ANNOUNCE = "Domain annotation for each model (and alignments):"
PIPE_SUMMARY = "Internal pipeline statistics summary:"
DOMAIN_ANNOUNCE = "  == domain"
NO_IND_DOM = (
    "   [No individual domains that satisfy reporting"
    " thresholds (although complete target did)]"
)
NOT_HIT = "   [No hits detected that satisfy reporting thresholds]"
NO_TGT_DETECT = "   [No targets detected that satisfy reporting thresholds]"


def _parse_target_consensus(row: str):
    line = row

    row = remove_multispace(row)
    profile = ""
    if row.count(" ") == 3:
        profile, start, tgt_cs, stop = row.split()
    else:
        start, tgt_cs, stop = row.split()

    y = line.rstrip().rindex(" ")
    x = line[:y].rindex(" ") + 1
    core_interval = RInterval(start=int(start), stop=int(stop))

    return slice(x, y), line[slice(x, y)], core_interval, profile


def _parse_match(row: str, line_slice: slice):
    return row[line_slice]


def _parse_target(row: str, line_slice: slice, last_pos: int):
    tgt = row[line_slice]
    vals = row[: line_slice.start].rstrip().split()
    start = vals[-1]
    query_name = vals[0] if len(vals) > 1 else ""
    stop = row[line_slice.stop :].strip()
    if start == "-":
        assert stop == "-"
        start = str(last_pos)
        stop = str(last_pos - 1)
    query_interval = RInterval(start=int(start), stop=int(stop))
    return tgt, query_name, query_interval


def _parse_align_chunk(stream: Iterable[str], last_pos: int):
    it = iter(stream)
    hmm_cs = ""
    hmm_rf = ""

    while True:
        row = next(it)
        if row.endswith(" CS"):
            hmm_cs = row[: row.rfind(" ")].strip()
        elif row.endswith(" RF"):
            hmm_rf = row[: row.rfind(" ")].strip()
        else:
            break

    line_slice, tgt_cs, core_interval, profile = _parse_target_consensus(row)
    match = _parse_match(next(it), line_slice)
    tgt, query_name, query_interval = _parse_target(next(it), line_slice, last_pos)

    row = next(it)
    assert row.endswith(" PP")
    score = row[line_slice]

    hmm_cs = "?" * len(tgt_cs) if len(hmm_cs) == 0 else hmm_cs
    hmm_rf = "?" * len(tgt_cs) if len(hmm_rf) == 0 else hmm_rf

    assert len(hmm_cs) == len(hmm_rf) == len(tgt_cs)
    assert len(tgt_cs) == len(match) == len(tgt) == len(score)

    return Align(
        core_interval=core_interval,
        query_interval=query_interval,
        profile=profile,
        query_name=query_name,
        hmm_cs=hmm_cs,
        hmm_rf=hmm_rf,
        query_cs=tgt_cs,
        match=match,
        query=tgt,
        score=score,
    )


def _merge_aligns_pair(x: Align, y: Align):
    assert x.core_interval.stop + 1 == y.core_interval.start
    assert x.query_interval.stop + 1 == y.query_interval.start
    core_interval = RInterval(start=x.core_interval.start, stop=y.core_interval.stop)
    query_interval = RInterval(start=x.query_interval.start, stop=y.query_interval.stop)
    assert x.profile == y.profile
    assert x.query_name == y.query_name
    return Align(
        core_interval=core_interval,
        query_interval=query_interval,
        profile=x.profile,
        query_name=x.query_name,
        hmm_cs=x.hmm_cs + y.hmm_cs,
        hmm_rf=x.hmm_rf + y.hmm_rf,
        query_cs=x.query_cs + y.query_cs,
        match=x.match + y.match,
        query=x.query + y.query,
        score=x.score + y.score,
    )


def _parse_align(stream: Iterable[str]):
    head, *body = stream
    aligns = []
    last_pos = 1
    for i in split_at(body, lambda x: len(x) == 0):
        x = _parse_align_chunk(i, last_pos)
        aligns.append(x)
        last_pos = x.query_interval.stop + 1
    return DomAlign(
        raw="\n".join(stream), head=head, align=reduce(_merge_aligns_pair, aligns)
    )


def _parse_annot(stream: Iterable[str]):
    rows = list(stream)
    if len(rows) > 1:
        if rows[1] == NO_IND_DOM:
            head = "\n".join(rows)
            return DomAnnot(head=head, aligns=DomAlignList(root=[]))
    xy = list(split_before(rows, lambda x: x.startswith(ALIGN_ANNOUNCE), 1))
    head = strip_empty_lines(xy[0] if len(xy) > 1 else [""])
    tail = xy[-1]
    tail = list(dropwhile(lambda x: x == ALIGN_ANNOUNCE, tail))
    aligns = list(split_before(tail, lambda x: x.startswith(DOMAIN_ANNOUNCE)))
    aligns = [strip_empty_lines(x) for x in aligns]
    return DomAnnot(
        head=head[0],
        aligns=DomAlignList.model_validate(_parse_align(x) for x in aligns),
    )


def _parse_annots(stream: Iterable[str]):
    annots = list(split_before(stream, lambda x: x.startswith(">> ")))
    return DomAnnotList.model_validate(
        _parse_annot(strip_empty_lines(x)) for x in annots
    )


def parse_query(stream: Iterable[str]):
    rows = strip_empty_lines(stream)

    xy = list(split_at(rows, lambda x: x == DOM_ANNOT_ANNOUNCE, 1))
    scores = xy[0] if len(xy) > 1 else ""
    tail = xy[-1]

    scores = strip_empty_lines(scores)
    tail = strip_empty_lines(tail)

    tail = list(dropwhile(lambda x: x == DOM_ANNOT_ANNOUNCE, tail))
    xy = list(split_before(tail, lambda x: x == PIPE_SUMMARY, 1))
    annots = xy[0]
    stat = xy[1] if len(xy) > 1 else ""
    annots = strip_empty_lines(annots)
    stat = "\n".join(strip_empty_lines(stat))

    if len(annots) == 1 and annots[0] == NO_TGT_DETECT:
        return QueryAnnot(
            raw="\n".join(rows),
            head="\n".join(scores),
            stat=stat,
            domains=DomAnnotList(root=[]),
        )

    return QueryAnnot(
        raw="\n".join(rows),
        head="\n".join(scores),
        stat=stat,
        domains=_parse_annots(annots),
    )


def _read_query(stream: Iterable[str]):
    rows = rstrip_newlines(stream)
    rows = strip_empty_lines(rows)
    return parse_query(rows)


def read_query(filename: PathLike | None = None, stream: TextIOBase | None = None):
    """
    Read query.
    """
    if filename is not None:
        assert stream is None
        with open(filename, "r") as stream:
            return _read_query(stream)
    else:
        assert stream is not None
        return _read_query(stream)
