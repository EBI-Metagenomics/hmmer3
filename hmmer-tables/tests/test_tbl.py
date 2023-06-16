from pathlib import Path

import pytest

from hmmer_tables.tbl import read_tbl


def test_tbl(files_path: Path):
    tbl = iter(read_tbl(files_path / "tbl.txt"))
    row = next(tbl)

    assert row.target.name == "item2"
    assert row.target.accession == "-"
    assert row.query.name == "Octapeptide"
    assert row.query.accession == "PF03373.14"
    assert row.full_sequence.e_value == 1.2e-07
    assert row.full_sequence.score == 19.5
    assert row.full_sequence.bias == 3.5
    assert row.best_1_domain.e_value == 1.2e-07
    assert row.best_1_domain.score == 19.5
    assert row.best_1_domain.bias == 3.5
    assert row.domain_numbers.exp == 1.0
    assert row.domain_numbers.reg == 1
    assert row.domain_numbers.clu == 0
    assert row.domain_numbers.ov == 0
    assert row.domain_numbers.env == 1
    assert row.domain_numbers.dom == 1
    assert row.domain_numbers.rep == 1
    assert row.domain_numbers.inc == 1
    assert row.description == "Description one two three"

    row = next(tbl)

    assert row.target.name == "item3"
    assert row.target.accession == "-"
    assert row.query.name == "Octapeptide"
    assert row.query.accession == "PF03373.14"
    assert row.full_sequence.e_value == 1.2e-07
    assert row.full_sequence.score == 19.5
    assert row.full_sequence.bias == 3.5
    assert row.best_1_domain.e_value == 1.2e-07
    assert row.best_1_domain.score == 19.5
    assert row.best_1_domain.bias == 3.5
    assert row.domain_numbers.exp == 1.0
    assert row.domain_numbers.reg == 1
    assert row.domain_numbers.clu == 0
    assert row.domain_numbers.ov == 0
    assert row.domain_numbers.env == 1
    assert row.domain_numbers.dom == 1
    assert row.domain_numbers.rep == 1
    assert row.domain_numbers.inc == 1
    assert row.description == "-"

    with pytest.raises(StopIteration):
        next(tbl)


def test_tbl_empty(files_path: Path):
    tbl = iter(read_tbl(files_path / "empty.txt"))
    with pytest.raises(StopIteration):
        next(tbl)
