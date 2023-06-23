from functools import reduce
from itertools import dropwhile
from typing import Iterable, List

import more_itertools
from more_itertools import split_at, split_before
from pydantic import BaseModel, RootModel

from hmmer_tables.interval import RInterval
from hmmer_tables.path_like import PathLike

__all__ = [
    "read_output",
    "Align",
    "DomAlign",
    "DomAlignList",
    "DomAnnot",
    "DomAnnotList",
    "QueryAnnot",
    "QueryAnnotList",
    "Output",
]


class Align(BaseModel):
    core_interval: RInterval
    query_interval: RInterval
    hmm_cs: str
    hmm_rf: str
    query_cs: str
    match: str
    query: str
    score: str


class DomAlign(BaseModel):
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
    head: str
    domains: DomAnnotList


class QueryAnnotList(RootModel):
    root: List[QueryAnnot]

    def __iter__(self):
        return iter(self.root)

    def __getitem__(self, item) -> QueryAnnot:
        return self.root[item]

    def __len__(self):
        return len(self.root)


class Output(BaseModel):
    head: str
    queries: QueryAnnotList


def remove_multispace(data: str):
    return " ".join(data.split())


DOM_ANNOT_ANNOUNCE = "Domain annotation for each model (and alignments):"
PIPE_SUMMARY = "Internal pipeline statistics summary:"
ALIGN_ANNOUNCE = "  Alignments for each domain:"
DOMAIN_ANNOUNCE = "  == domain"


def _split_queries(stream: Iterable[str]):
    return list(split_at(stream, lambda x: x == "//"))


def _rstrip_newlines(stream: Iterable[str]):
    return [x.rstrip("\n") for x in stream]


def _strip_empty_lines(items: Iterable[str]):
    return list(more_itertools.strip(items, lambda x: len(x) == 0))


def _parse_target_consensus(row: str):
    line = row

    row = remove_multispace(row)
    if row.count(" ") == 3:
        start, tgt_cs, stop = row.split()[1:]
    else:
        start, tgt_cs, stop = row.split()

    x = line.find(tgt_cs)
    y = x + len(tgt_cs)
    core_interval = RInterval(start=int(start), stop=int(stop))

    return slice(x, y), line[slice(x, y)], core_interval


def _parse_match(row: str, line_slice: slice):
    return row[line_slice]


def _parse_target(row: str, line_slice: slice):
    tgt = row[line_slice]
    start = int(row[: line_slice.start].rstrip().split()[-1])
    stop = int(row[line_slice.stop :])
    query_interval = RInterval(start=int(start), stop=int(stop))
    return tgt, query_interval


def _parse_align_chunk(stream: Iterable[str]):
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

    line_slice, tgt_cs, core_interval = _parse_target_consensus(row)
    match = _parse_match(next(it), line_slice)
    tgt, query_interval = _parse_target(next(it), line_slice)

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
    return Align(
        core_interval=core_interval,
        query_interval=query_interval,
        hmm_cs=x.hmm_cs + y.hmm_cs,
        hmm_rf=x.hmm_rf + y.hmm_rf,
        query_cs=x.query_cs + y.query_cs,
        match=x.match + y.match,
        query=x.query + y.query,
        score=x.score + y.score,
    )


def _parse_align(stream: Iterable[str]):
    head, body = stream[0], stream[1:]
    aligns = [_parse_align_chunk(i) for i in split_at(body, lambda x: len(x) == 0)]
    return DomAlign(head=head, align=reduce(_merge_aligns_pair, aligns))


def _consume_run_params(stream: Iterable[str]):
    x, tail = split_at(stream, lambda x: len(x) == 0, maxsplit=1)
    return x, tail


def _split_at_consecutive_empty_lines(stream: Iterable[str], count=-1):
    rows = "\n".join(stream).replace("\n\n\n", "\n@@\n", count).split("\n")
    return list(split_at(rows, lambda x: x == "@@"))


def _parse_annot(stream: Iterable[str]):
    head, tail = split_before(stream, lambda x: x.startswith(ALIGN_ANNOUNCE), 1)
    head = _strip_empty_lines(head)
    tail = list(dropwhile(lambda x: x == ALIGN_ANNOUNCE, tail))
    aligns = list(split_before(tail, lambda x: x.startswith(DOMAIN_ANNOUNCE)))
    aligns = [_strip_empty_lines(x) for x in aligns]
    return DomAnnot(head=head[0], aligns=[_parse_align(x) for x in aligns])


def _parse_annots(stream: Iterable[str]):
    annots = list(split_before(stream, lambda x: x.startswith(">> ")))
    return [_parse_annot(_strip_empty_lines(x)) for x in annots]


def _parse_query(stream: Iterable[str]):
    rows = _strip_empty_lines(stream)
    scores, tail = _split_at_consecutive_empty_lines(rows, 1)
    tail = list(dropwhile(lambda x: x == DOM_ANNOT_ANNOUNCE, tail))
    annots, stat = split_before(tail, lambda x: x == PIPE_SUMMARY, 1)
    annots = _strip_empty_lines(annots)
    stat = _strip_empty_lines(stat)
    return QueryAnnot(head="\n".join(scores), domains=_parse_annots(annots))


def _read_output_stream(stream: Iterable[str]):
    rows = _rstrip_newlines(stream)
    rows = list(more_itertools.rstrip(rows, lambda x: x == "[ok]"))
    rows = list(more_itertools.rstrip(rows, lambda x: x == "//"))
    head, rows = _consume_run_params(_strip_empty_lines(rows))
    rows = _strip_empty_lines(rows)
    queries = _split_queries(rows)
    query_annotations = [_parse_query(x) for x in queries]
    return Output(head="\n".join(head), queries=query_annotations)


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
