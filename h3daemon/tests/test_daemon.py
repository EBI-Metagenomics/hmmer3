import os
import shutil
from pathlib import Path

import pytest
from deciphon_schema import HMMFile

import h3daemon


@pytest.mark.repeat(30)
def test_hmmer(tmp_path, files_path: Path):
    os.chdir(tmp_path)
    debug_file = Path(tmp_path / "h3daemon_debug.txt")
    debug_file.touch()

    os.environ["H3DAEMON_DEBUG"] = "1"
    os.environ["H3DAEMON_DEBUG_FILE"] = str(debug_file)

    shutil.copy(files_path / "minifam.hmm", Path("minifam.hmm"))

    try:
        hmmfile = HMMFile(path=Path("minifam.hmm"))
        pidfile = h3daemon.spawn(hmmfile, detach=False, force=True)
        daemon = h3daemon.possess(pidfile)
        daemon.wait_for_readiness()
        print(f"daemon.port(): {daemon.port()}")
        daemon.shutdown()
    finally:
        with open(debug_file, "r") as f:
            print(f.read())
