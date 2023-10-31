import os
import re
import shutil
import sys
import sysconfig
import tarfile
import urllib.request
from dataclasses import dataclass
from pathlib import Path
from subprocess import check_call

RPATH = "$ORIGIN" if sys.platform.startswith("linux") else "@loader_path"

PWD = Path(os.path.dirname(os.path.abspath(__file__)))
TMP = PWD / ".build_ext"
PKG = PWD / "h3result"
INTERFACE = PKG / "interface.h"

BIN = Path(PKG) / "bin"
LIB = Path(PKG) / "lib"
INCL = Path(PKG) / "include"
EXTRA = f"-Wl,-rpath,{RPATH}/lib"
SHARE = Path(PKG) / "share"

CMAKE_OPTS = [
    "-DCMAKE_BUILD_TYPE=Release",
    "-DBUILD_SHARED_LIBS=ON",
    f"-DCMAKE_INSTALL_RPATH={RPATH}",
    "-DCMAKE_INSTALL_LIBDIR=lib",
]

CPM_OPTS = ["-DCPM_USE_LOCAL_PACKAGES=ON"]


@dataclass
class Ext:
    github_user: str
    github_project: str
    git_tag: str
    root_dir: str
    cmake_opts: list[str]


EXTS = [
    Ext("EBI-Metagenomics", "lip", "v0.5.4", "./", CMAKE_OPTS),
    Ext(
        "EBI-Metagenomics",
        "hmmer3",
        "h3result-v0.2.2",
        "./h3result",
        CMAKE_OPTS + CPM_OPTS,
    ),
]


def rm(folder: Path, pattern: str):
    for filename in folder.glob(pattern):
        filename.unlink()


def resolve_bin(bin: str):
    paths = [sysconfig.get_path("scripts", x) for x in sysconfig.get_scheme_names()]
    paths += ["/usr/local/bin/"]
    for x in paths:
        y = Path(x) / bin
        if y.exists():
            return str(y)
    raise RuntimeError(f"Failed to find {bin}.")


def build_ext(ext: Ext):
    from cmake import CMAKE_BIN_DIR

    url = (
        f"https://github.com/{ext.github_user}/{ext.github_project}"
        f"/archive/refs/tags/{ext.git_tag}.tar.gz"
    )
    tar_filename = f"{ext.github_project}-{ext.git_tag}.tar.gz"

    os.makedirs(TMP, exist_ok=True)
    with open(TMP / tar_filename, "wb") as lf:
        lf.write(urllib.request.urlopen(url).read())

    with tarfile.open(TMP / tar_filename) as tf:
        dir = os.path.commonprefix(tf.getnames())
        tf.extractall(TMP)

    prj_dir = TMP / dir / ext.root_dir
    bld_dir = prj_dir / "build"
    os.makedirs(bld_dir, exist_ok=True)

    cmake = [str(v) for v in Path(CMAKE_BIN_DIR).glob("cmake*")][0]
    check_call([cmake, "-S", str(prj_dir), "-B", str(bld_dir)] + ext.cmake_opts)
    n = os.cpu_count()
    check_call([cmake, "--build", str(bld_dir), "-j", str(n), "--config", "Release"])

    check_call([cmake, "--install", str(bld_dir), "--prefix", str(PKG)])


if __name__ == "__main__":
    from cffi import FFI

    ffibuilder = FFI()

    rm(PKG, "cffi.*")
    rm(PKG / "lib", "**/lib*")
    shutil.rmtree(TMP, ignore_errors=True)

    if not os.environ.get("H3RESULT_DEVELOP", False):
        for ext in EXTS:
            build_ext(ext)

    libs = os.environ.get("H3RESULT_LIBRARY_PATH", "").split(";")
    incls = os.environ.get("H3RESULT_INCLUDE_PATH", "").split(";")

    libs = [x for x in libs if len(x) > 0]
    incls = [x for x in incls if len(x) > 0]

    ffibuilder.cdef(open(INTERFACE, "r").read())
    ffibuilder.set_source(
        "h3result.cffi",
        """
        #include "h3result/h3result.h"
        """,
        language="c",
        libraries=["h3result"],
        library_dirs=libs + [str(LIB)],
        include_dirs=incls + [str(INCL)],
        extra_link_args=[str(EXTRA)],
    )
    ffibuilder.compile(verbose=True)

    shutil.rmtree(BIN, ignore_errors=True)
    shutil.rmtree(INCL, ignore_errors=True)
    shutil.rmtree(SHARE, ignore_errors=True)
    shutil.rmtree(LIB / "cmake", ignore_errors=True)

    if not os.environ.get("H3RESULT_DEVELOP", False):
        if sys.platform == "linux":
            patch = [resolve_bin("patchelf"), "--set-rpath", "$ORIGIN"]
            for lib in LIB.glob("*.so*"):
                check_call(patch + [str(lib)])

        find = ["/usr/bin/find", str(LIB), "-type", "l"]
        exec0 = ["-exec", "/bin/cp", "{}", "{}.tmp", ";"]
        exec1 = ["-exec", "/bin/mv", "{}.tmp", "{}", ";"]
        check_call(find + exec0 + exec1)

        for x in list(LIB.iterdir()):
            linux_pattern = r"lib[^.]*\.so\.[0-9]+"
            macos_pattern = r"lib[^.]*\.[0-9]+\.dylib"
            pattern = r"^(" + linux_pattern + r"|" + macos_pattern + r")$"
            if not re.match(pattern, x.name):
                x.unlink()
