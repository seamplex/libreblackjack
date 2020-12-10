
---
title: Internal player
...

# Internal player

> Difficulty: 00/100

If `blackjack` is called with the `-i` option, it uses an _internal_ player to play against itself. By default it plays basic strategy, although it can read a text file with the strategy. Run 

```terminal
blackjack -i
```

and you will get the following report with the results of playing one million hands with basic strategy.

```yaml
---
result: "(-0.7 ± 0.3) %"
mean: -0.006799
error: 0.00348707
hands: 1e+06
bankroll: -6799
bustsPlayer: 0.139358
bustsDealer: 0.239722
wins: 0.448034
pushes: 0.085913
losses: 0.495532
...

```

-------
:::{.text-center}
[Previous](../) | [Index](../) | [Next](../02-always-stand)
:::
