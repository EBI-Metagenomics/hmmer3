#!/bin/sh
set -e

pipx run h3daemon stop "$1"
