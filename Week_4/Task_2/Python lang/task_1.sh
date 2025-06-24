#!/bin/bash

python3 -m venv venv || exit 1
source venv/bin/activate
pip3 install --quiet -r requirements.txt || exit 1

SCRIPT_PATH="$(pwd)/Task_1.py"
ZSHRC=~/.zshrc
ENTRY="source \"$(pwd)/venv/bin/activate\" && python3 \"$SCRIPT_PATH\" &"

grep -Fxq "$ENTRY" "$ZSHRC" || echo "$ENTRY" >> "$ZSHRC"
