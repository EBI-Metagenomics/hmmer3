#!/bin/sh

test -f data/ross.5.hmm.pid && pipx run h3daemon stop data/ross.5.hmm
rm -f data/ross.5.hmm.*
