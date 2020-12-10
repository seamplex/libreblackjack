
---
title: Always stand
...

# Always stand

> Difficulty: 02/100

To play Blackjack as an “always-stander” run the following command:

```terminal
yes stand | blackjack -n1e5 --flat_bet=1 --no_insurance=true > /dev/null
```

The UNIX command `yes stand` writes the string “stand” repeteadly to the standard output, which is piped to the executable `blackjack` (assumed to be installed system-wide). The arguments tell Libre Blackjack to play one hundred thousand hands (`-n1e5`) using a flat bet and without asking for insurance if the dealer shows an ace (`no_insurance`). As there is no `blackjack.conf` file, the rules are---as expected---the default ones (see the documentation for details).

This example is only one-way (i.e. the player ignores what the dealer says) so it is better to redirect the standard output to `/dev/null` to save execution time. The results are written as a YAML-formatted data to `stderr` by default once the hands are over, so they will show up in the terminal nevertheless. This format is human-friendly (far more than JSON) so it can be easily parsed, but it also allows complex objects to be represented (arrays, lists, etc.).


```yaml
---
result: "(-15.8 ± 0.9) %"
mean: -0.157675
error: 0.00940803
hands: 100000
bankroll: -15767.5
bustsPlayer: 0
bustsDealer: 0.27344
wins: 0.38585
pushes: 0.04807
losses: 0.56608
...

```

> **Exercise:** verify that the analytical probability of getting a natural playing with a single deck (for both the dealer and the player) is 32/663 = 0.04826546...

-------
:::{.text-center}
[Previous](../00-internal) | [Index](../) | [Next](../05-no-bust)
:::
