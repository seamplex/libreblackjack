rm -f d2p p2d; mkfifo d2p p2d
./mimic-the-dealer.awk < d2p > p2d &
blackjack > d2p < p2d 
