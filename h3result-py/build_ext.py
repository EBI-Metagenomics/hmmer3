import os
import shutil
from pathlib import Path
from subprocess import check_call

from cffi import FFI
from git import Repo

CWD = Path(".").resolve()
TMP = CWD / ".build_ext"
PKG = CWD / "h3result"
INTERFACE = PKG / "interface.h"
LIB = PKG / "lib"
INC = PKG / "include"

SHARE = PKG / "share"


def make(args: list[str] = [], cwd: Path = CWD):
    check_call(["make"] + args, cwd=cwd)


def build_and_install(root: Path, prefix: str, prj_dir: str, git_url: str):
    git_dir = git_url.split("/")[-1].split(".")[-1][:-4]
    if not (root / git_dir).exists():
        Repo.clone_from(git_url, root / git_dir, depth=1)
    make(cwd=root / prj_dir)
    make(["install", f"PREFIX={prefix}"], cwd=root / prj_dir)


if __name__ == "__main__":
    shutil.rmtree(TMP, ignore_errors=True)
    os.makedirs(TMP, exist_ok=True)

    url = "https://github.com/EBI-Metagenomics/lite-pack.git"
    build_and_install(TMP / "lite-pack", str(PKG), ".", url)

    url = "https://github.com/EBI-Metagenomics/lite-pack.git"
    build_and_install(TMP / "lite-pack", str(PKG), "ext/", url)

    url = "https://github.com/EBI-Metagenomics/hmmer3.git"
    build_and_install(TMP / "hmmer3", str(PKG), "h3result/", url)

    ffibuilder = FFI()

    ffibuilder.cdef(open(INTERFACE, "r").read())
    ffibuilder.set_source(
        "h3result.cffi",
        """
        #include "h3r_result.h"
        """,
        language="c",
        libraries=["h3result", "lio", "lite_pack"],
        library_dirs=[str(LIB)],
        include_dirs=[str(INC)],
    )
    ffibuilder.compile(verbose=True)
