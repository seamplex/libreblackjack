
---
title: Internal player
...

# Internal player

> Difficulty: 00/100

If `blackjack` is called with the `-i` option, it uses an _internal_ player to play against itself. By default it plays basic strategy. Run 

```terminal
blackjack -i
```

and you will get the following report with the results of playing one million hands with basic strategy.

```yaml
---
rules:
  decks:                  6
  hands:                  1e+06
  hit_soft_17:            1
  double_after_split:     1
  blackjack_pays:         1.5
  rng_seed:               -421186149
  number_of_burnt_cards:  0
  no_negative_bankroll:   0
  max_bet:                0
  penetration:            0.75
  penetration_sigma:      0.05
cpu:
  user:             1.88959
  system:           0.32027
  wall:             2.20904
  second_per_hand:  2.2e-06
  hands_per_second: 4.5e+05
player: 
  wins:               0.445075
  pushes:             0.087039
  losses:             0.492821
  dealer_blackjacks:  0.047582
  player_blackjacks:  0.047375
  dealer_busts:       0.23537
  player_busts:       0.160637
  doubled_hands:      0.107207
  doubled_wins:       0.0613
  insured_hands:      0
  insured_wins:       0
  number_of_hands:    1e+06
  number_of_shuffles: 23215
  total_money_waged:  1.23935e+06
  worst_bankroll:     -7212.5
  final_bankroll:     -7111
  return:             -0.007111
  variance:            1.32892
  deviation:           1.15279
  error:               0.00115279
  result:             "(-0.7 ± 0.2) %"
...

```

-------
:::{.text-center}
[Previous](../) | [Index](../) | [Next](../02-always-stand)
:::
