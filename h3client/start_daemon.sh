#!/bin/sh
set -e

pipx run h3daemon start "$1" --port 51371
pipx run h3daemon isready "$1" --wait
