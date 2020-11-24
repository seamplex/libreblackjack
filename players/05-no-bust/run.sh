if test ! -e fifo; then
 mkfifo fifo
fi
blackjack -n1e5 < fifo | ./no-bust.pl > fifo
