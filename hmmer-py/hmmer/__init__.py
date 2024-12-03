import os
import pkgutil
import stat
import tempfile
from enum import Enum
import subprocess
import sys

__all__ = [
    "path",
    "Program",
    "alimask",
    "hmmalign",
    "hmmbuild",
    "hmmc2",
    "hmmconvert",
    "hmmemit",
    "hmmerfm_exactmatch",
    "hmmfetch",
    "hmmlogo",
    "hmmpgmd",
    "hmmpgmd_shard",
    "hmmpress",
    "hmmscan",
    "hmmsearch",
    "hmmsim",
    "hmmstat",
    "jackhmmer",
    "makehmmerdb",
    "nhmmer",
    "nhmmscan",
    "phmmer",
]

DATA_DIR = os.path.join(os.path.dirname(__file__), "data")
BIN_DIR = os.path.join(DATA_DIR, "bin")


class Program(Enum):
    alimask = "alimask"
    hmmalign = "hmmalign"
    hmmbuild = "hmmbuild"
    hmmc2 = "hmmc2"
    hmmconvert = "hmmconvert"
    hmmemit = "hmmemit"
    hmmerfm_exactmatch = "hmmerfm-exactmatch"
    hmmfetch = "hmmfetch"
    hmmlogo = "hmmlogo"
    hmmpgmd = "hmmpgmd"
    hmmpgmd_shard = "hmmpgmd_shard"
    hmmpress = "hmmpress"
    hmmscan = "hmmscan"
    hmmsearch = "hmmsearch"
    hmmsim = "hmmsim"
    hmmstat = "hmmstat"
    jackhmmer = "jackhmmer"
    makehmmerdb = "makehmmerdb"
    nhmmer = "nhmmer"
    nhmmscan = "nhmmscan"
    phmmer = "phmmer"


def _fetch_content(program: Program):
    data = pkgutil.get_data("hmmer", f"data/bin/{program.value}")
    assert data is not None
    return data


_program_path: dict[Program, str] = {}


def path(program: Program):
    if program not in _program_path:
        file = tempfile.NamedTemporaryFile(suffix=f"_{program.value}", delete=False)
        file.write(_fetch_content(program))
        file.close()
        st = os.stat(file.name)
        os.chmod(file.name, st.st_mode | stat.S_IEXEC)
        _program_path[program] = file.name
    return _program_path[program]


alimask = Program.alimask
hmmalign = Program.hmmalign
hmmbuild = Program.hmmbuild
hmmc2 = Program.hmmc2
hmmconvert = Program.hmmconvert
hmmemit = Program.hmmemit
hmmerfm_exactmatch = Program.hmmerfm_exactmatch
hmmfetch = Program.hmmfetch
hmmlogo = Program.hmmlogo
hmmpgmd = Program.hmmpgmd
hmmpgmd_shard = Program.hmmpgmd_shard
hmmpress = Program.hmmpress
hmmscan = Program.hmmscan
hmmsearch = Program.hmmsearch
hmmsim = Program.hmmsim
hmmstat = Program.hmmstat
jackhmmer = Program.jackhmmer
makehmmerdb = Program.makehmmerdb
nhmmer = Program.nhmmer
nhmmscan = Program.nhmmscan
phmmer = Program.phmmer


def _program(program: Program, args):
    return subprocess.call([path(program)] + args, close_fds=False)


def alimask_entry():
    raise SystemExit(_program(Program.alimask, sys.argv[1:]))


def hmmalign_entry():
    raise SystemExit(_program(Program.hmmalign, sys.argv[1:]))


def hmmbuild_entry():
    raise SystemExit(_program(Program.hmmbuild, sys.argv[1:]))


def hmmc2_entry():
    raise SystemExit(_program(Program.hmmc2, sys.argv[1:]))


def hmmconvert_entry():
    raise SystemExit(_program(Program.hmmconvert, sys.argv[1:]))


def hmmemit_entry():
    raise SystemExit(_program(Program.hmmemit, sys.argv[1:]))


def hmmerfm_exactmatch_entry():
    raise SystemExit(_program(Program.hmmerfm_exactmatch, sys.argv[1:]))


def hmmfetch_entry():
    raise SystemExit(_program(Program.hmmfetch, sys.argv[1:]))


def hmmlogo_entry():
    raise SystemExit(_program(Program.hmmlogo, sys.argv[1:]))


def hmmpgmd_entry():
    raise SystemExit(_program(Program.hmmpgmd, sys.argv[1:]))


def hmmpgmd_shard_entry():
    raise SystemExit(_program(Program.hmmpgmd_shard, sys.argv[1:]))


def hmmpress_entry():
    raise SystemExit(_program(Program.hmmpress, sys.argv[1:]))


def hmmscan_entry():
    raise SystemExit(_program(Program.hmmscan, sys.argv[1:]))


def hmmsearch_entry():
    raise SystemExit(_program(Program.hmmsearch, sys.argv[1:]))


def hmmsim_entry():
    raise SystemExit(_program(Program.hmmsim, sys.argv[1:]))


def hmmstat_entry():
    raise SystemExit(_program(Program.hmmstat, sys.argv[1:]))


def jackhmmer_entry():
    raise SystemExit(_program(Program.jackhmmer, sys.argv[1:]))


def makehmmerdb_entry():
    raise SystemExit(_program(Program.makehmmerdb, sys.argv[1:]))


def nhmmer_entry():
    raise SystemExit(_program(Program.nhmmer, sys.argv[1:]))


def nhmmscan_entry():
    raise SystemExit(_program(Program.nhmmscan, sys.argv[1:]))


def phmmer_entry():
    raise SystemExit(_program(Program.phmmer, sys.argv[1:]))
