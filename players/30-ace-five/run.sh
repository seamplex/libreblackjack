if test ! -e fifo; then
 mkfifo fifo
fi
blackjack --player=stdio --verbose=true -n10000 < fifo | ./ace-five.py > fifo
