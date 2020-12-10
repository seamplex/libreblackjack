blackjack -n1e4 --rng_seed=1 --report_file_path=perl.yml  < fifo | ./no-bust.pl  > fifo
blackjack -n1e4 --rng_seed=1 --report_file_path=awk.yml   < fifo | ./no-bust.awk > fifo
blackjack -n1e4 --rng_seed=1 --report_file_path=shell.yml < fifo | ./no-bust.sh  > fifo
md5sum *.yml
