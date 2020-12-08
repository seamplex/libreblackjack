---
title: Example players for LibreBlackjack
...

# Example players for LibreBlackjack

The subdirectory `players` contains some automatic players that play against LibreBlackjack. These players are coded in different languages and communicate with LibreBlackjack in a variety of ways in order to illustrate the design basis:

 * [`00-internal`](00-internal) uses the internal player (coded in C++) that defaults to playing one million hands of basic strategy
 * [`02-always-stand`](02-always-stand), using the UNIX tool `yes` this player always says “stand” into the standard output (which is piped to libreblackjack’s standard input) no matter what the cards are
 * [`05-no-bust`](05-no-bust) is a PERL-based player does not bust (i.e. hits if the hard total is less than twelve) that receives the cards through the standard input but draws or stands using a FIFO to talk back to the dealer. There are also implementation in AWK (similar speed) and in a pure Bash script (10x slower)
 * [`08-mimic-the-dealer`](08-mimic-the-dealer) does tha same the dealer do (hits soft seventeens). It is implemented in AWK using two FIFOs.
 * [`20-basic-strategy`](20-basic-strategy) derives the basic strategy from scratch in less than one minute by calling the internal player successively with different strategy files from a shell script.

