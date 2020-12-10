#!/bin/sh
if test -z "$1"; then n=1e5; else n=$1; fi; rm -f f;  mkfifo f;
cat f | blackjack --decks=1 --verbose=true -n${n} | python3 ace-five.py > f
