from pathlib import Path
from subprocess import DEVNULL, check_call

import hmmer
from deciphon_schema import HMMFile

__all__ = ["ensure_pressed"]

pressed_extensions = ["h3f", "h3i", "h3m", "h3p"]


def ensure_pressed(hmmfile: HMMFile):
    for x in pressed_extensions:
        filename = Path(f"{hmmfile.path}.{x}")
        if not filename.exists():
            for x in pressed_extensions:
                Path(f"{hmmfile.path}.{x}").unlink(True)
            check_call(
                [hmmer.path(hmmer.hmmpress), "-f", str(hmmfile.path)],
                stdout=DEVNULL,
                stderr=DEVNULL,
            )
