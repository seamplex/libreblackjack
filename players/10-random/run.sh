#!/bin/sh

# prepare the FIFO
rm -f f && mkfifo f;

# run the dealer & player
cat f | blackjack --verbose=true -n${n} | python3 player_random.py > f
