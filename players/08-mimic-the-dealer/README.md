
---
title: Mimic the dealer
...

# Mimic the dealer

> Difficulty: 08/100

This example implements a “mimic-the-dealer strategy,” i.e. hits if the hand totals less than seventeen and stands on eighteen or more. The player stands on hard seventeen but hits on soft seventeen. 

This time, the configuration file `blackjack.conf` is used. If a file with this name exists in the directory where `blackjack` is executed, it is read and parsed. The options should be fairly self descriptive. See the [configuration file] section of the manual for a detailed explanation of the variables and values that can be entered. In particular, we ask to play one hundred thousand hands at a six-deck game where the dealer hits soft seventeens. If the random seed is set to a fixed value so each execution will lead to the very same sequence of cards.

Now, there are two options that tell Libre Blackjack how the player is going to talk to the backend: `player2dealer` and `dealer2player`. The first one sets the communication mechanism from the player to the dealer (by default is `blackjack`’s standard input), and the second one sets the mechanism from the dealer to the player (by default `blackjack`’s standard output). In this case, the configuration file reads:

```ini
h17 = true
```

This means that two FIFOs (a.k.a. named pipes) are to be used for communication, `player2dealer` from the player to the dealer and `dealer2player` for the dealer to the player. If these FIFOs do not exist, they are created by `blackjack` upon execution. 

The player this time is implemented as an awk script, whose input should be read from `dealer2player` and whose output should be written to `player2dealer`. To run the game, execute `blackjack` in one terminal making sure the current directory is where the `blackjack.conf` file exists. It should print a message telling that it is waiting for someone to be at the other side of the named pipes:

```terminal
$ blackjack
[...]
waiting for dealer2player buffered fifo 'dealer2player'...
```

In another terminal run the player

```terminal
$ ./mimic-the-dealer.awk < dealer2player > player2dealer
```

Both dealer and player may be run in the same terminal putting the first one on the background:

```terminal
rm -f d2p p2d; mkfifo d2p p2d
gawk -f mimic-the-dealer.awk < d2p > p2d &
blackjack -n1e5 > d2p < p2d 
```

To understand the decisions taken by the player, we have to remember that when Libre Blackjack receives the command `count` asking for the current player's count, it returns a positive number for hard hands and a negative number for soft hands. The instructions `fflush()` are needed in order to avoid deadlocks on the named pipes:

```awk
#!/usr/bin/gawk -f
function abs(x){return ( x >= 0 ) ? x : -x } 

/bet\?/ {
  print "1";
  fflush();
}

/insurance\?/ {
  print "no";
  fflush();
}

/play\?/ {
  # mimic the dealer: hit until 17 (hit soft 17)
  if (abs($2) < 17 || $2 == -17) {   # soft hands are negative
    print "hit";
  } else {
    print "stand";
  }
  fflush();  
}

/bye/ {
  exit;
}
```

```yaml
---
result: "(-5.7 ± 0.9) %"
mean: -0.05716
error: 0.00926292
hands: 100000
bankroll: -5716
bustsPlayer: 0.27064
bustsDealer: 0.18905
wins: 0.41088
pushes: 0.09888
losses: 0.49024
...

```

> **Exercise:** modify the player and the configuration file so both the dealer and the player may stand on soft seventeen. Analyze the four combinations (player h17 - dealer h17, player h17 - dealer s17, player s17 - dealer h17, player s17 - dealer s17)



-------
:::{.text-center}
[Previous](../05-no-bust) | [Index](../) | [Next](../20-basic-strategy)
:::

