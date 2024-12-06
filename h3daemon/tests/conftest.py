from pathlib import Path

import pytest


@pytest.fixture
def files_path() -> Path:
    return Path(__file__).parent / "files"
