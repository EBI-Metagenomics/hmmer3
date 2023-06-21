import os
from pathlib import Path
from tempfile import NamedTemporaryFile

import pytest

from h3result.read_h3result import read_h3result


@pytest.fixture()
def files_path() -> Path:
    current_dir = os.path.dirname(__file__)
    return Path(current_dir) / "files"


class TempFile:
    def __init__(self):
        self._content = None
        self._stream = None

    def fileno(self):
        return self._stream.fileno()

    @property
    def content(self):
        assert isinstance(self._content, str)
        return self._content

    def __enter__(self):
        self._stream = NamedTemporaryFile(delete=False)

    def __exit__(self, *args, **kwargs):
        self._stream.close()
        self._content = open(self._stream.name).read()
        os.unlink(self._stream.name)


def test_targets(files_path: Path):
    tmp = TempFile()
    with tmp:
        x = read_h3result(files_path / "h3result.mp")
        x.print_targets(tmp.fileno())

    desired = open(files_path / "targets.txt").read()
    assert desired == tmp.content


def test_targets_table(files_path: Path):
    tmp = TempFile()
    with tmp:
        x = read_h3result(files_path / "h3result.mp")
        x.print_targets_table(tmp.fileno())

    desired = open(files_path / "targets_table.txt").read()
    assert desired == tmp.content


def test_domains(files_path: Path):
    tmp = TempFile()
    with tmp:
        x = read_h3result(files_path / "h3result.mp")
        x.print_domains(tmp.fileno())

    desired = open(files_path / "domains.txt").read()
    assert desired == tmp.content


def test_domains_table(files_path: Path):
    tmp = TempFile()
    with tmp:
        x = read_h3result(files_path / "h3result.mp")
        x.print_domains_table(tmp.fileno())

    desired = open(files_path / "domains_table.txt").read()
    assert desired == tmp.content


def test_open_via_fileno(files_path: Path):
    tmp = TempFile()
    with tmp:
        with open(files_path / "h3result.mp", "rb") as f:
            x = read_h3result(fileno=f.fileno())
            x.print_targets(tmp.fileno())

    desired = open(files_path / "targets.txt").read()
    assert desired == tmp.content
