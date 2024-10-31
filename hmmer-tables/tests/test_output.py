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
    assert align.hmm_rf is None
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


def test_output2(files_path: Path):
    x = read_output(files_path / "output2.txt")
    assert len(x.queries) == 3
    head = x.queries[0].domains[2].head.split("\n")
    assert head[0].startswith(">> AAA  ATPase family associate")
    assert head[1].startswith("   [No individual domains")
    assert "[No hits detected that satisfy reporting thresholds]" in x.queries[2].head
    assert len(x.queries[2].domains) == 0


def test_output3(files_path: Path):
    x = read_output(files_path / "output3.txt")
    assert len(x.queries) == 1
    assert len(x.queries[0].domains) == 1
    assert len(x.queries[0].domains[0].aligns) == 1
    desired = (
        "D+++l+il++++l G+++G + +v+s++++v++++v  +l +++a+ l+    +p +   +a++ l+"
        "+++ liv+++ ++++k ++   l+ +Dr +Ga +Gl+kg+ll++ v++f++al++ ++ q ++ +S++"
    )
    assert x.queries[0].domains[0].aligns[0].align.match == desired


def test_output_empty(files_path: Path):
    with pytest.raises(ValueError):
        read_output(files_path / "empty.txt")


def test_output4(files_path: Path):
    x = read_output(files_path / "output4.txt")
    assert len(x.queries) == 1
    assert len(x.queries[0].domains) == 3
    assert len(x.queries[0].domains[0].aligns) == 1
    desired = (
        "g+rdllpee++k++++ +r+ ++fe++ + +V +P++ey++ +  ++ge+  +q++ + D+"
        " gr+l+lR+D+T +vaR++ + ++ +++p +++Y+++++r ++ ++gr re+ "
        "Q+G+El+G+++vead+ev++l++e+  a g+e++t+ +g"
        "   l+++ ++ lg++++    l++++++kd     ++a  ++e +l+++"
        " +e + al+e++g +e++   +++l   ++++e +++l el++ll+a++"
        "    + +Dl+  rg++Yyt +vF+a+a++  +   +++GGrYD+l+++f g+  pAtGf++ +e+l"
    )
    assert x.queries[0].domains[0].aligns[0].align.match == desired
