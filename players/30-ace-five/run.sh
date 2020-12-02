# if test ! -e fifo; then
#  mkfifo fifo
# fi
# blackjack --decks=4 --player=stdio --verbose=true -n2e4 < fifo | ./ace-five.py > fifo

if test ! -e A; then
 mkfifo A
 mkfifo B
fi
python3 ace-five.py < A > B &
blackjack --decks=4 --player=stdio --verbose=true -n1e6 > A < B 
