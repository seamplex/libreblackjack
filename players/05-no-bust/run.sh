rm -f fifo; mkfifo fifo
blackjack -n1e5 < fifo | ./no-bust.pl > fifo
