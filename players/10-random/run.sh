#!/bin/sh

# prepare the FIFO
rm -f f && mkfifo f;

# run the dealer & player
cat f | blackjack | python3 player_random.py > f

# test cards
../../utils/analyze.py cards.txt
