from pathlib import Path
from subprocess import check_call

import hmmer


def test_hmmpgmd():
    hmmpgmd = str(Path(hmmer.BIN_DIR) / "hmmpgmd")
    assert check_call([hmmpgmd, "-h"]) == 0
