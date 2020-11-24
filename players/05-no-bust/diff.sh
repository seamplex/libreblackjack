../../blackjack -n100000 --rng_seed=1 --yaml_report=perl.yml  < fifo | ./no-bust.pl  > fifo
../../blackjack -n100000 --rng_seed=1 --yaml_report=shell.yml < fifo | ./no-bust.awk > fifo
# diff perl.yml shell.yml 
