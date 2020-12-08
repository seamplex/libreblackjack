---
title: Libre Blackjack, a free blackjack back end
lang: en-US
...


# Libre Blackjack

> A [free](https://www.gnu.org/philosophy/free-sw.html) [Blackjack](https://en.wikipedia.org/wiki/Blackjack) back end inspired by [GNU Chess](https://www.gnu.org/software/chess/).

[Libre Blackjack](https://www.seamplex.com/blackjack) is a blackjack engine that emulates a dealer, deals (digital) cards and understands plain-text commands such as `hit` or `stand`. The basic idea is that one or more players can talk to Libre Blackjack either in an interactive or in an automated way through
 
 * the standard input and/or output (optionally using named pipes or TCP (web)sockets with `netcat` or `gwsocket`), or
 * C++ methods (optionally loaded at runtime from shared objects TBD).
  
These players can be actual human players playing in real-time through a front end (a GUI application, a web-based interface, a mobile app, etc.) or robots that implement a certain betting and playing strategy playing (i.e. card counting) as fast as possible to study and analyze game statistics. There is an internal player that reads the strategy from a text file and plays accordingly. It can also be used to play interactive [ASCII blackjack](#play):

```{=html}
<asciinema-player src="doc/interactive.cast" cols="89" rows="28" preload="true" poster="npt:0:20"></asciinema-player>
```



## Why

:::{.alert .alert-light}
> “I am often surprised that when people drive down two-lane roads, they will trust complete strangers in the oncoming lane not to swerve into their lane causing a head-on collision; but they will not trust mathematicians to create the correct strategy for Blackjack.”
>
> [Norman Wattenberger, Modern Blackjack, 2009]{.blockquote-footer}
:::

With Libre Blackjack you do not have to trust other people anymore. You have a free blackjack engine which you can

 0. run as you wish, to see the results of billions of blackjack hands,
 1. study to see how it works and change it if you do not like it,
 2. share it with your friends and colleagues, and
 3. distribute copies of your modified versions.

If you do not know how to program, you have the _freedom_ to hire a programmer to do it for you. That is why [Libre Blackjack](https://www.seamplex.com/blackjack) is [free software](https://www.gnu.org/philosophy/free-sw.html).


## How 

Once you trust the blackjack engine is fair, you can model and simulate any blackjack situation you want, playing millions of times a certain hand (say a sixteen against a ten) in different ways (say hitting or standing) to obtain you own conclusions. You can even build  the [basic strategy charts](https://wizardofodds.com/games/blackjack/strategy/4-decks/) from scratch to convince yourself there is no [flaw](https://wizardofodds.com/ask-the-wizard/blackjack/).

The main objective is research and optimization of playing and betting strategies depending on

 * particular table rules (number of decks, hit on soft 17, double after split, etc.), 
 * card counting strategies 
 * risk of ruin
 * removal of cards
 * arranged shoes
 
These automatic players can range from simple no-bust or mimic-the-dealer hitters or standers, up to neural-networks trained players taking into account every card being dealt passing through basic strategy modified by traditional card counting mechanisms.

# Quick start


```
sudo apt-get install git autoconf make gcc
git clone https://github.com/seamplex/libreblackjack.git
cd libreblackjack
./autogen.sh
./configure
make
sudo make install
```

If you wan tab completion of commands and browseable history, make sure you also have [GNU Readline](http://tiswww.case.edu/php/chet/readline/rltop.html) available at compilation time.


## Test suite

Run as test suite to check the code work as expected.

```
$ make check
```

The subdirectory `players` contains some automatic players that play against Libre Blackjack. These players are coded in different languages and communicate with Libre Blackjack in a variety of ways in order to illustrate the design basis:

 * [00-internal](players/00-internal) uses the internal player that defaults to playing one million hands of basic strategy
 * [02-always-stand](players/02-always-stand), using the UNIX tool `yes` this player always says “stand” into the standard output (which is piped to `blackjack`’s standard input) no matter what the cards are
 * [05-no-bust](players/05-no-bust) is a PERL-based player does not bust (i.e. hits if the hard total is less than twelve) that receives the cards through the standard input but draws or stands using a FIFO to talk back to the dealer
 * [08-mimic-the-dealer](players/08-mimic-the-dealer) follows what the dealer does (i.e. hits soft seventeens). It is implemented in AWK using two FIFOs.
 * [20-basic-strategy](players/20-basic-strategy) derives the basic strategy from scratch in less than one minute by creating all the possible combination of hitting/standing/doubling/pairing strategies in an ASCII text file that the internal player can read and use.


# Play

Run Libre Blackjack with no arguments to play Blackjack interactively in ASCII mode.

```{=html}
<asciinema-player src="doc/libreblackjack2.cast" cols="89" rows="28" preload="true" poster="npt:0:20"></asciinema-player>
```

Edit the file [`blackjack.conf`](https://github.com/seamplex/libreblackjack/blob/master/blackjack.conf) in the same directory where the executable is run to set up rules, arranged shoes and other options. Type `help` at the prompt to get it.

## Automatic playing

The differential value of Libre Blackjack is that players can be programmed to play employing different strategies, card-counting techniques or even state-of-the-art AI algorithms.

See the directory [`players`](players) for examples of how to write players in

 * [a pure UNIX way using the `yes` tool](players/02-always-stand)
 * [Perl](players/05-no-bust)
 * [Awk](players/08-mimic-the-dealer)
 * [Bash](players/20-basic-strategy)
 * C
 * Python
 
## TCP Sockets

To play through a TCP socket, call `blackjack` from `netcat`. On one host, do

```
nc -l -p1234 -e blackjack
```

On the other one, connect to the first host on port 1234:

```
nc host 1234
```

# Licensing

Libre Blackjack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

# Further information

Home page: <https://www.seamplex.com/blackjack>  
Repository: <https://github.com/seamplex/blackjack>  
Mailing list and bug reports: <wasora@seamplex.com>  (you need to subscribe first at <wasora+subscribe@seamplex.com>)  
Follow us: [Twitter](https://twitter.com/seamplex/) [YouTube](https://www.youtube.com/channel/UCC6SzVLxO8h6j5rLlfCQPhA) [LinkedIn](https://www.linkedin.com/company/seamplex/) [GitHub](https://github.com/seamplex/)

----------------------------------------------------

Libre Blackjack is copyright (C) 2016,2020 Jeremy Theler  
Libre Blackjack is licensed under [GNU GPL version 3](http://www.gnu.org/copyleft/gpl.html) or (at your option) any later version.  
Libre Blackjack is free software: you are free to change and redistribute it.  
There is NO WARRANTY, to the extent permitted by law.  
See the file `COPYING` for copying conditions.  
