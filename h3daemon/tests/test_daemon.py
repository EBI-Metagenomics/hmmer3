import os
import shutil
from pathlib import Path

import pytest
from deciphon_schema import HMMFile

import h3daemon


@pytest.mark.repeat(30)
def test_hmmer(tmp_path, files_path: Path):
    os.chdir(tmp_path)
    shutil.copy(files_path / "minifam.hmm", Path("minifam.hmm"))
    hmmfile = HMMFile(path=Path("minifam.hmm"))
    pidfile = h3daemon.spawn(hmmfile, force=True)
    daemon = h3daemon.possess(pidfile, wait=True)
    daemon.shutdown()
