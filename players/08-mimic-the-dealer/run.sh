rm -f d2p p2d; mkfifo d2p p2d
gawk -f mimic-the-dealer.awk < d2p > p2d &
blackjack -n1e5 > d2p < p2d 
