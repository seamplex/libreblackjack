
---
title: Mimic the dealer
...

# Mimic the dealer

> Difficulty: 08/100

This example implements a “mimic-the-dealer strategy,” i.e. hits if the hand totals less than seventeen and stands on eighteen or more. The player---as the dealer---stands on hard seventeen but hits on soft seventeen. 

This time, the configuration file `blackjack.conf` is used. If a file with this name exists in the directory where `blackjack` is executed, it is read and parsed. The options should be fairly self descriptive. See the [configuration file] section of the manual for a detailed explanation of the variables and values that can be entered. In particular, we ask to play one hundred thousand hands at a six-deck game where the dealer hits soft seventeens. The random seed is set to a fixed value so each execution will lead to the very same sequence of cards. In this case, the configuration file reads:

```ini
h17 = true
hands = 1e5
rng_seed = 12345
```

The player this time is implemented as an AWK script, whose input should be read from a piped name `d2p` (i.e. dealer to player)  and whose output should be written to `p2`. To run the game, execute `blackjack` in one terminal making sure the current directory is where the `blackjack.conf` file exists. 

```terminal
$ blackjack > d2p < p2d 
[...]
```

In another terminal run the player

```terminal
$ ./mimic-the-dealer.awk < d2p > p2d
```

Both dealer and player may be run in the same terminal putting the first one on the background:

```terminal
rm -f d2p p2d; mkfifo d2p p2d
./mimic-the-dealer.awk < d2p > p2d &
blackjack > d2p < p2d 
```

The report should always be the same because the random number generator seed is fixed:

```yaml
---
result: "(-5.7 ± 0.9) %"
mean: -0.057095
error: 0.0092763
hands: 100000
bankroll: -5709.5
bustsPlayer: 0.27214
bustsDealer: 0.19076
wins: 0.41149
pushes: 0.09727
losses: 0.49124
...

```

> **Exercise:** modify the player and the configuration file so both the dealer and the player may stand on soft seventeen. Analyze the four combinations (player h17 - dealer h17, player h17 - dealer s17, player s17 - dealer h17, player s17 - dealer s17)



-------
:::{.text-center}
[Previous](../05-no-bust) | [Index](../) | [Next](../20-basic-strategy)
:::

