This test is a mimic of an actual single-deck play I tried.
The deck is in `cards.txt` and the play is in `play.txt`.
The results are

```
+1 +2 +2 -1 +1 0 +1 +1.5 -1 -1
```

The mean, variance and standard deviation can be computed with FeenoX:

```
VECTOR r[10] DATA +1 +2 +2 -1 +1 0 +1 +1.5 -1 -1
PRINTF "%.4f %.4f %.4f" vecmean(r) vecvariance(r) vecsd(r)
```

This gives

```
0.5500 1.4694 1.2122
```

Running `blackjack`, we get

```
$ blackjack < play.txt 
play? 20 4
play? 10 3
play? 11 8
play? 5 10
play? 7 10
play? 17 10
play? 9 10
play? 19 10
play? 17 8
play? 17 9
play? 8 -11
play? 12 -11
play? 17 -11
play? 15 5
bye
$ yq . report.yaml 
{
  "result": "(+55 ± 115)",
  "rules": "enhc h17 das doa nrsa",
  "mean": 0.55,
  "error": 1.15,
  "hands": 10,
  "bankroll": 5.5,
  "busts_player": 0,
  "busts_dealer": 0.2,
  "wins": 0.6,
  "pushes": 0.1,
  "losses": 0.3,
  "total_money_waged": 12,
  "blackjacks_player": 0.1,
  "blackjacks_dealer": 0,
  "variance": 1.46944,
  "deviation": 1.21221
}
```
