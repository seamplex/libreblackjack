define(case_title, No-bust strategy)
---
title: case_title
...

# case_title

> Difficulty: case_difficulty/100

This directory shows how to play a “no-bust” strategy, i.e. not hitting any hand higher or equal to hard twelve with Libre Blackjack. The communication between the player and the back end is through standard input and output. The player reads from its standard input Libre Blackjack's commands and writes to its standard output the playing commands. In order to do this a FIFO (a.k.a. named pipe) is needed. So first, we create it (if it is not already created):

```terminal
mkfifo fifo
```

Then we execute `blackjack`, piping its output to the player (say `no-bust.pl`) and reading the standard input from `fifo`, whilst at the same time we redirect the player's standard output to `fifo`:

```terminal
include(run.sh)dnl
```

As this time the player is coded in an interpreted langauge, it is far smarter than the previous `yes`-based player. So the player can handle bets and insurances, and there is not need to pass the options `--flat_bet` nor `--no_insurance` (though they can be passed anyway). Let us take a look at the Perl implementation:

```perl
include(no-bust.pl)dnl
```

The very same player may be implemented as a shell script:

```bash
include(no-bust.sh)dnl
```

To check these two players give the same results, make them play against Libre Blackjack with the same seed (say one) and send the YAML report to two different files:

```terminal
include(diff.sh)
esyscmd(diff perl.yml shell.yml)dnl
```

As expected, the reports are the same. They just differ in the speed because the shell script is orders of magnitude slower than its Perl-based counterpart. 

> **Exercise:** modify the players so they always insure aces and see if it improves or degrades the result.


case_nav
