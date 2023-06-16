#!/bin/bash
set -e

PREFIX=$PWD/hmmer/data
test -d "$PREFIX" || mkdir "$PREFIX"

TMP=$PWD/.build_hmmer
mkdir "$TMP"

(
    git clone https://github.com/horta/hmmer.git "$TMP/hmmer"
    cd "$TMP/hmmer"
    git reset --hard 0a6d8bef0634f002fe649cff674e7a5f104af43e
)

(
    git clone https://github.com/horta/easel.git "$TMP/hmmer/easel"
    cd "$TMP/hmmer/easel"
    git reset --hard fc4a44acc0773125bb16dc13cf529adc99d6ddd6
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
