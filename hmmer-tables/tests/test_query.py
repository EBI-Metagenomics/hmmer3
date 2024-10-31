from pathlib import Path

from hmmer_tables.query import read_query


def test_query(files_path: Path):
    x = read_query(files_path / "query.txt")

    assert x.head == ""
    assert len(x.domains) == 1

    assert x.domains[0].head == ">> PF07719.20  Tetratricopeptide repeat"
    assert len(x.domains[0].aligns) == 7

    desired = "  == domain 2  score: 27.3 bits;  conditional E-value: 1.3e-10"
    assert x.domains[0].aligns[1].head == desired

    assert x.domains[0].aligns[1].align.core_interval.start == 1
    assert x.domains[0].aligns[1].align.core_interval.stop == 33

    assert x.domains[0].aligns[1].align.query_interval.start == 113
    assert x.domains[0].aligns[1].align.query_interval.stop == 145

    desired = None
    assert x.domains[0].aligns[1].align.hmm_cs == desired
    assert x.domains[0].aligns[1].align.hmm_rf == desired

    query_name = "lcl|NZ_OU912926.1_cds_WP_239795528.1_168"
    profile = "PF07719.20"

    assert x.domains[0].aligns[0].align.query_name == query_name
    assert x.domains[0].aligns[0].align.profile == profile

    assert x.domains[0].aligns[1].align.query_name == query_name
    assert x.domains[0].aligns[1].align.profile == profile

    desired = "aealynlGlayyklgdyeeAleafekAleldPn"
    assert x.domains[0].aligns[1].align.query_cs == desired

    desired = "ae+++nlG+a+ ++g++e A+e f+++l l P+"
    assert x.domains[0].aligns[1].align.match == desired

    desired = "AEVHNNLGNALKEQGNLEAAIESFRRSLVLKPD"
    assert x.domains[0].aligns[1].align.query == desired

    desired = "79****************************998"
    assert x.domains[0].aligns[1].align.score == desired
