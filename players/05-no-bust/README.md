
---
title: No-bust strategy
...

# No-bust strategy

> Difficulty: 05/100

This directory shows how to play a “no-bust” strategy, i.e. not hitting any hand higher or equal to hard twelve with Libre Blackjack. The communication between the player and the back end is through standard input and output. The player reads from its standard input Libre Blackjack's commands and writes to its standard output the playing commands. In order to do this a FIFO (a.k.a. named pipe) is needed. So first, we create it (if it is not already created):

```terminal
mkfifo fifo
```

Then we execute `blackjack`, piping its output to the player (say `no-bust.pl`) and reading the standard input from `fifo`, whilst at the same time we redirect the player's standard output to `fifo`:

```terminal
rm -f fifo; mkfifo fifo
blackjack -n1e5 < fifo | ./no-bust.pl > fifo
```

As this time the player is coded in an interpreted langauge, it is far smarter than the previous `yes`-based player. So the player can handle bets and insurances, and there is not need to pass the options `--flat_bet` nor `--no_insurance` (though they can be passed anyway). Let us take a look at the Perl implementation:

```perl
#!/usr/bin/perl
# this is needed to avoid deadlock with the fifo
STDOUT->autoflush(1);

while ($command ne "bye") {
  # do not play more than a number of commands
  # if the argument -n was not passed to blackjack
  if ($i++ == 1234567) {
    print "quit\n";
  }
  
  # read and process the commands
  chomp($command = <STDIN>);
  
  if ($command eq "bet?") {
    print "1\n";
  } elsif ($command eq "insurance?") {
    print "no\n";
  } elsif ($comm eq "play?") {
    @tokens = split(/ /, $command);
    if ($tokens[1] < 12) {
      print "hit\n";
    } else {
      print "stand\n";
    }
  }
}
```

The very same player may be implemented as a shell script:

```bash
#!/bin/sh

i=0
while read command
do
  i=$((i+1))
  if test ${i} -ge 12345; then
    echo "quit"
  elif test "${command}" = 'bye'; then
    exit
  elif test "${command}" = 'bet?'; then
    echo 1  
  elif test "${command}" = 'insurance?'; then
    echo "no"
  elif test "$(echo ${command} | cut -c-5)" = 'play?'; then
    count=$(echo ${command} | cut -f2 -d" ")
    if test ${count} -lt 12; then
      echo "hit"
    else
      echo "stand"
    fi
  fi
done
```

To check these two players give the same results, make them play against Libre Blackjack with the same seed (say one) and send the YAML report to two different files:

```terminal
blackjack -n1e5 --rng_seed=1 --report_file_path=perl.yml  < fifo | ./no-bust.pl  > fifo
blackjack -n1e5 --rng_seed=1 --report_file_path=shell.yml < fifo | ./no-bust.awk > fifo
diff perl.yml shell.yml 

```

As expected, the reports are the same. They just differ in the speed because the shell script is orders of magnitude slower than its Perl-based counterpart. 

> **Exercise:** modify the players so they always insure aces and see if it improves or degrades the result.


-------
:::{.text-center}
[Previous](../02-always-stand) | [Index](../) | [Next](../08-mimic-the-dealer)
:::
