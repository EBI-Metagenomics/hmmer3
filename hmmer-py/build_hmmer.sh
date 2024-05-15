#!/bin/bash
set -e

PREFIX=$PWD/hmmer/data
test -d "$PREFIX" || mkdir "$PREFIX"

TMP=$PWD/.build_hmmer
mkdir "$TMP"

(
    git clone https://github.com/horta/hmmer.git "$TMP/hmmer"
    cd "$TMP/hmmer"
    git reset --hard 1277e6d4aa760b2c6f38f7e9b88f46e1530ead44
)

(
    git clone https://github.com/EddyRivasLab/easel.git "$TMP/hmmer/easel"
    cd "$TMP/hmmer/easel"
    git reset --hard 07ca83ba9ef0414dba9ce0a9331d465b5eb58f2b
)

(cd "$TMP/hmmer" && autoconf && ./configure && make)

declare -a binaries=(
    alimask
    hmmalign
    hmmbuild
    hmmc2
    hmmconvert
    hmmemit
    hmmerfm-exactmatch
    hmmfetch
    hmmlogo
    hmmpgmd
    hmmpgmd_shard
    hmmpress
    hmmscan
    hmmsearch
    hmmsim
    hmmstat
    jackhmmer
    makehmmerdb
    nhmmer
    nhmmscan
    phmmer
)

test -d "$PREFIX/bin" || mkdir "$PREFIX/bin"
for x in "${binaries[@]}"; do
    cp "$TMP/hmmer/src/$x" "$PREFIX/bin/"
done

rm -rf "$TMP"
