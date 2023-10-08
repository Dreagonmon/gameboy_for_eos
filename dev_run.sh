#!/bin/bash

readonly PYTHON=".venv/bin/python"
readonly EXPEMU="../EXPEmulator"
readonly SCALE=4
readonly EXP="build/src/gameboy.exp"

rm -r build
cmake -B build
cmake --build build

# install deps
# "$PYTHON" -m pip install -r "$EXPEMU/requirements.txt"

# run
echo ================
echo
echo
"$PYTHON" "$EXPEMU/expemu" -s $SCALE "$EXP"
