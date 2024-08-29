#!/bin/sh
set -e

pipx run h3daemon start data/ross.5.hmm --port 51371
pipx run h3daemon isready data/ross.5.hmm --wait
