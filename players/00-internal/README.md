# Internal player

If `blackjack` is called with the `-i` option, it uses an _internal_ player to play against itself.
By default it plays basic strategy, although it can read a text file with the strategy. Run 

```terminal
blackjack -i
```

and you will get the following report with the results of playing one million hands with basic strategy within a couple of seconds

```yaml
result: "(-0.4 ± 0.3)"
rules: "ahc h17 das doa 3rsp 0decks"
mean: -0.0043645
error: 0.00348685
hands: 1e+06
bankroll: -4364.5
busts_player: 0.138314
busts_dealer: 0.240593
wins: 0.449235
pushes: 0.085877
losses: 0.494409
total_money_waged: 1.13739e+06
blackjacks_player: 0.047296
blackjacks_dealer: 0.047421
variance: 1.3509
deviation: 1.16228
```

If you want to change the strategy the player plays, then prepare a text file that looks like this

```
#    2  3  4  5  6  7  8  9  T  A
h20  s  s  s  s  s  s  s  s  s  s  
h19  s  s  s  s  s  s  s  s  s  s  
h18  s  s  s  s  s  s  s  s  s  s  
h17  s  s  s  s  s  s  s  s  s  s  
h16  s  s  s  s  s  h  h  h  h  h  
h15  s  s  s  s  s  h  h  h  h  h  
h14  s  s  s  s  s  h  h  h  h  h  
h13  s  s  s  s  s  h  h  h  h  h  
h12  h  h  s  s  s  h  h  h  h  h  
h11  d  d  d  d  d  d  d  d  h  h  
h10  d  d  d  d  d  d  d  d  h  h  
h9   h  d  d  d  d  h  h  h  h  h  
h8   h  h  h  h  h  h  h  h  h  h  
h7   h  h  h  h  h  h  h  h  h  h  
h6   h  h  h  h  h  h  h  h  h  h  
h5   h  h  h  h  h  h  h  h  h  h  
h4   h  h  h  h  h  h  h  h  h  h  
#    2  3  4  5  6  7  8  9  T  A
s20  s  s  s  s  s  s  s  s  s  s  
s19  s  s  s  s  s  s  s  s  s  s  
s18  s  d  d  d  d  s  s  h  h  h  
s17  h  d  d  d  d  h  h  h  h  h  
s16  h  h  d  d  d  h  h  h  h  h  
s15  h  h  d  d  d  h  h  h  h  h  
s14  h  h  h  d  d  h  h  h  h  h  
s13  h  h  h  h  d  h  h  h  h  h  
s12  h  h  h  h  d  h  h  h  h  h  
#    2  3  4  5  6  7  8  9  T  A
pA   y  y  y  y  y  y  y  y  y  n  
pT   n  n  n  n  n  n  n  n  n  n  
p9   y  y  y  y  y  n  y  y  n  n  
p8   n  n  n  n  n  n  n  n  n  n  
p7   y  y  y  y  y  y  n  n  n  n  
p6   y  y  y  y  y  n  n  n  n  n  
p5   n  n  n  n  n  n  n  n  n  n  
p4   n  n  n  y  y  n  n  n  n  n  
p3   y  y  y  y  y  y  n  n  n  n  
p2   y  y  y  y  y  y  n  n  n  n  
```

save it as `strategy.txt` (or pick a more informative name) and run it like

```terminal
blackjack -i --strategy=strategy.txt
```

to see how dumb it is to never split:


```txt
#    2  3  4  5  6  7  8  9  T  A
h20  s  s  s  s  s  s  s  s  s  s  
h19  s  s  s  s  s  s  s  s  s  s  
h18  s  s  s  s  s  s  s  s  s  s  
h17  s  s  s  s  s  s  s  s  s  s  
h16  s  s  s  s  s  h  h  h  h  h  
h15  s  s  s  s  s  h  h  h  h  h  
h14  s  s  s  s  s  h  h  h  h  h  
h13  s  s  s  s  s  h  h  h  h  h  
h12  h  h  s  s  s  h  h  h  h  h  
h11  d  d  d  d  d  d  d  d  h  h  
h10  d  d  d  d  d  d  d  d  h  h  
h9   h  d  d  d  d  h  h  h  h  h  
h8   h  h  h  h  h  h  h  h  h  h  
h7   h  h  h  h  h  h  h  h  h  h  
h6   h  h  h  h  h  h  h  h  h  h  
h5   h  h  h  h  h  h  h  h  h  h  
h4   h  h  h  h  h  h  h  h  h  h  
#    2  3  4  5  6  7  8  9  T  A
s20  s  s  s  s  s  s  s  s  s  s  
s19  s  s  s  s  s  s  s  s  s  s  
s18  s  d  d  d  d  s  s  h  h  h  
s17  h  d  d  d  d  h  h  h  h  h  
s16  h  h  d  d  d  h  h  h  h  h  
s15  h  h  d  d  d  h  h  h  h  h  
s14  h  h  h  d  d  h  h  h  h  h  
s13  h  h  h  h  d  h  h  h  h  h  
s12  h  h  h  h  d  h  h  h  h  h  
#    2  3  4  5  6  7  8  9  T  A
pA   n  n  n  n  n  n  n  n  n  n  
pT   n  n  n  n  n  n  n  n  n  n  
p9   n  n  n  n  n  n  n  n  n  n  
p8   n  n  n  n  n  n  n  n  n  n  
p7   n  n  n  n  n  n  n  n  n  n  
p6   n  n  n  n  n  n  n  n  n  n  
p5   n  n  n  n  n  n  n  n  n  n  
p4   n  n  n  n  n  n  n  n  n  n  
p3   n  n  n  n  n  n  n  n  n  n  
p2   n  n  n  n  n  n  n  n  n  n  
```

i.e., roughly you give up 1% of your expected return:

```yaml
result: "(-1.4 ± 0.3)"
rules: "ahc h17 das doa 3rsp 0decks"
mean: -0.0135275
error: 0.00328319
hands: 1e+06
bankroll: -13527.5
busts_player: 0.16438
busts_dealer: 0.232731
wins: 0.431502
pushes: 0.086498
losses: 0.482
total_money_waged: 1.08152e+06
blackjacks_player: 0.047659
blackjacks_dealer: 0.047255
variance: 1.1977
deviation: 1.0944
```

