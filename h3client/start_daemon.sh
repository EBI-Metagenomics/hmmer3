#!/bin/sh

command -v h3daemon >/dev/null || pipx install h3daemon

h3daemon start data/ross.5.hmm --port 51371 >/dev/null
h3daemon ready data/ross.5.hmm --wait       >/dev/null
