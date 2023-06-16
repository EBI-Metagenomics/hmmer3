from pathlib import Path
from subprocess import check_call

import hmmer

__all__ = ["HMMFile"]

pressed_extensions = ["h3f", "h3i", "h3m", "h3p"]


class HMMFile:
    def __init__(self, file: Path):
        self._file = file.absolute()

        if not file.name.endswith(".hmm"):
            raise ValueError(f"`{file}` does not end with `.hmm`.")

        if not file.exists():
            raise ValueError(f"`{file}` does not exist.")

    def ensure_pressed(self):
        try:
            self._raise_on_missing_pressed_files()
        except ValueError:
            for x in pressed_extensions:
                Path(f"{self._file}.{x}").unlink(True)
            check_call([str(Path(hmmer.BIN_DIR) / "hmmpress"), str(self._file)])

    @property
    def _lockfile(self):
        return self.path.parent / f"{self.path}.lock"

    def _raise_on_missing_pressed_files(self):
        for x in pressed_extensions:
            filename = Path(f"{self._file}.{x}")
            if not filename.exists():
                raise ValueError(f"`{filename.name}` must exist as well.")

    def __str__(self):
        return str(self.path)

    @property
    def path(self) -> Path:
        return self._file
