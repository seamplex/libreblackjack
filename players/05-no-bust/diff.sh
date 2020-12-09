blackjack -n1e5 --rng_seed=1 --report_file_path=perl.yml  < fifo | ./no-bust.pl  > fifo
blackjack -n1e5 --rng_seed=1 --report_file_path=shell.yml < fifo | ./no-bust.awk > fifo
diff perl.yml shell.yml 
