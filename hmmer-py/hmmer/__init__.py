import os
import subprocess
import sys

DATA_DIR = os.path.join(os.path.dirname(__file__), "data")
BIN_DIR = os.path.join(DATA_DIR, "bin")


def _program(name, args):
    return subprocess.call([os.path.join(BIN_DIR, name)] + args, close_fds=False)


def alimask():
    raise SystemExit(_program("alimask", sys.argv[1:]))


def hmmalign():
    raise SystemExit(_program("hmmalign", sys.argv[1:]))


def hmmbuild():
    raise SystemExit(_program("hmmbuild", sys.argv[1:]))


def hmmc2():
    raise SystemExit(_program("hmmc2", sys.argv[1:]))


def hmmconvert():
    raise SystemExit(_program("hmmconvert", sys.argv[1:]))


def hmmemit():
    raise SystemExit(_program("hmmemit", sys.argv[1:]))


def hmmerfm_exactmatch():
    raise SystemExit(_program("hmmerfm-exactmatch", sys.argv[1:]))


def hmmfetch():
    raise SystemExit(_program("hmmfetch", sys.argv[1:]))


def hmmlogo():
    raise SystemExit(_program("hmmlogo", sys.argv[1:]))


def hmmpgmd():
    raise SystemExit(_program("hmmpgmd", sys.argv[1:]))


def hmmpgmd_shard():
    raise SystemExit(_program("hmmpgmd_shard", sys.argv[1:]))


def hmmpress():
    raise SystemExit(_program("hmmpress", sys.argv[1:]))


def hmmscan():
    raise SystemExit(_program("hmmscan", sys.argv[1:]))


def hmmsearch():
    raise SystemExit(_program("hmmsearch", sys.argv[1:]))


def hmmsim():
    raise SystemExit(_program("hmmsim", sys.argv[1:]))


def hmmstat():
    raise SystemExit(_program("hmmstat", sys.argv[1:]))


def jackhmmer():
    raise SystemExit(_program("jackhmmer", sys.argv[1:]))


def makehmmerdb():
    raise SystemExit(_program("makehmmerdb", sys.argv[1:]))


def nhmmer():
    raise SystemExit(_program("nhmmer", sys.argv[1:]))


def nhmmscan():
    raise SystemExit(_program("nhmmscan", sys.argv[1:]))


def phmmer():
    raise SystemExit(_program("phmmer", sys.argv[1:]))
