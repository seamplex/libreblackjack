#!/bin/sh

if test -z "$1"; then
 n=1e5
else
 n=$1
fi

if test ! -e A; then
 mkfifo A
 mkfifo B
fi
python3 ace-five.py < A > B &
blackjack --decks=1 --player=stdio --verbose=true -n${1} > A < B 
