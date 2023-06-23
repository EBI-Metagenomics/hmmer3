from pathlib import Path

import pytest

from hmmer_tables.output import read_output


def test_output(files_path: Path):
    x = read_output(files_path / "output.txt")
    assert len(x.queries) == 2

    qa = x.queries[1]
    assert len(qa.domains) == 3
    assert len(qa.domains[0].aligns) == 2

    align = qa.domains[0].aligns[0].align
    assert align.core_interval.start == 1
    assert align.core_interval.stop == 121
    assert align.query_interval.start == 1
    assert align.query_interval.stop == 118
    assert align.hmm_cs == (
        "EEEEEEHHHHHHHHHHHHTTSSSSTSSGGGGEEEEEEETTE"
        "EEEEEE-SSEEEEEEEESEECEE-EEEEEEHHHHHHHHH"
        "HSTTTSEEEEEEETTEEEEEEETTEEEEEE-BEGGGSC-XX"
    )
    assert align.hmm_rf == (
        "?????????????????????????????????????????"
        "???????????????????????????????????????"
        "?????????????????????????????????????????"
    )
    assert align.query_cs == (
        "mkftiereellkelekvakvisnRntlpiLsgvllevkdsq"
        "lsltgtDleisiestisaeseeeegkvlvparllldivkaL"
        "pndkkvkisvneqklliiksgssrfslstlsaeeypnlp"
    )
    assert align.match == (
        "mk+++ r  l ++++ v ++i++R+  piLs"
        "g+l+e+k+++++l+ +++e s+++++sa"
        "++  e gkvlvp+ l++ i ++Lp+ ++v+i"
        "s +++k +++ +g  ++ l+ ++  eyp lp"
    )
    assert align.query == (
        "MKLIVDRGLLSSSVNFVTRMIPARPPSPILSGMLIEAK"
        "NEKVTLSTFNYETSAKVEFSANVI-EGGKVLVPGTLIS"
        "SISQKLPD-EPVEISKEDEKISLT-CGPVNYILNLMPIAEYPPLP"
    )
    assert align.score == (
        "9*************************************"
        "***********************9.99***********"
        "********.***************.*****************886"
    )
    pass


def test_output_empty(files_path: Path):
    with pytest.raises(ValueError):
        read_output(files_path / "empty.txt")
