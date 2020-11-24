
---
title: Mimic the dealer
...

# Mimic the dealer

> Difficulty: 08/100

This example implements a “mimic-the-dealer strategy,” i.e. hits if the hand totals less than seventeen and stands on eighteen or more. The player stands on hard seventeen but hits on soft seventeen. 

This time, the configuration file `blackjack.conf` is used. If a file with this name exists in the directory where `blackjack` is executed, it is read and parsed. The options should be fairly self descriptive. See the [configuration file] section of the manual for a detailed explanation of the variables and values that can be entered. In particular, we ask to play one hundred thousand hands at a six-deck game where the dealer hits soft seventeens. If the random seed is set to a fixed value so each execution will lead to the very same sequence of cards.

Now, there are two options that tell Libre Blackjack how the player is going to talk to the backend: `player2dealer` and `dealer2player`. The first one sets the communication mechanism from the player to the dealer (by default is `blackjack`’s standard input), and the second one sets the mechanism from the dealer to the player (by default `blackjack`’s standard output). In this case, the configuration file reads:

```ini
hands = 1e5
decks = 6
hit_soft_17 = 1
# uncomment to obtain the same cards each time
# rng_seed = 1  

player2dealer = fifo mimic_p2d
dealer2player = fifo mimic_d2p
buffered_fifo = 1
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
rm -f mimic_d2p mimic_p2d
mkfifo mimic_d2p mimic_p2d
blackjack &
gawk -f mimic-the-dealer.awk < mimic_d2p > mimic_p2d
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
  count = $2
  # mimic the dealer: hit until 17 (hit soft 17)
  if (abs(count) < 17 || count == -17) {   # soft hands are negative
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
rules:
  decks:                  6
  hands:                  100000
  hit_soft_17:            1
  double_after_split:     1
  blackjack_pays:         1.5
  rng_seed:               -1448949563
  number_of_burnt_cards:  0
  no_negative_bankroll:   0
  max_bet:                0
  penetration:            0.75
  penetration_sigma:      0.05
cpu:
  user:             0.631905
  system:           1.19576
  wall:             2.15273
  second_per_hand:  2.2e-05
  hands_per_second: 4.6e+04
player: 
  wins:               0.41044
  pushes:             0.09695
  losses:             0.49261
  dealer_blackjacks:  0.04658
  player_blackjacks:  0.04663
  dealer_busts:       0.18984
  player_busts:       0.27268
  doubled_hands:      0
  doubled_wins:       0
  insured_hands:      0
  insured_wins:       0
  number_of_hands:    100000
  number_of_shuffles: 2326
  total_money_waged:  100000
  worst_bankroll:     -5996.5
  final_bankroll:     -5994.5
  return:             -0.059945
  variance:            0.955017
  deviation:           0.97725
  error:               0.00309034
  result:             "(-6.0 ± 0.6) %"
...

```

> **Exercise:** modify the player and the configuration file so both the dealer and the player may stand on soft seventeen. Analyze the four combinations (player h17 - dealer h17, player h17 - dealer s17, player s17 - dealer h17, player s17 - dealer s17)



-------
:::{.text-center}
[Previous](../05-no-bust) | [Index](../) | [Next](../20-basic-strategy)
:::

