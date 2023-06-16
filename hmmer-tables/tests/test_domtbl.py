from pathlib import Path

import pytest

from hmmer_tables.domtbl import read_domtbl


def test_domtbl(files_path: Path):
    tbl = iter(read_domtbl(files_path / "domtbl.txt"))
    row = next(tbl)

    assert row.target.name == "Leader_Thr"
    assert row.target.accession == "PF08254.12"
    assert row.target.length == 22
    assert row.query.name == "AE014075.1:190-252|amino|11"
    assert row.query.accession == "-"
    assert row.query.length == 21
    assert row.full_sequence.e_value == 5.3e-10
    assert row.full_sequence.score == 38.8
    assert row.full_sequence.bias == 17.5
    assert row.domain.id == 1
    assert row.domain.size == 1
    assert row.domain.c_value == 3e-14
    assert row.domain.i_value == 5.5e-10
    assert row.domain.score == 38.8
    assert row.domain.bias == 17.5

    assert row.hmm_coord.start == 1
    assert row.hmm_coord.stop == 22
    assert row.hmm_coord.interval.start == 0
    assert row.hmm_coord.interval.stop == 22

    assert row.ali_coord.start == 1
    assert row.ali_coord.stop == 21
    assert row.ali_coord.interval.start == 0
    assert row.ali_coord.interval.stop == 21

    assert row.env_coord.start == 1
    assert row.env_coord.stop == 21
    assert row.env_coord.interval.start == 0
    assert row.env_coord.interval.stop == 21

    assert row.acc == 0.99
    assert row.description == "Threonine leader peptide"

    row = next(tbl)
    assert row.target.name == "Y1_Tnp"

    assert len(read_domtbl(files_path / "domtbl.txt")) == 8


def test_domtbl_empty(files_path: Path):
    tbl = iter(read_domtbl(files_path / "empty.txt"))
    with pytest.raises(StopIteration):
        next(tbl)
