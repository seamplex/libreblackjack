

## Internal player


If `blackjack` is called with the `-i` option, it uses an _internal_ player to play against itself. By default it plays basic strategy, although it can read a text file with the strategy. Run 

```terminal
blackjack -i
```

and you will get the following report with the results of playing one million hands with basic strategy.

```yaml
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

```





## Always stand


To play Blackjack as an “always-stander” run the following command:

```terminal
yes stand | blackjack -n1e5 --flat_bet=1 --no_insurance=true > /dev/null
```

The UNIX command `yes stand` writes the string “stand” repeteadly to the standard output, which is piped to the executable `blackjack` (assumed to be installed system-wide). The arguments tell Libre Blackjack to play one hundred thousand hands (`-n1e5`) using a flat bet and without asking for insurance if the dealer shows an ace (`no_insurance`). As there is no `blackjack.conf` file, the rules are---as expected---the default ones (see the documentation for details).

This example is only one-way (i.e. the player ignores what the dealer says) so it is better to redirect the standard output to `/dev/null` to save execution time. The results are written as a YAML-formatted data to `stderr` by default once the hands are over, so they will show up in the terminal nevertheless. This format is human-friendly (far more than JSON) so it can be easily parsed, but it also allows complex objects to be represented (arrays, lists, etc.).


```yaml
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

```

> **Exercise:** verify that the analytical probability of getting a natural playing with a single deck (for both the dealer and the player) is 32/663 = 0.04826546...





## No-bust strategy


This example shows how to play a “no-bust” strategy, i.e. not hitting any hand higher or equal to hard twelve with Libre Blackjack. The communication between the player and the back end is through standard input and output. The player reads from its standard input Libre Blackjack's commands and writes to its standard output the playing commands. In order to do this a FIFO (a.k.a. named pipe) is needed. So first, we create it (if it is not already created):

```terminal
mkfifo fifo
```

Then we execute `blackjack`, piping its output to the player (say `no-bust.pl`) and reading the standard input from `fifo`, whilst at the same time we redirect the player's standard output to `fifo`:

```terminal
rm -f fifo; mkfifo fifo
blackjack -n1e5 < fifo | ./no-bust.pl > fifo
```

As this time the player is coded in an interpreted langauge, it is far smarter than the previous `yes`-based player. Since the player can handle bets and insurances, and there is not need to pass the options `--flat_bet` nor `--no_insurance` (though they can be passed anyway). Let us take a look at the Perl implementation:

```perl
##!/usr/bin/perl
## this is needed to avoid deadlock with the fifo
STDOUT->autoflush(1);

while ($command ne "bye") {
  ## do not play more than a number of commands
  ## if the argument -n was not passed to blackjack
  if ($i++ == 1234567) {
    print "quit\n";
  }
  
  ## read and process the commands
  chomp($command = <STDIN>);
  
  if ($command eq "bet?") {
    print "1\n";
  } elsif ($command eq "insurance?") {
    print "no\n";
  } elsif ($comm eq "play?") {
    @tokens = split(/ /, $command);
    if ($tokens[1] < 12) {
      print "hit\n";
    } else {
      print "stand\n";
    }
  }
}
```

The very same player may be implemented in AWK:

```bash
##!/usr/bin/gawk -f
## mawk does not work, it hangs due to the one-sided FIFO
{
  if (n++ > 1234567) {
    print "quit";
  }
}
/bet\?/ {
  print "1"
  fflush()
}
/insurance\?/ {
  print "no"
  fflush()
}
/play\?/ {
  if ($2 < 12) {
    print "hit";
  } else {
    print "stand";
  }
  fflush()
}
/bye/ {
  exit;
}
```

And even as a shell script:

```bash
##!/bin/sh

i=0
while read command
do
  i=$((i+1))
  if test ${i} -ge 1234567; then
    echo "quit"
  elif test "${command}" = 'bye'; then
    exit
  elif test "${command}" = 'bet?'; then
    echo 1  
  elif test "${command}" = 'insurance?'; then
    echo "no"
  elif test "$(echo ${command} | cut -c-5)" = 'play?'; then
    count=$(echo ${command} | cut -f2 -d" ")
    if test ${count} -lt 12; then
      echo "hit"
    else
      echo "stand"
    fi
  fi
done
```

To check these three players give the same results, make them play against Libre Blackjack with the same random seed (say one) and send the YAML report to three different files:

```terminal
blackjack -n1e4 --rng_seed=1 --report_file_path=perl.yml  < fifo | ./no-bust.pl  > fifo
blackjack -n1e4 --rng_seed=1 --report_file_path=awk.yml   < fifo | ./no-bust.awk > fifo
blackjack -n1e4 --rng_seed=1 --report_file_path=shell.yml < fifo | ./no-bust.sh  > fifo
md5sum *.yml
7b0d1e16347288df7815e7b2cc55d930  awk.yml
7b0d1e16347288df7815e7b2cc55d930  perl.yml
7b0d1e16347288df7815e7b2cc55d930  shell.yml
```

> **Exercise:** modify the players so they always insure aces and see if it improves or degrades the result.






## Mimic the dealer


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

```

> **Exercise:** modify the player and the configuration file so both the dealer and the player may stand on soft seventeen. Analyze the four combinations (player h17 - dealer h17, player h17 - dealer s17, player s17 - dealer h17, player s17 - dealer s17)








## Derivation of the basic strategy


### Quick run

Execute the `run.sh` script. It should take a few minutes:

```terminal
$ ./run.sh
h20-2 (10 10)   8.0e+04 +63.23  (1.1)   -171.17 (1.1)   -85.32  (0.5)   stand
h20-3 (10 10)   8.0e+04 +64.54  (1.1)   -171.50 (1.1)   -85.50  (0.5)   stand
h20-4 (10 10)   8.0e+04 +65.55  (1.1)   -170.33 (1.1)   -85.50  (0.5)   stand
h20-5 (10 10)   8.0e+04 +66.65  (1.1)   -171.25 (1.1)   -85.51  (0.5)   stand
h20-6 (10 10)   8.0e+04 +67.80  (1.1)   -171.07 (1.1)   -85.59  (0.5)   stand
h20-7 (10 10)   8.0e+04 +77.44  (1.1)   -170.53 (1.1)   -85.44  (0.5)   stand
h20-8 (10 10)   8.0e+04 +79.11  (1.1)   -170.08 (1.1)   -85.02  (0.6)   stand
h20-9 (10 10)   8.0e+04 +75.77  (1.1)   -170.31 (1.1)   -84.87  (0.6)   stand
p2-6            8e+04   +24.78  (2.9)   +3.07   (1.0)   yes
p2-7            8e+04   +1.48   (2.0)   -8.90   (1.0)   yes
p2-8            8e+04   -17.57  (2.0)   -16.33  (1.0)   uncertain
p2-8            3e+05   -17.88  (1.0)   -16.10  (0.5)   no
p2-9            8e+04   -38.73  (2.0)   -24.38  (1.0)   no
p2-T            8e+04   -54.45  (1.8)   -34.92  (0.9)   no
p2-A            8e+04   -67.11  (1.5)   -51.59  (0.9)   no
```

A new text file called `bs.txt` with the strategy should be created from scratch:

```
##    2  3  4  5  6  7  8  9  T  A
h20  s  s  s  s  s  s  s  s  s  s  
h19  s  s  s  s  s  s  s  s  s  s  
h18  s  s  s  s  s  s  s  s  s  s  
h17  s  s  s  s  s  s  s  s  s  s  
h16  s  s  s  s  s  h  h  h  h  h  
h15  s  s  s  s  s  h  h  h  h  h  
h14  s  s  s  s  s  h  h  h  h  h  
h13  s  s  s  s  s  h  h  h  h  h  
h12  h  h  s  s  s  h  h  h  h  h  
h11  d  d  d  d  d  d  d  d  d  d  
h10  d  d  d  d  d  d  d  d  h  h  
h9   h  d  d  d  d  h  h  h  h  h  
h8   h  h  h  h  h  h  h  h  h  h  
h7   h  h  h  h  h  h  h  h  h  h  
h6   h  h  h  h  h  h  h  h  h  h  
h5   h  h  h  h  h  h  h  h  h  h  
h4   h  h  h  h  h  h  h  h  h  h  
##    2  3  4  5  6  7  8  9  T  A
s20  s  s  s  s  s  s  s  s  s  s  
s19  s  s  s  s  d  s  s  s  s  s  
s18  d  d  d  d  d  s  s  h  h  h  
s17  h  d  d  d  d  h  h  h  h  h  
s16  h  h  d  d  d  h  h  h  h  h  
s15  h  h  d  d  d  h  h  h  h  h  
s14  h  h  h  d  d  h  h  h  h  h  
s13  h  h  h  h  d  h  h  h  h  h  
s12  h  h  h  h  d  h  h  h  h  h  
##    2  3  4  5  6  7  8  9  T  A
pA   y  y  y  y  y  y  y  y  y  y  
pT   n  n  n  n  n  n  n  n  n  n  
p9   y  y  y  y  y  n  y  y  n  n  
p8   y  y  y  y  y  y  y  y  y  y  
p7   y  y  y  y  y  y  n  n  n  n  
p6   y  y  y  y  y  n  n  n  n  n  
p5   n  n  n  n  n  n  n  n  n  n  
p4   n  n  n  y  y  n  n  n  n  n  
p3   y  y  y  y  y  y  n  n  n  n  
p2   y  y  y  y  y  y  n  n  n  n  
```

This is the format that the internal player (i.e. `blackjack -i`) can read and undertand.


### Full table with results

The script computes the expected value of each combination

 1. Player’s hand (hard, soft and pair)
 2. Dealer upcard
 3. Hit, double or stand (for hard and soft hands) and splitting or not (for pairs)
 
The results are given as the expected value in percentage with the uncertainty (one standard deviation) in the last significant digit.
 

 
```{=html}
<table class="table table-sm table-responsive table-hover small w-100">
 <thead>
  <tr>
   <th class="text-center" width="10%" colspan="2">Hand</th>
   <th class="text-center" width="9%">2</th>
   <th class="text-center" width="9%">3</th>
   <th class="text-center" width="9%">4</th>
   <th class="text-center" width="9%">5</th>
   <th class="text-center" width="9%">6</th>
   <th class="text-center" width="9%">7</th>
   <th class="text-center" width="9%">8</th>
   <th class="text-center" width="9%">9</th>
   <th class="text-center" width="9%">T</th>
   <th class="text-center" width="9%">A</th>
  </tr>
 </thead> 

 <tbody> 
 <!-- 2 -->
 <td>
<div class="text-center " style='background-color: rgb(101,233,139)'>+48(2)</div><div class="text-center " style='background-color: rgb(169,190,152)'>+8(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(94,237,136)'>+52(2)</div><div class="text-center " style='background-color: rgb(165,193,152)'>+10(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(85,240,134)'>+57(2)</div><div class="text-center " style='background-color: rgb(162,196,152)'>+12(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(76,243,131)'>+61(2)</div><div class="text-center " style='background-color: rgb(156,201,151)'>+16(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(67,246,127)'>+66(2)</div><div class="text-center " style='background-color: rgb(149,206,150)'>+20(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(103,232,139)'>+47(2)</div><div class="text-center " style='background-color: rgb(156,201,151)'>+16(1)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center " style='background-color: rgb(124,222,145)'>+35(2)</div><div class="text-center " style='background-color: rgb(165,193,152)'>+10(1)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center " style='background-color: rgb(144,209,149)'>+23(2)</div><div class="text-center text-white" style='background-color: rgb(180,180,152)'>-0(1)</div> </td>
 <!-- T -->
 <td>
<div class="text-center " style='background-color: rgb(167,192,152)'>+9(2)</div><div class="text-center text-white" style='background-color: rgb(199,159,151)'>-14(1)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,143,149)'>-24(2)</div><div class="text-center text-white" style='background-color: rgb(222,123,145)'>-35.4(9)</div> </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(186,174,152)'>-4(4)</div><div class="text-center " style='background-color: rgb(72,244,129)'>+63.3(7)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(173,186,152)'>+5(4)</div><div class="text-center " style='background-color: rgb(70,245,129)'>+64.3(7)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(157,200,151)'>+15(4)</div><div class="text-center " style='background-color: rgb(67,245,128)'>+65.6(7)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(137,214,148)'>+28(4)</div><div class="text-center " style='background-color: rgb(65,246,127)'>+66.8(7)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(119,225,144)'>+38(4)</div><div class="text-center " style='background-color: rgb(63,246,126)'>+67.9(7)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(167,192,152)'>+9(3)</div><div class="text-center " style='background-color: rgb(44,251,119)'>+77.5(6)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(209,145,149)'>-23(2)</div><div class="text-center " style='background-color: rgb(41,251,118)'>+79.0(6)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(243,76,131)'>-61(3)</div><div class="text-center " style='background-color: rgb(47,250,120)'>+76.2(6)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(250,46,120)'>-76(3)</div><div class="text-center " style='background-color: rgb(109,230,141)'>+43.6(7)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(253,28,113)'>-86(2)</div><div class="text-center " style='background-color: rgb(163,195,152)'>+11(1)</div> </td>
 <!-- 2 -->
 <td>
<div class="text-center " style='background-color: rgb(149,206,150)'>+20(2)</div><div class="text-center " style='background-color: rgb(164,195,152)'>+11(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(140,212,148)'>+26(2)</div><div class="text-center " style='background-color: rgb(159,198,151)'>+14(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(128,220,146)'>+33(2)</div><div class="text-center " style='background-color: rgb(156,201,151)'>+16(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(117,226,143)'>+39(2)</div><div class="text-center " style='background-color: rgb(151,205,150)'>+19(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(102,233,139)'>+47(2)</div><div class="text-center " style='background-color: rgb(145,209,149)'>+23(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(121,224,144)'>+36.6(9)</div><div class="text-center " style='background-color: rgb(115,227,143)'>+40.0(4)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center " style='background-color: rgb(144,210,149)'>+24(2)</div><div class="text-center " style='background-color: rgb(165,193,152)'>+10.1(8)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(191,167,152)'>-8(2)</div><div class="text-center text-white" style='background-color: rgb(204,152,150)'>-18(1)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(224,121,144)'>-37(2)</div><div class="text-center text-white" style='background-color: rgb(210,143,149)'>-24(1)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(234,100,138)'>-48.4(8)</div><div class="text-center text-white" style='background-color: rgb(233,103,139)'>-46.8(4)</div> </td>
 <!-- 2 -->
 <td>
<div class="text-center " style='background-color: rgb(168,191,152)'>+8(2)</div><div class="text-center text-white" style='background-color: rgb(216,134,147)'>-29(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(158,200,151)'>+15(2)</div><div class="text-center text-white" style='background-color: rgb(210,143,149)'>-24(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(148,207,150)'>+21(2)</div><div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(133,217,147)'>+30(2)</div><div class="text-center text-white" style='background-color: rgb(201,155,151)'>-16(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(116,226,143)'>+39(3)</div><div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(130,219,146)'>+32(2)</div><div class="text-center text-white" style='background-color: rgb(228,112,142)'>-41.8(9)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(183,176,152)'>-3(2)</div><div class="text-center text-white" style='background-color: rgb(232,105,140)'>-45.6(9)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(226,117,143)'>-39(2)</div><div class="text-center text-white" style='background-color: rgb(236,96,137)'>-50.7(9)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(240,83,133)'>-57.3(8)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(246,66,127)'>-66.5(8)</div><div class="text-center text-white" style='background-color: rgb(247,62,126)'>-68.4(4)</div> </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(197,161,151)'>-13(3)</div><div class="text-center text-white" style='background-color: rgb(215,135,148)'>-28(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(188,171,152)'>-6(3)</div><div class="text-center text-white" style='background-color: rgb(211,142,149)'>-24(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(169,190,152)'>+7(3)</div><div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(155,201,151)'>+16(3)</div><div class="text-center text-white" style='background-color: rgb(202,155,151)'>-16(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(142,211,149)'>+25(3)</div><div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(186,173,152)'>-4(2)</div><div class="text-center text-white" style='background-color: rgb(219,130,146)'>-32(1)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(226,116,143)'>-39(1)</div><div class="text-center text-white" style='background-color: rgb(224,121,144)'>-36.9(5)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,83,133)'>-57(2)</div><div class="text-center text-white" style='background-color: rgb(230,109,141)'>-43.2(9)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(247,63,126)'>-68(2)</div><div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51.0(9)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(252,36,116)'>-82(2)</div><div class="text-center text-white" style='background-color: rgb(244,73,130)'>-63.0(8)</div> </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(2)</div><div class="text-center text-white" style='background-color: rgb(211,141,149)'>-25(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(198,160,151)'>-13(3)</div><div class="text-center text-white" style='background-color: rgb(209,145,149)'>-23(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(182,178,152)'>-1(3)</div><div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(167,192,152)'>+9(3)</div><div class="text-center text-white" style='background-color: rgb(202,155,151)'>-16(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(153,203,150)'>+18(3)</div><div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(212,140,148)'>-26(2)</div><div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(1)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(229,111,142)'>-42(2)</div><div class="text-center text-white" style='background-color: rgb(214,138,148)'>-27(1)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(243,76,131)'>-61(2)</div><div class="text-center text-white" style='background-color: rgb(221,126,145)'>-34(1)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(249,52,122)'>-74(2)</div><div class="text-center text-white" style='background-color: rgb(229,110,141)'>-43.0(9)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(252,37,116)'>-81(2)</div><div class="text-center text-white" style='background-color: rgb(240,83,133)'>-57.5(8)</div> </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(216,134,147)'>-29(2)</div><div class="text-center " style='background-color: rgb(123,223,145)'>+36(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(2)</div><div class="text-center " style='background-color: rgb(114,227,142)'>+41(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(199,159,151)'>-14(3)</div><div class="text-center " style='background-color: rgb(104,232,140)'>+46(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(183,177,152)'>-2(3)</div><div class="text-center " style='background-color: rgb(95,236,137)'>+51(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(169,190,152)'>+8(3)</div><div class="text-center " style='background-color: rgb(87,239,134)'>+55(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(217,133,147)'>-30(2)</div><div class="text-center " style='background-color: rgb(118,226,143)'>+39(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(232,105,140)'>-46(2)</div><div class="text-center " style='background-color: rgb(133,217,147)'>+30(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(244,72,129)'>-63(2)</div><div class="text-center " style='background-color: rgb(157,200,151)'>+15(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(250,48,121)'>-76(2)</div><div class="text-center text-white" style='background-color: rgb(187,172,152)'>-5(1)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(252,32,115)'>-83(1)</div><div class="text-center text-white" style='background-color: rgb(216,135,147)'>-29(1)</div> </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(204,152,150)'>-18(2)</div><div class="text-center text-white" style='background-color: rgb(183,177,152)'>-2(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(194,165,152)'>-10(3)</div><div class="text-center text-white" style='background-color: rgb(180,180,152)'>-0(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(182,178,152)'>-2(3)</div><div class="text-center " style='background-color: rgb(174,186,152)'>+4(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(168,191,152)'>+8.3(7)</div><div class="text-center " style='background-color: rgb(170,189,152)'>+6.9(3)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(152,204,150)'>+19(3)</div><div class="text-center " style='background-color: rgb(166,192,152)'>+9(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,154,151)'>-17(2)</div><div class="text-center " style='background-color: rgb(168,191,152)'>+8(1)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(219,129,146)'>-32(2)</div><div class="text-center text-white" style='background-color: rgb(188,171,152)'>-6(1)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(237,93,136)'>-52(2)</div><div class="text-center text-white" style='background-color: rgb(207,147,150)'>-21(1)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(245,68,128)'>-65(2)</div><div class="text-center text-white" style='background-color: rgb(218,131,147)'>-31(1)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(250,48,121)'>-76(2)</div><div class="text-center text-white" style='background-color: rgb(234,99,138)'>-48.9(9)</div> </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(198,160,151)'>-13.55(1)</div><div class="text-center text-white" style='background-color: rgb(198,159,151)'>-13.84(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(186,173,152)'>-5(1)</div><div class="text-center text-white" style='background-color: rgb(194,164,152)'>-10.6(5)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(174,186,152)'>+4(3)</div><div class="text-center text-white" style='background-color: rgb(190,169,152)'>-8(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(161,197,151)'>+13(3)</div><div class="text-center text-white" style='background-color: rgb(184,176,152)'>-3(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(146,208,149)'>+22(3)</div><div class="text-center " style='background-color: rgb(179,180,152)'>+0(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(188,172,152)'>-6(2)</div><div class="text-center text-white" style='background-color: rgb(199,158,151)'>-15(1)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(209,144,149)'>-23(1)</div><div class="text-center text-white" style='background-color: rgb(208,147,150)'>-21.6(5)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(230,109,141)'>-43(2)</div><div class="text-center text-white" style='background-color: rgb(217,133,147)'>-30(1)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,80,132)'>-59(2)</div><div class="text-center text-white" style='background-color: rgb(226,117,143)'>-38.9(9)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,59,125)'>-70(2)</div><div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54.1(9)</div> </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(191,168,152)'>-8(2)</div><div class="text-center text-white" style='background-color: rgb(195,163,152)'>-12(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(182,177,152)'>-2(2)</div><div class="text-center text-white" style='background-color: rgb(191,168,152)'>-8(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(172,187,152)'>+5(3)</div><div class="text-center text-white" style='background-color: rgb(186,173,152)'>-4(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(155,202,151)'>+17(3)</div><div class="text-center text-white" style='background-color: rgb(182,178,152)'>-1(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(140,212,148)'>+26(3)</div><div class="text-center " style='background-color: rgb(176,183,152)'>+3(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(179,181,152)'>+1(2)</div><div class="text-center text-white" style='background-color: rgb(192,167,152)'>-9(1)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(203,153,151)'>-17.8(5)</div><div class="text-center text-white" style='background-color: rgb(201,156,151)'>-16.0(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(227,115,143)'>-40(2)</div><div class="text-center text-white" style='background-color: rgb(210,143,149)'>-24(1)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,91,136)'>-53(2)</div><div class="text-center text-white" style='background-color: rgb(221,125,145)'>-34.2(9)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(246,64,126)'>-67(2)</div><div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51.1(9)</div> </td>

 </tbody>
 <thead>
  <tr>
   <th class="text-center" width="10%" colspan="2">Hand</th>
   <th class="text-center" width="9%">2</th>
   <th class="text-center" width="9%">3</th>
   <th class="text-center" width="9%">4</th>
   <th class="text-center" width="9%">5</th>
   <th class="text-center" width="9%">6</th>
   <th class="text-center" width="9%">7</th>
   <th class="text-center" width="9%">8</th>
   <th class="text-center" width="9%">9</th>
   <th class="text-center" width="9%">T</th>
   <th class="text-center" width="9%">A</th>
  </tr>
 </thead> 

 <tbody> 
 <tr>
  <td>s20</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center " style='background-color: rgb(72,244,129)'>+63(2)</div><div class="text-center " style='background-color: rgb(153,203,150)'>+18(1)</div><div class="text-center " style='background-color: rgb(122,223,144)'>+36(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(70,245,128)'>+65(2)</div><div class="text-center " style='background-color: rgb(149,206,150)'>+20(1)</div><div class="text-center " style='background-color: rgb(112,228,142)'>+42(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(68,245,128)'>+65(2)</div><div class="text-center " style='background-color: rgb(145,209,149)'>+23(1)</div><div class="text-center " style='background-color: rgb(105,232,140)'>+46(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(66,246,127)'>+67(2)</div><div class="text-center " style='background-color: rgb(139,213,148)'>+26(1)</div><div class="text-center " style='background-color: rgb(96,236,137)'>+51(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(62,247,126)'>+68(2)</div><div class="text-center " style='background-color: rgb(136,215,148)'>+28(1)</div><div class="text-center " style='background-color: rgb(85,240,134)'>+57(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(44,251,119)'>+77(2)</div><div class="text-center " style='background-color: rgb(139,213,148)'>+26(1)</div><div class="text-center " style='background-color: rgb(115,227,143)'>+40(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center " style='background-color: rgb(41,251,118)'>+79(2)</div><div class="text-center " style='background-color: rgb(150,205,150)'>+19(1)</div><div class="text-center " style='background-color: rgb(135,215,147)'>+28(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center " style='background-color: rgb(47,250,120)'>+76(2)</div><div class="text-center " style='background-color: rgb(162,196,152)'>+12(1)</div><div class="text-center " style='background-color: rgb(156,201,151)'>+16(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center " style='background-color: rgb(108,230,141)'>+44(2)</div><div class="text-center text-white" style='background-color: rgb(187,172,152)'>-5(1)</div><div class="text-center text-white" style='background-color: rgb(191,168,152)'>-8(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center " style='background-color: rgb(164,194,152)'>+11(2)</div><div class="text-center text-white" style='background-color: rgb(215,135,147)'>-28(1)</div><div class="text-center text-white" style='background-color: rgb(222,125,145)'>-35(2)</div> </td>
 <tr>
  <td>s19</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center " style='background-color: rgb(120,224,144)'>+38(2)</div><div class="text-center " style='background-color: rgb(162,196,152)'>+12(1)</div><div class="text-center " style='background-color: rgb(144,210,149)'>+23(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(116,226,143)'>+39(2)</div><div class="text-center " style='background-color: rgb(158,199,151)'>+14(1)</div><div class="text-center " style='background-color: rgb(135,216,147)'>+29(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(112,228,142)'>+42(2)</div><div class="text-center " style='background-color: rgb(153,203,150)'>+18(1)</div><div class="text-center " style='background-color: rgb(125,221,145)'>+34(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(108,230,141)'>+44(1)</div><div class="text-center " style='background-color: rgb(149,206,150)'>+20.2(5)</div><div class="text-center " style='background-color: rgb(116,226,143)'>+40(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(106,231,140)'>+45.3(2)</div><div class="text-center " style='background-color: rgb(144,209,149)'>+23.1(1)</div><div class="text-center " style='background-color: rgb(104,232,140)'>+46.2(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(74,243,130)'>+62(2)</div><div class="text-center " style='background-color: rgb(145,209,149)'>+23(1)</div><div class="text-center " style='background-color: rgb(129,219,146)'>+32(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center " style='background-color: rgb(79,242,132)'>+60(2)</div><div class="text-center " style='background-color: rgb(157,200,151)'>+15(1)</div><div class="text-center " style='background-color: rgb(149,206,150)'>+20(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center " style='background-color: rgb(135,216,147)'>+29(2)</div><div class="text-center " style='background-color: rgb(178,182,152)'>+1(1)</div><div class="text-center text-white" style='background-color: rgb(190,170,152)'>-7(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(182,177,152)'>-2(2)</div><div class="text-center text-white" style='background-color: rgb(200,157,151)'>-15(1)</div><div class="text-center text-white" style='background-color: rgb(216,134,147)'>-29(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(203,153,151)'>-18(2)</div><div class="text-center text-white" style='background-color: rgb(221,125,145)'>-34.4(9)</div><div class="text-center text-white" style='background-color: rgb(233,103,139)'>-47(2)</div> </td>
 <tr>
  <td>s18</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center " style='background-color: rgb(164,195,152)'>+11.01(1)</div><div class="text-center " style='background-color: rgb(171,188,152)'>+6.00(1)</div><div class="text-center " style='background-color: rgb(163,195,152)'>+11.43(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(159,198,151)'>+14(1)</div><div class="text-center " style='background-color: rgb(167,192,152)'>+8.8(5)</div><div class="text-center " style='background-color: rgb(153,203,151)'>+18(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(155,201,151)'>+16(2)</div><div class="text-center " style='background-color: rgb(162,196,152)'>+12(1)</div><div class="text-center " style='background-color: rgb(146,209,149)'>+22(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(150,205,150)'>+20(2)</div><div class="text-center " style='background-color: rgb(159,199,151)'>+14(1)</div><div class="text-center " style='background-color: rgb(134,216,147)'>+29(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(146,208,149)'>+22(2)</div><div class="text-center " style='background-color: rgb(156,201,151)'>+16(1)</div><div class="text-center " style='background-color: rgb(123,223,145)'>+36(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(115,227,143)'>+40(2)</div><div class="text-center " style='background-color: rgb(154,202,151)'>+17(1)</div><div class="text-center " style='background-color: rgb(147,208,150)'>+22(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center " style='background-color: rgb(164,194,152)'>+11(2)</div><div class="text-center " style='background-color: rgb(174,185,152)'>+4(1)</div><div class="text-center text-white" style='background-color: rgb(183,177,152)'>-2(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(204,152,150)'>-18(2)</div><div class="text-center text-white" style='background-color: rgb(194,165,152)'>-10(1)</div><div class="text-center text-white" style='background-color: rgb(215,136,148)'>-28(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(210,143,149)'>-24(2)</div><div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(1)</div><div class="text-center text-white" style='background-color: rgb(226,117,143)'>-39(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(232,104,140)'>-46(2)</div><div class="text-center text-white" style='background-color: rgb(228,112,142)'>-41.6(9)</div><div class="text-center text-white" style='background-color: rgb(242,79,132)'>-60(2)</div> </td>
 <tr>
  <td>s17</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(201,156,151)'>-15.67(1)</div><div class="text-center text-white" style='background-color: rgb(181,179,152)'>-0.59(1)</div><div class="text-center text-white" style='background-color: rgb(181,179,152)'>-0.88(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(1)</div><div class="text-center " style='background-color: rgb(176,183,152)'>+2.4(5)</div><div class="text-center " style='background-color: rgb(171,188,152)'>+6(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(191,168,152)'>-8(2)</div><div class="text-center " style='background-color: rgb(172,187,152)'>+5(1)</div><div class="text-center " style='background-color: rgb(163,195,152)'>+12(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(186,173,152)'>-4(2)</div><div class="text-center " style='background-color: rgb(166,192,152)'>+9(1)</div><div class="text-center " style='background-color: rgb(152,204,150)'>+19(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(180,179,152)'>-0(2)</div><div class="text-center " style='background-color: rgb(164,194,152)'>+10(1)</div><div class="text-center " style='background-color: rgb(140,212,148)'>+26(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(194,164,152)'>-11(2)</div><div class="text-center " style='background-color: rgb(171,188,152)'>+6(1)</div><div class="text-center text-white" style='background-color: rgb(183,177,152)'>-2(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(225,119,144)'>-38(2)</div><div class="text-center text-white" style='background-color: rgb(190,169,152)'>-8(1)</div><div class="text-center text-white" style='background-color: rgb(211,141,149)'>-25(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(229,111,142)'>-43(2)</div><div class="text-center text-white" style='background-color: rgb(199,159,151)'>-14(1)</div><div class="text-center text-white" style='background-color: rgb(226,116,143)'>-39(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(232,104,139)'>-46(2)</div><div class="text-center text-white" style='background-color: rgb(212,140,148)'>-26(1)</div><div class="text-center text-white" style='background-color: rgb(235,97,137)'>-50(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(246,65,127)'>-67(2)</div><div class="text-center text-white" style='background-color: rgb(232,103,139)'>-46.6(9)</div><div class="text-center text-white" style='background-color: rgb(247,61,125)'>-69(2)</div> </td>
 <tr>
  <td>s16</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(216,135,147)'>-29(2)</div><div class="text-center text-white" style='background-color: rgb(183,177,152)'>-2(1)</div><div class="text-center text-white" style='background-color: rgb(190,168,152)'>-8(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-25(1)</div><div class="text-center " style='background-color: rgb(179,180,152)'>+0.3(5)</div><div class="text-center text-white" style='background-color: rgb(181,178,152)'>-1(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(1)</div><div class="text-center " style='background-color: rgb(175,185,152)'>+3.6(5)</div><div class="text-center " style='background-color: rgb(172,188,152)'>+6(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(201,155,151)'>-16(2)</div><div class="text-center " style='background-color: rgb(170,189,152)'>+7(1)</div><div class="text-center " style='background-color: rgb(160,198,151)'>+14(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(197,161,152)'>-13(2)</div><div class="text-center " style='background-color: rgb(167,191,152)'>+8(1)</div><div class="text-center " style='background-color: rgb(150,205,150)'>+20(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,101,139)'>-48(2)</div><div class="text-center text-white" style='background-color: rgb(180,179,152)'>-0(1)</div><div class="text-center text-white" style='background-color: rgb(203,153,151)'>-18(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(189,170,152)'>-7(1)</div><div class="text-center text-white" style='background-color: rgb(218,130,146)'>-31(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(199,158,151)'>-15(1)</div><div class="text-center text-white" style='background-color: rgb(231,106,140)'>-45(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,82,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(214,138,148)'>-27(1)</div><div class="text-center text-white" style='background-color: rgb(239,86,134)'>-56(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(249,54,123)'>-73(2)</div><div class="text-center text-white" style='background-color: rgb(231,107,141)'>-44.4(9)</div><div class="text-center text-white" style='background-color: rgb(248,56,123)'>-72(2)</div> </td>
 <tr>
  <td>s15</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,136,148)'>-28(2)</div><div class="text-center text-white" style='background-color: rgb(181,179,152)'>-1(1)</div><div class="text-center text-white" style='background-color: rgb(189,170,152)'>-7(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,141,149)'>-25(2)</div><div class="text-center " style='background-color: rgb(176,184,152)'>+3(1)</div><div class="text-center text-white" style='background-color: rgb(180,180,152)'>-0(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(206,148,150)'>-20.6(3)</div><div class="text-center " style='background-color: rgb(172,188,152)'>+5.6(1)</div><div class="text-center " style='background-color: rgb(171,188,152)'>+6.2(3)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(201,155,151)'>-16(2)</div><div class="text-center " style='background-color: rgb(167,192,152)'>+9(1)</div><div class="text-center " style='background-color: rgb(159,198,151)'>+14(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(197,161,151)'>-13(2)</div><div class="text-center " style='background-color: rgb(165,193,152)'>+10(1)</div><div class="text-center " style='background-color: rgb(149,206,150)'>+20(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,101,139)'>-48(2)</div><div class="text-center " style='background-color: rgb(175,184,152)'>+3(1)</div><div class="text-center text-white" style='background-color: rgb(204,152,150)'>-18(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(184,176,152)'>-3(1)</div><div class="text-center text-white" style='background-color: rgb(219,130,146)'>-32(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(239,88,135)'>-55(2)</div><div class="text-center text-white" style='background-color: rgb(195,163,152)'>-11(1)</div><div class="text-center text-white" style='background-color: rgb(232,104,140)'>-46(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,83,133)'>-57(2)</div><div class="text-center text-white" style='background-color: rgb(210,143,149)'>-24(1)</div><div class="text-center text-white" style='background-color: rgb(239,88,135)'>-55(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(229,111,142)'>-42.4(9)</div><div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72(2)</div> </td>
 <tr>
  <td>s14</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,136,148)'>-28(2)</div><div class="text-center " style='background-color: rgb(177,182,152)'>+2(1)</div><div class="text-center text-white" style='background-color: rgb(188,171,152)'>-6(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-24(2)</div><div class="text-center " style='background-color: rgb(173,186,152)'>+4(1)</div><div class="text-center text-white" style='background-color: rgb(181,179,152)'>-1(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(2)</div><div class="text-center " style='background-color: rgb(168,191,152)'>+8(1)</div><div class="text-center " style='background-color: rgb(171,188,152)'>+6(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(201,155,151)'>-16(1)</div><div class="text-center " style='background-color: rgb(164,194,152)'>+10.8(5)</div><div class="text-center " style='background-color: rgb(162,196,152)'>+12(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center " style='background-color: rgb(162,196,152)'>+12(1)</div><div class="text-center " style='background-color: rgb(150,205,150)'>+20(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,102,139)'>-48(2)</div><div class="text-center " style='background-color: rgb(168,191,152)'>+8(1)</div><div class="text-center text-white" style='background-color: rgb(204,152,150)'>-19(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,94,137)'>-51(2)</div><div class="text-center " style='background-color: rgb(177,183,152)'>+2(1)</div><div class="text-center text-white" style='background-color: rgb(219,129,146)'>-32(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(190,169,152)'>-8(1)</div><div class="text-center text-white" style='background-color: rgb(231,105,140)'>-45(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,84,133)'>-57(2)</div><div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(1)</div><div class="text-center text-white" style='background-color: rgb(240,86,134)'>-56(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(249,54,123)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(226,116,143)'>-39.8(9)</div><div class="text-center text-white" style='background-color: rgb(248,57,124)'>-71(2)</div> </td>
 <tr>
  <td>s13</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(216,135,147)'>-29(2)</div><div class="text-center " style='background-color: rgb(174,186,152)'>+4(1)</div><div class="text-center text-white" style='background-color: rgb(190,170,152)'>-7(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-24(2)</div><div class="text-center " style='background-color: rgb(170,189,152)'>+7(1)</div><div class="text-center text-white" style='background-color: rgb(180,179,152)'>-0(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(2)</div><div class="text-center " style='background-color: rgb(165,194,152)'>+10(1)</div><div class="text-center " style='background-color: rgb(172,187,152)'>+5(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,155,151)'>-16.46(1)</div><div class="text-center " style='background-color: rgb(161,197,151)'>+12.88(1)</div><div class="text-center " style='background-color: rgb(161,197,151)'>+12.73(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(197,161,151)'>-13(2)</div><div class="text-center " style='background-color: rgb(159,198,151)'>+14(1)</div><div class="text-center " style='background-color: rgb(151,205,150)'>+19(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,101,139)'>-48(2)</div><div class="text-center " style='background-color: rgb(161,197,152)'>+12(1)</div><div class="text-center text-white" style='background-color: rgb(204,152,150)'>-19(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,94,137)'>-51(2)</div><div class="text-center " style='background-color: rgb(172,187,152)'>+6(1)</div><div class="text-center text-white" style='background-color: rgb(220,128,146)'>-33(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(184,175,152)'>-3(1)</div><div class="text-center text-white" style='background-color: rgb(232,105,140)'>-46(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,84,133)'>-57(2)</div><div class="text-center text-white" style='background-color: rgb(203,154,151)'>-17(1)</div><div class="text-center text-white" style='background-color: rgb(239,87,134)'>-55(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(224,120,144)'>-37.2(9)</div><div class="text-center text-white" style='background-color: rgb(249,54,123)'>-72(2)</div> </td>
 <tr>
  <td>s12</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(216,135,147)'>-29(2)</div><div class="text-center " style='background-color: rgb(169,190,152)'>+7(1)</div><div class="text-center text-white" style='background-color: rgb(188,171,152)'>-6(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-25(2)</div><div class="text-center " style='background-color: rgb(165,193,152)'>+10(1)</div><div class="text-center text-white" style='background-color: rgb(180,179,152)'>-0(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(2)</div><div class="text-center " style='background-color: rgb(162,196,152)'>+12(1)</div><div class="text-center " style='background-color: rgb(171,189,152)'>+6(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(201,155,151)'>-16(2)</div><div class="text-center " style='background-color: rgb(157,200,151)'>+15(1)</div><div class="text-center " style='background-color: rgb(161,197,151)'>+13(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center " style='background-color: rgb(157,200,151)'>+15(1)</div><div class="text-center " style='background-color: rgb(148,207,150)'>+21(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,103,139)'>-47(2)</div><div class="text-center " style='background-color: rgb(155,202,151)'>+17(1)</div><div class="text-center text-white" style='background-color: rgb(204,152,150)'>-18(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center " style='background-color: rgb(166,193,152)'>+9(1)</div><div class="text-center text-white" style='background-color: rgb(218,131,146)'>-31(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,90,135)'>-54(2)</div><div class="text-center " style='background-color: rgb(179,180,152)'>+0(1)</div><div class="text-center text-white" style='background-color: rgb(231,106,140)'>-45(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,82,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(199,158,151)'>-15(1)</div><div class="text-center text-white" style='background-color: rgb(240,84,133)'>-57(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(222,124,145)'>-35.2(9)</div><div class="text-center text-white" style='background-color: rgb(249,54,123)'>-72(2)</div> </td>

 </tbody>
 <thead>
  <tr>
   <th class="text-center" width="10%" colspan="2">Hand</th>
   <th class="text-center" width="9%">2</th>
   <th class="text-center" width="9%">3</th>
   <th class="text-center" width="9%">4</th>
   <th class="text-center" width="9%">5</th>
   <th class="text-center" width="9%">6</th>
   <th class="text-center" width="9%">7</th>
   <th class="text-center" width="9%">8</th>
   <th class="text-center" width="9%">9</th>
   <th class="text-center" width="9%">T</th>
   <th class="text-center" width="9%">A</th>
  </tr>
 </thead> 

 <tbody> 
 <tr>
  <td>h20</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center " style='background-color: rgb(72,244,129)'>+64(1)</div><div class="text-center text-white" style='background-color: rgb(253,28,113)'>-85.6(5)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-171(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(70,245,128)'>+65(1)</div><div class="text-center text-white" style='background-color: rgb(253,28,113)'>-85.6(5)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-171(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(68,245,128)'>+65(1)</div><div class="text-center text-white" style='background-color: rgb(253,28,113)'>-85.7(5)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-171(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(65,246,127)'>+67(1)</div><div class="text-center text-white" style='background-color: rgb(253,28,113)'>-85.5(5)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-171(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(63,246,126)'>+68(1)</div><div class="text-center text-white" style='background-color: rgb(253,29,113)'>-85.2(5)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-171(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(45,250,119)'>+77(1)</div><div class="text-center text-white" style='background-color: rgb(253,29,113)'>-85.1(6)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-170(1)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center " style='background-color: rgb(41,251,118)'>+79(1)</div><div class="text-center text-white" style='background-color: rgb(253,29,113)'>-85.0(6)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-170(1)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center " style='background-color: rgb(47,250,120)'>+76(1)</div><div class="text-center text-white" style='background-color: rgb(253,29,113)'>-85.0(6)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-170(1)</div> </td>
 <!-- T -->
 <td>
<div class="text-center " style='background-color: rgb(109,230,141)'>+44(1)</div><div class="text-center text-white" style='background-color: rgb(253,27,113)'>-86.1(5)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-164(1)</div> </td>
 <!-- A -->
 <td>
<div class="text-center " style='background-color: rgb(164,195,152)'>+11(1)</div><div class="text-center text-white" style='background-color: rgb(254,20,110)'>-89.6(5)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-149(1)</div> </td>
 <tr>
  <td>h19</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center " style='background-color: rgb(120,224,144)'>+37(1)</div><div class="text-center text-white" style='background-color: rgb(249,53,122)'>-73.1(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-146(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(115,227,143)'>+40(1)</div><div class="text-center text-white" style='background-color: rgb(249,53,123)'>-73.0(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-145(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(113,228,142)'>+41(1)</div><div class="text-center text-white" style='background-color: rgb(249,54,123)'>-72.8(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-146(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(109,230,141)'>+44(1)</div><div class="text-center text-white" style='background-color: rgb(249,54,123)'>-72.8(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-144(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(105,232,140)'>+46(1)</div><div class="text-center text-white" style='background-color: rgb(249,54,123)'>-72.6(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-145(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(76,243,131)'>+61(2)</div><div class="text-center text-white" style='background-color: rgb(248,56,124)'>-71.5(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-143(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center " style='background-color: rgb(79,242,132)'>+60(2)</div><div class="text-center text-white" style='background-color: rgb(248,56,124)'>-71.4(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-143(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center " style='background-color: rgb(136,215,148)'>+28(2)</div><div class="text-center text-white" style='background-color: rgb(248,56,124)'>-71.6(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-143(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(183,177,152)'>-2(1)</div><div class="text-center text-white" style='background-color: rgb(250,49,121)'>-74.9(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-143(1)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(203,153,150)'>-18(1)</div><div class="text-center text-white" style='background-color: rgb(252,37,116)'>-81.4(6)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-131(1)</div> </td>
 <tr>
  <td>h18</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center " style='background-color: rgb(164,195,152)'>+11(2)</div><div class="text-center text-white" style='background-color: rgb(243,74,130)'>-62.2(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-124(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(160,198,151)'>+13(2)</div><div class="text-center text-white" style='background-color: rgb(243,75,130)'>-61.5(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-124(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(154,202,151)'>+17(2)</div><div class="text-center text-white" style='background-color: rgb(243,75,130)'>-61.8(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-124(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(150,205,150)'>+19(2)</div><div class="text-center text-white" style='background-color: rgb(243,76,131)'>-61.4(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-124(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(146,209,149)'>+22(2)</div><div class="text-center text-white" style='background-color: rgb(243,76,131)'>-61.4(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-122(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(116,226,143)'>+39(2)</div><div class="text-center text-white" style='background-color: rgb(242,80,132)'>-59.4(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-118(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center " style='background-color: rgb(165,194,152)'>+10(2)</div><div class="text-center text-white" style='background-color: rgb(241,81,133)'>-58.4(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-117(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(204,151,150)'>-19(2)</div><div class="text-center text-white" style='background-color: rgb(243,75,130)'>-61.5(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-123(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(210,143,149)'>-24(2)</div><div class="text-center text-white" style='background-color: rgb(246,64,126)'>-67.5(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-128(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,103,139)'>-47(1)</div><div class="text-center text-white" style='background-color: rgb(249,50,121)'>-74.6(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-119(1)</div> </td>
 <tr>
  <td>h17</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(200,157,151)'>-15(2)</div><div class="text-center text-white" style='background-color: rgb(238,90,135)'>-53.9(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-109(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center text-white" style='background-color: rgb(238,90,135)'>-53.6(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-106(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(191,168,152)'>-8(2)</div><div class="text-center text-white" style='background-color: rgb(237,92,136)'>-52.9(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-106(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(186,173,152)'>-5(2)</div><div class="text-center text-white" style='background-color: rgb(236,94,137)'>-51.7(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-105(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(181,178,152)'>-1(2)</div><div class="text-center text-white" style='background-color: rgb(237,93,136)'>-51.9(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-104(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(194,164,152)'>-11(2)</div><div class="text-center text-white" style='background-color: rgb(234,101,139)'>-48.1(9)</div><div class="text-center text-white" style='background-color: rgb(254,7,104)'>-96(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(226,118,143)'>-39(2)</div><div class="text-center text-white" style='background-color: rgb(235,96,137)'>-50.4(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-101(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(229,111,142)'>-43(2)</div><div class="text-center text-white" style='background-color: rgb(239,87,134)'>-55.4(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-111(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(232,104,140)'>-46(2)</div><div class="text-center text-white" style='background-color: rgb(243,75,130)'>-61.9(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-116(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(246,67,127)'>-66(1)</div><div class="text-center text-white" style='background-color: rgb(248,57,124)'>-70.8(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-111(1)</div> </td>
 <tr>
  <td>h16</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,135,147)'>-29(2)</div><div class="text-center text-white" style='background-color: rgb(233,102,139)'>-47.5(9)</div><div class="text-center text-white" style='background-color: rgb(254,9,105)'>-95(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,141,149)'>-25(2)</div><div class="text-center text-white" style='background-color: rgb(232,103,139)'>-46.5(9)</div><div class="text-center text-white" style='background-color: rgb(254,12,106)'>-94(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(2)</div><div class="text-center text-white" style='background-color: rgb(231,105,140)'>-45.5(9)</div><div class="text-center text-white" style='background-color: rgb(254,16,108)'>-92(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,155,151)'>-16(2)</div><div class="text-center text-white" style='background-color: rgb(231,106,140)'>-45.2(9)</div><div class="text-center text-white" style='background-color: rgb(254,20,110)'>-90(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(195,163,152)'>-12(2)</div><div class="text-center text-white" style='background-color: rgb(231,107,141)'>-44.4(9)</div><div class="text-center text-white" style='background-color: rgb(253,24,111)'>-88(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,102,139)'>-47(2)</div><div class="text-center text-white" style='background-color: rgb(228,113,142)'>-41.3(9)</div><div class="text-center text-white" style='background-color: rgb(253,31,114)'>-84(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(232,105,140)'>-46.0(9)</div><div class="text-center text-white" style='background-color: rgb(254,15,108)'>-92(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-55(2)</div><div class="text-center text-white" style='background-color: rgb(236,96,137)'>-50.7(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-103(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,83,133)'>-57.57(1)</div><div class="text-center text-white" style='background-color: rgb(240,83,133)'>-57.53(1)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-107.34(1)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(249,54,123)'>-72(1)</div><div class="text-center text-white" style='background-color: rgb(247,63,126)'>-68.2(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-105(1)</div> </td>
 <tr>
  <td>h15</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,136,148)'>-28(2)</div><div class="text-center text-white" style='background-color: rgb(228,113,142)'>-41.5(9)</div><div class="text-center text-white" style='background-color: rgb(252,32,114)'>-84(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(212,141,149)'>-25(2)</div><div class="text-center text-white" style='background-color: rgb(227,114,142)'>-40.7(9)</div><div class="text-center text-white" style='background-color: rgb(252,36,116)'>-81(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(2)</div><div class="text-center text-white" style='background-color: rgb(226,116,143)'>-39.4(9)</div><div class="text-center text-white" style='background-color: rgb(251,42,118)'>-79(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,155,151)'>-17(2)</div><div class="text-center text-white" style='background-color: rgb(225,118,144)'>-38(1)</div><div class="text-center text-white" style='background-color: rgb(251,44,119)'>-78(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center text-white" style='background-color: rgb(225,119,144)'>-38(1)</div><div class="text-center text-white" style='background-color: rgb(250,49,121)'>-75(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(234,101,139)'>-48(2)</div><div class="text-center text-white" style='background-color: rgb(224,121,144)'>-37.0(9)</div><div class="text-center text-white" style='background-color: rgb(250,49,121)'>-75(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,96,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(228,113,142)'>-41.5(9)</div><div class="text-center text-white" style='background-color: rgb(252,32,114)'>-84(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(233,102,139)'>-47.3(9)</div><div class="text-center text-white" style='background-color: rgb(254,9,105)'>-95(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,83,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(238,88,135)'>-54.6(9)</div><div class="text-center text-white" style='background-color: rgb(254,0,102)'>-100(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(245,67,128)'>-65.8(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-101(2)</div> </td>
 <tr>
  <td>h14</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,135,147)'>-29(2)</div><div class="text-center text-white" style='background-color: rgb(224,121,144)'>-37(1)</div><div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,143,149)'>-24(2)</div><div class="text-center text-white" style='background-color: rgb(222,125,145)'>-35(1)</div><div class="text-center text-white" style='background-color: rgb(248,57,124)'>-71(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(2)</div><div class="text-center text-white" style='background-color: rgb(221,126,146)'>-34(1)</div><div class="text-center text-white" style='background-color: rgb(247,61,125)'>-69(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,155,151)'>-17(2)</div><div class="text-center text-white" style='background-color: rgb(219,129,146)'>-32(1)</div><div class="text-center text-white" style='background-color: rgb(245,70,129)'>-64(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center text-white" style='background-color: rgb(218,132,147)'>-31(1)</div><div class="text-center text-white" style='background-color: rgb(242,77,131)'>-61(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,101,139)'>-48(2)</div><div class="text-center text-white" style='background-color: rgb(219,129,146)'>-32(1)</div><div class="text-center text-white" style='background-color: rgb(246,66,127)'>-66(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(224,120,144)'>-37.6(9)</div><div class="text-center text-white" style='background-color: rgb(250,46,120)'>-77(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(230,109,141)'>-43.3(9)</div><div class="text-center text-white" style='background-color: rgb(253,24,111)'>-88(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,83,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(236,96,137)'>-50.7(9)</div><div class="text-center text-white" style='background-color: rgb(254,10,106)'>-95(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(244,73,130)'>-62.8(8)</div><div class="text-center text-white" style='background-color: rgb(254,4,103)'>-98(2)</div> </td>
 <tr>
  <td>h13</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,135,147)'>-29(1)</div><div class="text-center text-white" style='background-color: rgb(218,131,147)'>-31.0(5)</div><div class="text-center text-white" style='background-color: rgb(243,75,130)'>-62(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-25(2)</div><div class="text-center text-white" style='background-color: rgb(216,134,147)'>-29(1)</div><div class="text-center text-white" style='background-color: rgb(241,80,132)'>-59(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(2)</div><div class="text-center text-white" style='background-color: rgb(214,138,148)'>-27(1)</div><div class="text-center text-white" style='background-color: rgb(239,87,134)'>-55(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,154,151)'>-17(2)</div><div class="text-center text-white" style='background-color: rgb(213,139,148)'>-26(1)</div><div class="text-center text-white" style='background-color: rgb(235,97,137)'>-50(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center text-white" style='background-color: rgb(211,143,149)'>-24(1)</div><div class="text-center text-white" style='background-color: rgb(233,101,139)'>-48(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,102,139)'>-48(2)</div><div class="text-center text-white" style='background-color: rgb(213,138,148)'>-27(1)</div><div class="text-center text-white" style='background-color: rgb(241,81,132)'>-58(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(219,129,146)'>-32(1)</div><div class="text-center text-white" style='background-color: rgb(248,59,125)'>-70(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(225,118,143)'>-38.6(9)</div><div class="text-center text-white" style='background-color: rgb(252,37,117)'>-81(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,84,133)'>-57(2)</div><div class="text-center text-white" style='background-color: rgb(232,104,139)'>-46.5(9)</div><div class="text-center text-white" style='background-color: rgb(253,25,112)'>-87(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(249,54,123)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(242,78,131)'>-60.3(8)</div><div class="text-center text-white" style='background-color: rgb(254,15,108)'>-92(2)</div> </td>
 <tr>
  <td>h12</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,135,147)'>-29(2)</div><div class="text-center text-white" style='background-color: rgb(212,141,149)'>-25(1)</div><div class="text-center text-white" style='background-color: rgb(235,97,138)'>-50(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-25(1)</div><div class="text-center text-white" style='background-color: rgb(210,144,149)'>-23.2(5)</div><div class="text-center text-white" style='background-color: rgb(233,102,139)'>-47(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20.5(3)</div><div class="text-center text-white" style='background-color: rgb(207,147,150)'>-21.3(1)</div><div class="text-center text-white" style='background-color: rgb(229,110,141)'>-42.8(3)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,155,151)'>-17(1)</div><div class="text-center text-white" style='background-color: rgb(205,150,150)'>-19.5(5)</div><div class="text-center text-white" style='background-color: rgb(225,118,143)'>-39(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(195,163,152)'>-12(2)</div><div class="text-center text-white" style='background-color: rgb(203,153,151)'>-17(1)</div><div class="text-center text-white" style='background-color: rgb(222,124,145)'>-35(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,102,139)'>-47(2)</div><div class="text-center text-white" style='background-color: rgb(207,147,150)'>-21(1)</div><div class="text-center text-white" style='background-color: rgb(235,98,138)'>-50(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(214,138,148)'>-27(1)</div><div class="text-center text-white" style='background-color: rgb(243,75,130)'>-62(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,90,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(221,126,145)'>-34(1)</div><div class="text-center text-white" style='background-color: rgb(249,51,122)'>-74(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,82,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(229,110,141)'>-43.0(9)</div><div class="text-center text-white" style='background-color: rgb(252,35,116)'>-82(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(249,54,123)'>-73(2)</div><div class="text-center text-white" style='background-color: rgb(240,83,133)'>-57.4(8)</div><div class="text-center text-white" style='background-color: rgb(254,22,110)'>-89(2)</div> </td>
 <tr>
  <td>h11</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,137,148)'>-28(2)</div><div class="text-center " style='background-color: rgb(143,210,149)'>+24(1)</div><div class="text-center " style='background-color: rgb(103,232,139)'>+47(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,141,149)'>-25(2)</div><div class="text-center " style='background-color: rgb(139,213,148)'>+26(1)</div><div class="text-center " style='background-color: rgb(95,236,137)'>+51(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(2)</div><div class="text-center " style='background-color: rgb(135,216,147)'>+29(1)</div><div class="text-center " style='background-color: rgb(85,240,134)'>+56(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(201,156,151)'>-16(2)</div><div class="text-center " style='background-color: rgb(131,218,147)'>+31(1)</div><div class="text-center " style='background-color: rgb(77,242,131)'>+61(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center " style='background-color: rgb(127,220,146)'>+33(1)</div><div class="text-center " style='background-color: rgb(64,246,127)'>+67(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,102,139)'>-47(2)</div><div class="text-center " style='background-color: rgb(134,216,147)'>+29(1)</div><div class="text-center " style='background-color: rgb(104,232,140)'>+46(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center " style='background-color: rgb(145,209,149)'>+22(1)</div><div class="text-center " style='background-color: rgb(124,222,145)'>+35(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center " style='background-color: rgb(156,201,151)'>+16(1)</div><div class="text-center " style='background-color: rgb(143,210,149)'>+24(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,83,133)'>-58(2)</div><div class="text-center " style='background-color: rgb(175,184,152)'>+3(1)</div><div class="text-center " style='background-color: rgb(165,193,152)'>+10(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72.22(1)</div><div class="text-center text-white" style='background-color: rgb(210,143,149)'>-23.67(1)</div><div class="text-center text-white" style='background-color: rgb(210,144,149)'>-23.56(1)</div> </td>
 <tr>
  <td>h10</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,135,147)'>-28(2)</div><div class="text-center " style='background-color: rgb(152,204,150)'>+18(1)</div><div class="text-center " style='background-color: rgb(123,223,145)'>+36(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(210,143,149)'>-24(2)</div><div class="text-center " style='background-color: rgb(148,207,150)'>+21(1)</div><div class="text-center " style='background-color: rgb(114,227,142)'>+41(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(207,147,150)'>-21(2)</div><div class="text-center " style='background-color: rgb(144,209,149)'>+23(1)</div><div class="text-center " style='background-color: rgb(103,232,139)'>+47(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(201,155,151)'>-16(2)</div><div class="text-center " style='background-color: rgb(140,212,148)'>+26(1)</div><div class="text-center " style='background-color: rgb(94,236,137)'>+52(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center " style='background-color: rgb(135,215,147)'>+29(1)</div><div class="text-center " style='background-color: rgb(85,240,134)'>+56(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,102,139)'>-47(2)</div><div class="text-center " style='background-color: rgb(141,212,149)'>+25(1)</div><div class="text-center " style='background-color: rgb(118,225,144)'>+38(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center " style='background-color: rgb(149,206,150)'>+20(1)</div><div class="text-center " style='background-color: rgb(135,215,147)'>+29(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(1)</div><div class="text-center " style='background-color: rgb(162,196,152)'>+11.8(5)</div><div class="text-center " style='background-color: rgb(157,200,151)'>+15(1)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,82,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(188,171,152)'>-6(1)</div><div class="text-center text-white" style='background-color: rgb(191,168,152)'>-8(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(249,54,123)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(216,135,147)'>-29(1)</div><div class="text-center text-white" style='background-color: rgb(221,127,146)'>-34(2)</div> </td>
 <tr>
  <td>h9</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(216,135,147)'>-29(1)</div><div class="text-center " style='background-color: rgb(169,190,152)'>+7.6(5)</div><div class="text-center " style='background-color: rgb(171,188,152)'>+6(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-25(1)</div><div class="text-center " style='background-color: rgb(165,193,152)'>+9.9(5)</div><div class="text-center " style='background-color: rgb(162,196,152)'>+12(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(2)</div><div class="text-center " style='background-color: rgb(162,196,152)'>+12(1)</div><div class="text-center " style='background-color: rgb(153,203,151)'>+18(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,155,151)'>-16(2)</div><div class="text-center " style='background-color: rgb(156,201,151)'>+16(1)</div><div class="text-center " style='background-color: rgb(144,210,149)'>+24(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center " style='background-color: rgb(150,205,150)'>+20(1)</div><div class="text-center " style='background-color: rgb(132,218,147)'>+31(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,101,139)'>-48(2)</div><div class="text-center " style='background-color: rgb(154,202,151)'>+17(1)</div><div class="text-center " style='background-color: rgb(165,193,152)'>+10(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center " style='background-color: rgb(165,194,152)'>+10(1)</div><div class="text-center text-white" style='background-color: rgb(183,176,152)'>-2(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,90,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(186,173,152)'>-5(1)</div><div class="text-center text-white" style='background-color: rgb(217,133,147)'>-30(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,83,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(208,146,149)'>-22(1)</div><div class="text-center text-white" style='background-color: rgb(237,93,136)'>-52(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(249,54,123)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(226,117,143)'>-38.8(9)</div><div class="text-center text-white" style='background-color: rgb(243,76,131)'>-61(2)</div> </td>
 <tr>
  <td>h8</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(216,134,147)'>-29(2)</div><div class="text-center text-white" style='background-color: rgb(182,177,152)'>-2(1)</div><div class="text-center text-white" style='background-color: rgb(205,151,150)'>-19(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(212,140,148)'>-26(2)</div><div class="text-center " style='background-color: rgb(179,180,152)'>+0(1)</div><div class="text-center text-white" style='background-color: rgb(198,159,151)'>-14(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(2)</div><div class="text-center " style='background-color: rgb(174,185,152)'>+4(1)</div><div class="text-center text-white" style='background-color: rgb(190,169,152)'>-8(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(201,156,151)'>-16(2)</div><div class="text-center " style='background-color: rgb(169,190,152)'>+8(1)</div><div class="text-center " style='background-color: rgb(179,180,152)'>+0(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(1)</div><div class="text-center " style='background-color: rgb(164,194,152)'>+10.5(5)</div><div class="text-center " style='background-color: rgb(169,190,152)'>+7(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,102,139)'>-47(2)</div><div class="text-center " style='background-color: rgb(167,192,152)'>+9(1)</div><div class="text-center text-white" style='background-color: rgb(202,155,151)'>-17(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,94,137)'>-52(2)</div><div class="text-center text-white" style='background-color: rgb(188,171,152)'>-6(1)</div><div class="text-center text-white" style='background-color: rgb(232,104,140)'>-46(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(208,147,150)'>-22(1)</div><div class="text-center text-white" style='background-color: rgb(248,56,123)'>-72(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,84,133)'>-57(2)</div><div class="text-center text-white" style='background-color: rgb(218,131,147)'>-31(1)</div><div class="text-center text-white" style='background-color: rgb(250,47,120)'>-76(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(234,99,138)'>-49.0(9)</div><div class="text-center text-white" style='background-color: rgb(253,23,111)'>-88(2)</div> </td>
 <tr>
  <td>h7</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(216,134,147)'>-29(2)</div><div class="text-center text-white" style='background-color: rgb(195,163,152)'>-11(1)</div><div class="text-center text-white" style='background-color: rgb(229,111,142)'>-43(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-24(2)</div><div class="text-center text-white" style='background-color: rgb(190,169,152)'>-8(1)</div><div class="text-center text-white" style='background-color: rgb(222,125,145)'>-35(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(205,150,150)'>-19(2)</div><div class="text-center text-white" style='background-color: rgb(185,174,152)'>-4(1)</div><div class="text-center text-white" style='background-color: rgb(214,137,148)'>-28(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,154,151)'>-17(2)</div><div class="text-center text-white" style='background-color: rgb(181,179,152)'>-1(1)</div><div class="text-center text-white" style='background-color: rgb(206,148,150)'>-21(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center " style='background-color: rgb(176,183,152)'>+3(1)</div><div class="text-center text-white" style='background-color: rgb(194,164,152)'>-11(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,102,139)'>-47(2)</div><div class="text-center text-white" style='background-color: rgb(190,169,152)'>-7(1)</div><div class="text-center text-white" style='background-color: rgb(242,79,132)'>-60(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,96,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(1)</div><div class="text-center text-white" style='background-color: rgb(253,29,113)'>-85(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(216,134,147)'>-29(1)</div><div class="text-center text-white" style='background-color: rgb(254,7,105)'>-96(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,83,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(224,120,144)'>-37.1(9)</div><div class="text-center text-white" style='background-color: rgb(254,8,105)'>-96(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72(1)</div><div class="text-center text-white" style='background-color: rgb(239,87,134)'>-55.5(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-105(1)</div> </td>
 <tr>
  <td>h6</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,135,148)'>-28(2)</div><div class="text-center text-white" style='background-color: rgb(200,156,151)'>-16(1)</div><div class="text-center text-white" style='background-color: rgb(240,85,134)'>-56(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-24(2)</div><div class="text-center text-white" style='background-color: rgb(195,163,152)'>-12(1)</div><div class="text-center text-white" style='background-color: rgb(232,103,139)'>-47(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(208,147,150)'>-21(2)</div><div class="text-center text-white" style='background-color: rgb(191,168,152)'>-8(1)</div><div class="text-center text-white" style='background-color: rgb(226,117,143)'>-39(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,155,151)'>-17(2)</div><div class="text-center text-white" style='background-color: rgb(187,173,152)'>-5(1)</div><div class="text-center text-white" style='background-color: rgb(218,131,147)'>-31(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,161,152)'>-12(2)</div><div class="text-center text-white" style='background-color: rgb(181,179,152)'>-1(1)</div><div class="text-center text-white" style='background-color: rgb(209,145,149)'>-22(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,102,139)'>-48(2)</div><div class="text-center text-white" style='background-color: rgb(202,155,151)'>-16(1)</div><div class="text-center text-white" style='background-color: rgb(254,20,110)'>-90(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,96,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(211,143,149)'>-24(1)</div><div class="text-center text-white" style='background-color: rgb(254,2,103)'>-99(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(218,130,146)'>-31(1)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-107(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,83,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(227,115,143)'>-40.0(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-105(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72(1)</div><div class="text-center text-white" style='background-color: rgb(240,85,134)'>-56.3(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-112(1)</div> </td>
 <tr>
  <td>h5</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,135,147)'>-28(2)</div><div class="text-center text-white" style='background-color: rgb(199,159,151)'>-14(1)</div><div class="text-center text-white" style='background-color: rgb(239,86,134)'>-56(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-25(2)</div><div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(1)</div><div class="text-center text-white" style='background-color: rgb(235,98,138)'>-49(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(2)</div><div class="text-center text-white" style='background-color: rgb(191,168,152)'>-8(1)</div><div class="text-center text-white" style='background-color: rgb(229,110,141)'>-43(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,155,151)'>-16(2)</div><div class="text-center text-white" style='background-color: rgb(186,173,152)'>-4(1)</div><div class="text-center text-white" style='background-color: rgb(220,128,146)'>-33(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center text-white" style='background-color: rgb(180,179,152)'>-0(1)</div><div class="text-center text-white" style='background-color: rgb(211,142,149)'>-24(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,102,139)'>-47(2)</div><div class="text-center text-white" style='background-color: rgb(201,155,151)'>-16(1)</div><div class="text-center text-white" style='background-color: rgb(254,11,106)'>-94(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(208,146,149)'>-22(1)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-102(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(217,133,147)'>-30(1)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-108(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,83,133)'>-57(2)</div><div class="text-center text-white" style='background-color: rgb(226,117,143)'>-38.8(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-107(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(249,54,123)'>-72(1)</div><div class="text-center text-white" style='background-color: rgb(239,88,135)'>-54.7(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-114(1)</div> </td>
 <tr>
  <td>h4</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(216,135,147)'>-29(2)</div><div class="text-center text-white" style='background-color: rgb(197,161,151)'>-13(1)</div><div class="text-center text-white" style='background-color: rgb(240,83,133)'>-57(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-25(2)</div><div class="text-center text-white" style='background-color: rgb(194,164,152)'>-10(1)</div><div class="text-center text-white" style='background-color: rgb(235,97,138)'>-50(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(2)</div><div class="text-center text-white" style='background-color: rgb(190,169,152)'>-7(1)</div><div class="text-center text-white" style='background-color: rgb(228,113,142)'>-41(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,154,151)'>-17(2)</div><div class="text-center text-white" style='background-color: rgb(185,175,152)'>-3(1)</div><div class="text-center text-white" style='background-color: rgb(220,128,146)'>-33(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center " style='background-color: rgb(179,180,152)'>+0(1)</div><div class="text-center text-white" style='background-color: rgb(210,144,149)'>-24(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,101,139)'>-48(2)</div><div class="text-center text-white" style='background-color: rgb(197,161,151)'>-13(1)</div><div class="text-center text-white" style='background-color: rgb(254,10,106)'>-95(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,96,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(205,150,150)'>-20(1)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-103(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,90,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(215,136,148)'>-28(1)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-109(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,84,133)'>-57(2)</div><div class="text-center text-white" style='background-color: rgb(224,121,144)'>-36.6(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-107(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(249,54,123)'>-72(1)</div><div class="text-center text-white" style='background-color: rgb(238,90,135)'>-53.8(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-113(1)</div> </td>

 </tbody>
</table>
```

|  Hand  |  Hands needed  |  Stand [%]  |  Double [%]  |  Hit [%] |   Play    |
|:------:|:-----:|:-----------:|:------------:|:--------:|:---------:|
| h20-2 | 8.0e+04 | +63.55 (1.1) | -85.62 (0.5) | -171.16 (1.1) | stand | 
| h20-3 | 8.0e+04 | +64.58 (1.1) | -85.56 (0.5) | -170.56 (1.1) | stand | 
| h20-4 | 8.0e+04 | +65.30 (1.1) | -85.70 (0.5) | -171.10 (1.1) | stand | 
| h20-5 | 8.0e+04 | +67.04 (1.1) | -85.52 (0.5) | -171.34 (1.1) | stand | 
| h20-6 | 8.0e+04 | +67.81 (1.1) | -85.17 (0.5) | -170.61 (1.1) | stand | 
| h20-7 | 8.0e+04 | +77.12 (1.1) | -85.13 (0.6) | -170.32 (1.1) | stand | 
| h20-8 | 8.0e+04 | +79.14 (1.1) | -84.99 (0.6) | -170.46 (1.1) | stand | 
| h20-9 | 8.0e+04 | +75.94 (1.1) | -85.00 (0.6) | -169.53 (1.1) | stand | 
| h20-T | 8.0e+04 | +43.51 (1.1) | -86.06 (0.5) | -164.02 (1.1) | stand | 
| h20-A | 8.0e+04 | +10.86 (1.0) | -89.62 (0.5) | -149.06 (1.0) | stand | 
| h19-2 | 8.0e+04 | +37.41 (1.4) | -73.09 (0.7) | -146.25 (1.4) | stand | 
| h19-3 | 8.0e+04 | +40.09 (1.4) | -72.96 (0.7) | -145.29 (1.4) | stand | 
| h19-4 | 8.0e+04 | +41.38 (1.4) | -72.80 (0.7) | -146.07 (1.4) | stand | 
| h19-5 | 8.0e+04 | +43.76 (1.4) | -72.76 (0.7) | -144.19 (1.4) | stand | 
| h19-6 | 8.0e+04 | +45.80 (1.4) | -72.62 (0.7) | -145.14 (1.4) | stand | 
| h19-7 | 8.0e+04 | +61.39 (1.5) | -71.50 (0.7) | -143.39 (1.5) | stand | 
| h19-8 | 8.0e+04 | +59.58 (1.5) | -71.36 (0.7) | -143.03 (1.5) | stand | 
| h19-9 | 8.0e+04 | +28.15 (1.5) | -71.59 (0.7) | -143.42 (1.5) | stand | 
| h19-T | 8.0e+04 | -2.00 (1.3) | -74.91 (0.7) | -142.88 (1.3) | stand | 
| h19-A | 8.0e+04 | -18.04 (1.2) | -81.43 (0.6) | -131.46 (1.2) | stand | 
| h18-2 | 8.0e+04 | +11.02 (1.6) | -62.22 (0.8) | -123.71 (1.6) | stand | 
| h18-3 | 8.0e+04 | +13.20 (1.6) | -61.53 (0.8) | -124.35 (1.6) | stand | 
| h18-4 | 8.0e+04 | +16.92 (1.6) | -61.80 (0.8) | -123.86 (1.6) | stand | 
| h18-5 | 8.0e+04 | +19.35 (1.6) | -61.38 (0.8) | -123.62 (1.6) | stand | 
| h18-6 | 8.0e+04 | +22.36 (1.6) | -61.35 (0.8) | -121.85 (1.6) | stand | 
| h18-7 | 8.0e+04 | +39.35 (1.7) | -59.36 (0.8) | -118.48 (1.7) | stand | 
| h18-8 | 8.0e+04 | +10.34 (1.7) | -58.35 (0.8) | -117.43 (1.7) | stand | 
| h18-9 | 8.0e+04 | -18.74 (1.6) | -61.52 (0.8) | -123.34 (1.6) | stand | 
| h18-T | 8.0e+04 | -23.84 (1.5) | -67.55 (0.8) | -127.96 (1.5) | stand | 
| h18-A | 8.0e+04 | -46.97 (1.3) | -74.64 (0.7) | -118.63 (1.3) | stand | 
| h17-2 | 8.0e+04 | -15.44 (1.7) | -53.93 (0.9) | -108.61 (1.7) | stand | 
| h17-3 | 8.0e+04 | -12.18 (1.8) | -53.57 (0.9) | -105.83 (1.8) | stand | 
| h17-4 | 8.0e+04 | -8.33 (1.8) | -52.86 (0.9) | -105.69 (1.8) | stand | 
| h17-5 | 8.0e+04 | -4.55 (1.8) | -51.66 (0.9) | -105.34 (1.8) | stand | 
| h17-6 | 8.0e+04 | -1.15 (1.8) | -51.94 (0.9) | -104.07 (1.8) | stand | 
| h17-7 | 8.0e+04 | -10.79 (1.8) | -48.10 (0.9) | -96.43 (1.8) | stand | 
| h17-8 | 8.0e+04 | -38.74 (1.8) | -50.44 (0.9) | -101.03 (1.8) | stand | 
| h17-9 | 8.0e+04 | -42.54 (1.7) | -55.42 (0.8) | -111.46 (1.7) | stand | 
| h17-T | 8.0e+04 | -46.12 (1.6) | -61.91 (0.8) | -115.53 (1.6) | stand | 
| h17-A | 8.0e+04 | -66.10 (1.4) | -70.80 (0.7) | -110.62 (1.4) | stand | 
| h16-2 | 8.0e+04 | -28.62 (1.8) | -47.46 (0.9) | -95.12 (1.8) | stand | 
| h16-3 | 8.0e+04 | -24.81 (1.8) | -46.52 (0.9) | -93.95 (1.8) | stand | 
| h16-4 | 8.0e+04 | -20.67 (1.8) | -45.46 (0.9) | -91.92 (1.8) | stand | 
| h16-5 | 8.0e+04 | -16.48 (1.8) | -45.18 (0.9) | -89.56 (1.8) | stand | 
| h16-6 | 8.0e+04 | -11.55 (1.9) | -44.39 (0.9) | -87.64 (1.9) | stand | 
| h16-7 | 8.0e+04 | -47.14 (1.9) | -41.27 (0.9) | -84.33 (1.9) | hit | 
| h16-8 | 8.0e+04 | -50.94 (1.8) | -45.95 (0.9) | -92.12 (1.8) | hit | 
| h16-9 | 8.0e+04 | -54.50 (1.7) | -50.71 (0.9) | -102.58 (1.7) | hit | 
| h16-T | 2.0e+07 | -57.57 (0) | -57.53 (0) | -107.34 (0) | hit | 
| h16-A | 8.0e+04 | -72.39 (1.4) | -68.19 (0.7) | -105.24 (1.4) | hit | 
| h15-2 | 8.0e+04 | -28.36 (1.9) | -41.53 (0.9) | -83.65 (1.9) | stand | 
| h15-3 | 8.0e+04 | -25.21 (1.9) | -40.72 (0.9) | -81.48 (1.9) | stand | 
| h15-4 | 8.0e+04 | -20.66 (1.9) | -39.38 (0.9) | -78.86 (1.9) | stand | 
| h15-5 | 8.0e+04 | -16.55 (1.9) | -38.38 (1.0) | -77.57 (1.9) | stand | 
| h15-6 | 8.0e+04 | -11.88 (1.9) | -37.86 (1.0) | -75.22 (1.9) | stand | 
| h15-7 | 8.0e+04 | -48.09 (1.9) | -37.04 (0.9) | -75.35 (1.9) | hit | 
| h15-8 | 8.0e+04 | -50.84 (1.9) | -41.53 (0.9) | -83.94 (1.9) | hit | 
| h15-9 | 8.0e+04 | -54.38 (1.8) | -47.30 (0.9) | -95.06 (1.8) | hit | 
| h15-T | 8.0e+04 | -57.73 (1.7) | -54.62 (0.9) | -99.81 (1.7) | hit | 
| h15-A | 8.0e+04 | -72.00 (1.5) | -65.80 (0.8) | -101.33 (1.5) | hit | 
| h14-2 | 8.0e+04 | -28.56 (1.9) | -36.62 (1.0) | -72.17 (1.9) | stand | 
| h14-3 | 8.0e+04 | -24.12 (1.9) | -34.63 (1.0) | -70.90 (1.9) | stand | 
| h14-4 | 8.0e+04 | -20.48 (1.9) | -33.73 (1.0) | -68.97 (1.9) | stand | 
| h14-5 | 8.0e+04 | -16.54 (2.0) | -31.93 (1.0) | -64.39 (2.0) | stand | 
| h14-6 | 8.0e+04 | -12.19 (2.0) | -30.64 (1.0) | -60.75 (2.0) | stand | 
| h14-7 | 8.0e+04 | -47.85 (1.9) | -32.25 (1.0) | -66.44 (1.9) | hit | 
| h14-8 | 8.0e+04 | -51.09 (1.9) | -37.59 (0.9) | -76.59 (1.9) | hit | 
| h14-9 | 8.0e+04 | -54.41 (1.8) | -43.27 (0.9) | -87.80 (1.8) | hit | 
| h14-T | 8.0e+04 | -57.74 (1.7) | -50.71 (0.9) | -94.55 (1.7) | hit | 
| h14-A | 8.0e+04 | -72.11 (1.5) | -62.81 (0.8) | -97.57 (1.5) | hit | 
| h13-2 | 3.2e+05 | -28.58 (1.0) | -30.97 (0.5) | -61.58 (1.0) | stand | 
| h13-3 | 8.0e+04 | -24.61 (2.0) | -29.17 (1.0) | -58.99 (2.0) | stand | 
| h13-4 | 8.0e+04 | -21.15 (2.0) | -27.05 (1.0) | -55.31 (2.0) | stand | 
| h13-5 | 8.0e+04 | -16.88 (2.0) | -26.15 (1.0) | -50.31 (2.0) | stand | 
| h13-6 | 8.0e+04 | -12.35 (2.0) | -24.12 (1.0) | -47.91 (2.0) | stand | 
| h13-7 | 8.0e+04 | -47.50 (2.0) | -26.69 (1.0) | -58.47 (2.0) | hit | 
| h13-8 | 8.0e+04 | -51.33 (1.9) | -32.22 (1.0) | -70.19 (1.9) | hit | 
| h13-9 | 8.0e+04 | -54.31 (1.9) | -38.57 (0.9) | -80.97 (1.9) | hit | 
| h13-T | 8.0e+04 | -56.99 (1.8) | -46.49 (0.9) | -87.36 (1.8) | hit | 
| h13-A | 8.0e+04 | -72.39 (1.5) | -60.33 (0.8) | -92.36 (1.5) | hit | 
| h12-2 | 8.0e+04 | -28.53 (2.0) | -25.26 (1.0) | -50.10 (2.0) | hit | 
| h12-3 | 3.2e+05 | -24.74 (1.0) | -23.20 (0.5) | -47.17 (1.0) | hit | 
| h12-4 | 5.1e+06 | -20.50 (0.3) | -21.34 (0.1) | -42.80 (0.3) | stand | 
| h12-5 | 3.2e+05 | -16.67 (1.0) | -19.49 (0.5) | -38.59 (1.0) | stand | 
| h12-6 | 8.0e+04 | -11.57 (2.0) | -17.45 (1.0) | -34.91 (2.0) | stand | 
| h12-7 | 8.0e+04 | -47.16 (2.0) | -21.22 (1.0) | -49.56 (2.0) | hit | 
| h12-8 | 8.0e+04 | -51.23 (2.0) | -26.82 (1.0) | -61.96 (2.0) | hit | 
| h12-9 | 8.0e+04 | -54.02 (1.9) | -33.93 (1.0) | -74.10 (1.9) | hit | 
| h12-T | 8.0e+04 | -58.14 (1.8) | -43.03 (0.9) | -82.21 (1.8) | hit | 
| h12-A | 8.0e+04 | -72.60 (1.5) | -57.38 (0.8) | -88.89 (1.5) | hit | 
| h11-2 | 8.0e+04 | -27.73 (2.0) | +23.74 (1.0) | +46.68 (2.0) | double | 
| h11-3 | 8.0e+04 | -24.81 (2.0) | +26.30 (1.0) | +50.86 (2.0) | double | 
| h11-4 | 8.0e+04 | -20.28 (2.0) | +28.91 (1.0) | +56.46 (2.0) | double | 
| h11-5 | 8.0e+04 | -16.10 (1.9) | +31.08 (1.0) | +60.58 (1.9) | double | 
| h11-6 | 8.0e+04 | -12.18 (1.9) | +33.16 (1.0) | +67.19 (1.9) | double | 
| h11-7 | 8.0e+04 | -47.49 (2.0) | +29.49 (1.0) | +46.09 (2.0) | double | 
| h11-8 | 8.0e+04 | -51.29 (2.0) | +22.39 (1.0) | +34.94 (2.0) | double | 
| h11-9 | 8.0e+04 | -54.41 (2.0) | +15.62 (1.0) | +23.72 (2.0) | double | 
| h11-T | 8.0e+04 | -57.70 (2.0) | +3.21 (1.0) | +9.84 (2.0) | double | 
| h11-A | 2.0e+07 | -72.22 (0) | -23.67 (0) | -23.56 (0) | double | 
| h10-2 | 8.0e+04 | -28.45 (2.0) | +18.25 (1.0) | +35.73 (2.0) | double | 
| h10-3 | 8.0e+04 | -23.99 (2.0) | +20.70 (1.0) | +40.59 (2.0) | double | 
| h10-4 | 8.0e+04 | -21.31 (2.0) | +23.14 (1.0) | +46.54 (2.0) | double | 
| h10-5 | 8.0e+04 | -16.23 (2.0) | +25.77 (1.0) | +51.66 (2.0) | double | 
| h10-6 | 8.0e+04 | -12.01 (2.0) | +28.63 (1.0) | +56.36 (2.0) | double | 
| h10-7 | 8.0e+04 | -47.40 (2.0) | +25.23 (1.0) | +38.38 (2.0) | double | 
| h10-8 | 8.0e+04 | -51.35 (2.0) | +20.43 (1.0) | +28.63 (2.0) | double | 
| h10-9 | 3.2e+05 | -54.13 (1.0) | +11.81 (0.5) | +15.23 (1.0) | double | 
| h10-T | 8.0e+04 | -58.04 (1.9) | -5.84 (1.0) | -7.94 (1.9) | hit | 
| h10-A | 8.0e+04 | -72.46 (1.7) | -28.90 (1.0) | -33.54 (1.7) | hit | 
| h9-2 | 3.2e+05 | -28.82 (1.0) | +7.62 (0.5) | +5.95 (1.0) | hit | 
| h9-3 | 3.2e+05 | -24.59 (1.0) | +9.93 (0.5) | +11.94 (1.0) | double | 
| h9-4 | 8.0e+04 | -20.79 (2.0) | +12.15 (1.0) | +17.76 (2.0) | double | 
| h9-5 | 8.0e+04 | -16.50 (2.0) | +15.92 (1.0) | +23.51 (2.0) | double | 
| h9-6 | 8.0e+04 | -12.35 (2.0) | +19.57 (1.0) | +30.65 (2.0) | double | 
| h9-7 | 8.0e+04 | -47.62 (2.0) | +17.20 (1.0) | +10.00 (2.0) | hit | 
| h9-8 | 8.0e+04 | -50.97 (2.0) | +10.15 (1.0) | -2.38 (2.0) | hit | 
| h9-9 | 8.0e+04 | -53.95 (1.9) | -4.61 (1.0) | -29.84 (1.9) | hit | 
| h9-T | 8.0e+04 | -57.78 (1.9) | -21.96 (1.0) | -52.36 (1.9) | hit | 
| h9-A | 8.0e+04 | -72.33 (1.7) | -38.77 (0.9) | -61.26 (1.7) | hit | 
| h8-2 | 8.0e+04 | -29.03 (2.0) | -1.76 (1.0) | -19.15 (2.0) | hit | 
| h8-3 | 8.0e+04 | -25.61 (2.1) | +0.33 (1.0) | -13.68 (2.1) | hit | 
| h8-4 | 8.0e+04 | -20.69 (2.1) | +3.82 (1.0) | -7.66 (2.1) | hit | 
| h8-5 | 8.0e+04 | -16.06 (2.1) | +7.53 (1.0) | +0.32 (2.1) | hit | 
| h8-6 | 3.2e+05 | -12.16 (1.0) | +10.49 (0.5) | +7.43 (1.0) | hit | 
| h8-7 | 8.0e+04 | -47.34 (2.0) | +8.56 (1.0) | -16.64 (2.0) | hit | 
| h8-8 | 8.0e+04 | -51.56 (1.9) | -6.18 (1.0) | -46.07 (1.9) | hit | 
| h8-9 | 8.0e+04 | -54.44 (1.9) | -21.65 (1.0) | -71.64 (1.9) | hit | 
| h8-T | 8.0e+04 | -57.15 (1.8) | -30.78 (1.0) | -76.35 (1.8) | hit | 
| h8-A | 8.0e+04 | -72.01 (1.5) | -49.02 (0.9) | -88.47 (1.5) | hit | 
| h7-2 | 8.0e+04 | -29.15 (2.0) | -11.08 (1.0) | -42.53 (2.0) | hit | 
| h7-3 | 8.0e+04 | -24.37 (2.0) | -7.51 (1.0) | -34.68 (2.0) | hit | 
| h7-4 | 8.0e+04 | -19.40 (2.1) | -3.90 (1.0) | -27.57 (2.1) | hit | 
| h7-5 | 8.0e+04 | -16.84 (2.1) | -0.53 (1.0) | -20.59 (2.1) | hit | 
| h7-6 | 8.0e+04 | -11.93 (2.1) | +2.53 (1.0) | -10.66 (2.1) | hit | 
| h7-7 | 8.0e+04 | -47.22 (1.9) | -7.33 (1.0) | -59.71 (1.9) | hit | 
| h7-8 | 8.0e+04 | -50.71 (1.8) | -20.35 (1.0) | -85.02 (1.8) | hit | 
| h7-9 | 8.0e+04 | -54.12 (1.8) | -29.11 (1.0) | -96.16 (1.8) | hit | 
| h7-T | 8.0e+04 | -57.52 (1.7) | -37.06 (0.9) | -95.95 (1.7) | hit | 
| h7-A | 8.0e+04 | -72.23 (1.4) | -55.54 (0.8) | -104.99 (1.4) | hit | 
| h6-2 | 8.0e+04 | -28.38 (2.0) | -15.59 (1.0) | -56.44 (2.0) | hit | 
| h6-3 | 8.0e+04 | -24.34 (2.1) | -11.56 (1.0) | -46.62 (2.1) | hit | 
| h6-4 | 8.0e+04 | -21.46 (2.1) | -8.03 (1.0) | -39.24 (2.1) | hit | 
| h6-5 | 8.0e+04 | -16.68 (2.1) | -4.95 (1.0) | -31.10 (2.1) | hit | 
| h6-6 | 8.0e+04 | -12.40 (2.1) | -0.76 (1.0) | -22.49 (2.1) | hit | 
| h6-7 | 8.0e+04 | -47.58 (1.9) | -16.43 (1.0) | -89.56 (1.9) | hit | 
| h6-8 | 8.0e+04 | -50.72 (1.8) | -24.13 (1.0) | -98.62 (1.8) | hit | 
| h6-9 | 8.0e+04 | -54.32 (1.8) | -31.34 (1.0) | -107.41 (1.8) | hit | 
| h6-T | 8.0e+04 | -57.56 (1.7) | -40.02 (0.9) | -105.28 (1.7) | hit | 
| h6-A | 8.0e+04 | -72.04 (1.4) | -56.31 (0.8) | -112.37 (1.4) | hit | 
| h5-2 | 8.0e+04 | -28.46 (2.0) | -14.09 (1.0) | -56.05 (2.0) | hit | 
| h5-3 | 8.0e+04 | -24.73 (2.1) | -11.80 (1.0) | -49.48 (2.1) | hit | 
| h5-4 | 8.0e+04 | -20.62 (2.1) | -8.01 (1.0) | -42.69 (2.1) | hit | 
| h5-5 | 8.0e+04 | -16.47 (2.1) | -4.45 (1.0) | -32.88 (2.1) | hit | 
| h5-6 | 8.0e+04 | -12.29 (2.1) | -0.24 (1.0) | -24.41 (2.1) | hit | 
| h5-7 | 8.0e+04 | -47.07 (1.9) | -16.19 (1.0) | -94.28 (1.9) | hit | 
| h5-8 | 8.0e+04 | -51.00 (1.8) | -22.25 (1.0) | -101.66 (1.8) | hit | 
| h5-9 | 8.0e+04 | -54.20 (1.8) | -29.74 (1.0) | -107.96 (1.8) | hit | 
| h5-T | 8.0e+04 | -57.42 (1.7) | -38.76 (0.9) | -107.09 (1.7) | hit | 
| h5-A | 8.0e+04 | -72.39 (1.4) | -54.71 (0.8) | -114.03 (1.4) | hit | 
| h4-2 | 8.0e+04 | -28.76 (2.0) | -12.93 (1.0) | -57.47 (2.0) | hit | 
| h4-3 | 8.0e+04 | -24.68 (2.1) | -10.48 (1.0) | -49.92 (2.1) | hit | 
| h4-4 | 8.0e+04 | -20.67 (2.1) | -7.36 (1.0) | -41.23 (2.1) | hit | 
| h4-5 | 8.0e+04 | -16.91 (2.1) | -3.43 (1.0) | -32.59 (2.1) | hit | 
| h4-6 | 8.0e+04 | -12.25 (2.1) | +0.40 (1.0) | -23.57 (2.1) | hit | 
| h4-7 | 8.0e+04 | -47.75 (1.9) | -12.72 (1.0) | -94.73 (1.9) | hit | 
| h4-8 | 8.0e+04 | -50.76 (1.8) | -19.63 (1.0) | -103.23 (1.8) | hit | 
| h4-9 | 8.0e+04 | -54.06 (1.8) | -28.13 (1.0) | -108.57 (1.8) | hit | 
| h4-T | 8.0e+04 | -56.86 (1.7) | -36.62 (0.9) | -107.18 (1.7) | hit | 
| h4-A | 8.0e+04 | -72.38 (1.4) | -53.76 (0.9) | -113.22 (1.4) | hit | 
| s20-2 | 8.0e+04 | +63.48 (2.0) | +18.05 (1.0) | +36.30 (2.0) | stand | 
| s20-3 | 8.0e+04 | +64.51 (2.0) | +19.96 (1.0) | +41.60 (2.0) | stand | 
| s20-4 | 8.0e+04 | +65.47 (2.0) | +22.51 (1.0) | +45.96 (2.0) | stand | 
| s20-5 | 8.0e+04 | +66.59 (2.0) | +26.19 (1.0) | +50.69 (2.0) | stand | 
| s20-6 | 8.0e+04 | +68.22 (2.0) | +28.20 (1.0) | +56.57 (2.0) | stand | 
| s20-7 | 8.0e+04 | +77.45 (2.0) | +26.05 (1.0) | +40.16 (2.0) | stand | 
| s20-8 | 8.0e+04 | +79.03 (2.0) | +19.32 (1.0) | +28.44 (2.0) | stand | 
| s20-9 | 8.0e+04 | +76.09 (2.0) | +11.81 (1.0) | +15.62 (2.0) | stand | 
| s20-T | 8.0e+04 | +43.83 (1.9) | -5.43 (1.0) | -8.42 (1.9) | stand | 
| s20-A | 8.0e+04 | +10.71 (1.7) | -28.46 (1.0) | -34.57 (1.7) | stand | 
| s19-2 | 8.0e+04 | +37.55 (2.0) | +12.16 (1.0) | +23.28 (2.0) | stand | 
| s19-3 | 8.0e+04 | +39.32 (2.0) | +14.45 (1.0) | +28.76 (2.0) | stand | 
| s19-4 | 8.0e+04 | +41.67 (2.0) | +17.97 (1.0) | +34.49 (2.0) | stand | 
| s19-5 | 3.2e+05 | +43.87 (1.0) | +20.25 (0.5) | +39.76 (1.0) | stand | 
| s19-6 | 5.1e+06 | +45.30 (0.2) | +23.06 (0.1) | +46.24 (0.2) | double | 
| s19-7 | 8.0e+04 | +62.00 (2.0) | +22.80 (1.0) | +32.42 (2.0) | stand | 
| s19-8 | 8.0e+04 | +59.68 (2.0) | +15.07 (1.0) | +20.09 (2.0) | stand | 
| s19-9 | 8.0e+04 | +28.83 (2.0) | +1.29 (1.0) | -7.06 (2.0) | stand | 
| s19-T | 8.0e+04 | -1.88 (1.9) | -15.44 (1.0) | -29.40 (1.9) | stand | 
| s19-A | 8.0e+04 | -17.63 (1.7) | -34.41 (0.9) | -46.94 (1.7) | stand | 
| s18-2 | 2.0e+07 | +11.01 (0) | +6.00 (0) | +11.43 (0) | double | 
| s18-3 | 3.2e+05 | +13.74 (1.0) | +8.81 (0.5) | +17.70 (1.0) | double | 
| s18-4 | 8.0e+04 | +16.38 (2.0) | +12.04 (1.0) | +22.38 (2.0) | double | 
| s18-5 | 8.0e+04 | +19.67 (2.0) | +14.12 (1.0) | +29.11 (2.0) | double | 
| s18-6 | 8.0e+04 | +22.24 (2.0) | +15.83 (1.0) | +35.89 (2.0) | double | 
| s18-7 | 8.0e+04 | +39.95 (2.0) | +17.00 (1.0) | +21.63 (2.0) | stand | 
| s18-8 | 8.0e+04 | +10.59 (2.0) | +3.79 (1.0) | -2.20 (2.0) | stand | 
| s18-9 | 8.0e+04 | -18.25 (2.0) | -10.19 (1.0) | -27.83 (2.0) | hit | 
| s18-T | 8.0e+04 | -23.79 (1.9) | -20.45 (1.0) | -38.84 (1.9) | hit | 
| s18-A | 8.0e+04 | -46.31 (1.7) | -41.64 (0.9) | -59.63 (1.7) | hit | 
| s17-2 | 2.0e+07 | -15.67 (0) | -0.59 (0) | -0.88 (0) | hit | 
| s17-3 | 3.2e+05 | -12.35 (1.0) | +2.42 (0.5) | +5.90 (1.0) | double | 
| s17-4 | 8.0e+04 | -8.13 (2.0) | +5.14 (1.0) | +11.54 (2.0) | double | 
| s17-5 | 8.0e+04 | -4.49 (2.0) | +9.23 (1.0) | +18.59 (2.0) | double | 
| s17-6 | 8.0e+04 | -0.39 (2.0) | +10.43 (1.0) | +25.67 (2.0) | double | 
| s17-7 | 8.0e+04 | -10.70 (2.0) | +5.76 (1.0) | -2.08 (2.0) | hit | 
| s17-8 | 8.0e+04 | -37.98 (2.0) | -7.55 (1.0) | -24.83 (2.0) | hit | 
| s17-9 | 8.0e+04 | -42.52 (2.0) | -14.14 (1.0) | -39.49 (2.0) | hit | 
| s17-T | 8.0e+04 | -46.49 (1.9) | -25.88 (1.0) | -50.28 (1.9) | hit | 
| s17-A | 8.0e+04 | -66.85 (1.6) | -46.56 (0.9) | -68.92 (1.6) | hit | 
| s16-2 | 8.0e+04 | -28.87 (2.1) | -1.93 (1.0) | -7.79 (2.1) | hit | 
| s16-3 | 3.2e+05 | -24.68 (1.0) | +0.27 (0.5) | -1.15 (1.0) | hit | 
| s16-4 | 3.2e+05 | -20.67 (1.0) | +3.56 (0.5) | +5.67 (1.0) | double | 
| s16-5 | 8.0e+04 | -16.41 (2.1) | +6.62 (1.0) | +13.59 (2.1) | double | 
| s16-6 | 8.0e+04 | -12.54 (2.1) | +8.47 (1.0) | +19.52 (2.1) | double | 
| s16-7 | 8.0e+04 | -47.66 (2.1) | -0.28 (1.0) | -17.54 (2.1) | hit | 
| s16-8 | 8.0e+04 | -51.11 (2.0) | -6.82 (1.0) | -31.42 (2.0) | hit | 
| s16-9 | 8.0e+04 | -54.37 (2.0) | -14.60 (1.0) | -45.25 (2.0) | hit | 
| s16-T | 8.0e+04 | -57.80 (1.9) | -26.81 (1.0) | -55.92 (1.9) | hit | 
| s16-A | 8.0e+04 | -72.59 (1.6) | -44.41 (0.9) | -71.66 (1.6) | hit | 
| s15-2 | 8.0e+04 | -28.34 (2.1) | -0.67 (1.0) | -7.00 (2.1) | hit | 
| s15-3 | 8.0e+04 | -24.95 (2.1) | +2.82 (1.0) | -0.19 (2.1) | hit | 
| s15-4 | 5.1e+06 | -20.55 (0.3) | +5.57 (0.1) | +6.16 (0.3) | double | 
| s15-5 | 8.0e+04 | -16.27 (2.1) | +8.83 (1.0) | +13.72 (2.1) | double | 
| s15-6 | 8.0e+04 | -12.97 (2.1) | +9.96 (1.0) | +20.24 (2.1) | double | 
| s15-7 | 8.0e+04 | -47.65 (2.1) | +3.09 (1.0) | -18.41 (2.1) | hit | 
| s15-8 | 8.0e+04 | -51.20 (2.0) | -2.86 (1.0) | -31.79 (2.0) | hit | 
| s15-9 | 8.0e+04 | -55.02 (2.0) | -11.42 (1.0) | -46.42 (2.0) | hit | 
| s15-T | 8.0e+04 | -57.45 (1.9) | -23.91 (1.0) | -54.95 (1.9) | hit | 
| s15-A | 8.0e+04 | -72.00 (1.6) | -42.42 (0.9) | -72.09 (1.6) | hit | 
| s14-2 | 8.0e+04 | -28.06 (2.1) | +1.78 (1.0) | -6.21 (2.1) | hit | 
| s14-3 | 8.0e+04 | -24.41 (2.1) | +4.42 (1.0) | -0.62 (2.1) | hit | 
| s14-4 | 8.0e+04 | -20.41 (2.1) | +8.31 (1.0) | +5.82 (2.1) | hit | 
| s14-5 | 3.2e+05 | -16.31 (1.0) | +10.82 (0.5) | +12.34 (1.0) | double | 
| s14-6 | 8.0e+04 | -12.35 (2.1) | +11.90 (1.0) | +19.57 (2.1) | double | 
| s14-7 | 8.0e+04 | -47.52 (2.0) | +7.95 (1.0) | -18.64 (2.0) | hit | 
| s14-8 | 8.0e+04 | -51.41 (2.0) | +2.01 (1.0) | -32.23 (2.0) | hit | 
| s14-9 | 8.0e+04 | -54.26 (2.0) | -7.57 (1.0) | -45.45 (2.0) | hit | 
| s14-T | 8.0e+04 | -57.20 (1.9) | -20.14 (1.0) | -56.20 (1.9) | hit | 
| s14-A | 8.0e+04 | -72.41 (1.6) | -39.75 (0.9) | -70.94 (1.6) | hit | 
| s13-2 | 8.0e+04 | -28.79 (2.1) | +4.29 (1.0) | -7.07 (2.1) | hit | 
| s13-3 | 8.0e+04 | -24.39 (2.1) | +7.00 (1.0) | -0.33 (2.1) | hit | 
| s13-4 | 8.0e+04 | -20.39 (2.1) | +10.23 (1.0) | +5.20 (2.1) | hit | 
| s13-5 | 2.0e+07 | -16.46 (0) | +12.88 (0) | +12.73 (0) | hit | 
| s13-6 | 8.0e+04 | -12.67 (2.1) | +13.75 (1.0) | +18.93 (2.1) | double | 
| s13-7 | 8.0e+04 | -47.64 (2.1) | +12.41 (1.0) | -18.59 (2.1) | hit | 
| s13-8 | 8.0e+04 | -51.47 (2.0) | +5.50 (1.0) | -32.61 (2.0) | hit | 
| s13-9 | 8.0e+04 | -54.33 (2.0) | -3.30 (1.0) | -45.89 (2.0) | hit | 
| s13-T | 8.0e+04 | -57.17 (1.9) | -17.29 (1.0) | -55.28 (1.9) | hit | 
| s13-A | 8.0e+04 | -71.95 (1.6) | -37.21 (0.9) | -72.33 (1.6) | hit | 
| s12-2 | 8.0e+04 | -28.81 (2.1) | +7.24 (1.0) | -5.99 (2.1) | hit | 
| s12-3 | 8.0e+04 | -24.61 (2.1) | +9.89 (1.0) | -0.34 (2.1) | hit | 
| s12-4 | 8.0e+04 | -20.33 (2.1) | +12.28 (1.0) | +6.36 (2.1) | hit | 
| s12-5 | 8.0e+04 | -16.22 (2.1) | +15.13 (1.0) | +12.78 (2.1) | hit | 
| s12-6 | 8.0e+04 | -12.16 (2.1) | +15.30 (1.0) | +21.02 (2.1) | double | 
| s12-7 | 8.0e+04 | -46.97 (2.1) | +16.64 (1.0) | -18.22 (2.1) | hit | 
| s12-8 | 8.0e+04 | -50.98 (2.0) | +9.38 (1.0) | -31.23 (2.0) | hit | 
| s12-9 | 8.0e+04 | -54.01 (2.0) | +0.31 (1.0) | -45.30 (2.0) | hit | 
| s12-T | 8.0e+04 | -58.03 (1.9) | -14.72 (1.0) | -56.96 (1.9) | hit | 
| s12-A | 8.0e+04 | -72.15 (1.6) | -35.17 (0.9) | -72.39 (1.6) | hit | 


|  Hand  |  Hands needed  |   Yes [%]  |   No [%]   |
|:------:|:-------:|:----------:|:----------:|
| pA-2 | 8.0e+04 | +47.79 (1.7) | +7.70 (1.0) | yes | 
| pA-3 | 8.0e+04 | +51.89 (1.7) | +9.80 (1.0) | yes | 
| pA-4 | 8.0e+04 | +56.73 (1.7) | +11.81 (1.0) | yes | 
| pA-5 | 8.0e+04 | +61.10 (1.7) | +15.76 (1.0) | yes | 
| pA-6 | 8.0e+04 | +66.09 (1.7) | +20.40 (2.1) | yes | 
| pA-7 | 8.0e+04 | +46.61 (1.6) | +15.82 (1.0) | yes | 
| pA-8 | 8.0e+04 | +35.34 (1.6) | +9.87 (1.0) | yes | 
| pA-9 | 8.0e+04 | +23.05 (1.6) | -0.13 (1.0) | yes | 
| pA-T | 8.0e+04 | +8.79 (1.6) | -14.01 (1.0) | yes | 
| pA-A | 8.0e+04 | -24.16 (1.5) | -35.35 (0.9) | yes | 
| pT-2 | 8.0e+04 | -4.29 (3.7) | +63.29 (0.7) | no | 
| pT-3 | 8.0e+04 | +4.76 (3.8) | +64.28 (0.7) | no | 
| pT-4 | 8.0e+04 | +15.42 (4.0) | +65.64 (0.7) | no | 
| pT-5 | 8.0e+04 | +27.51 (4.1) | +66.77 (0.7) | no | 
| pT-6 | 8.0e+04 | +37.88 (4.1) | +67.89 (0.7) | no | 
| pT-7 | 8.0e+04 | +8.98 (2.6) | +77.52 (0.6) | no | 
| pT-8 | 8.0e+04 | -22.83 (2.5) | +79.04 (0.6) | no | 
| pT-9 | 8.0e+04 | -61.33 (2.6) | +76.23 (0.6) | no | 
| pT-T | 8.0e+04 | -76.47 (2.7) | +43.58 (0.7) | no | 
| pT-A | 8.0e+04 | -85.74 (2.2) | +11.11 (1.0) | no | 
| p9-2 | 8.0e+04 | +20.25 (2.2) | +10.96 (1.0) | yes | 
| p9-3 | 8.0e+04 | +25.62 (2.2) | +13.70 (1.0) | yes | 
| p9-4 | 8.0e+04 | +32.53 (2.2) | +16.16 (1.0) | yes | 
| p9-5 | 8.0e+04 | +39.22 (2.2) | +19.08 (1.0) | yes | 
| p9-6 | 8.0e+04 | +47.19 (2.2) | +22.75 (1.0) | yes | 
| p9-7 | 3.2e+05 | +36.59 (0.9) | +40.03 (0.4) | no | 
| p9-8 | 8.0e+04 | +23.50 (1.9) | +10.05 (0.8) | yes | 
| p9-9 | 8.0e+04 | -8.46 (1.8) | -18.37 (1.0) | yes | 
| p9-T | 8.0e+04 | -36.82 (1.9) | -23.97 (1.0) | no | 
| p9-A | 3.2e+05 | -48.36 (0.8) | -46.75 (0.4) | no | 
| p8-2 | 8.0e+04 | +7.89 (2.4) | -29.06 (1.0) | yes | 
| p8-3 | 8.0e+04 | +14.84 (2.4) | -23.82 (1.0) | yes | 
| p8-4 | 8.0e+04 | +21.02 (2.5) | -20.67 (1.0) | yes | 
| p8-5 | 8.0e+04 | +29.77 (2.5) | -16.36 (1.0) | yes | 
| p8-6 | 8.0e+04 | +39.44 (2.7) | -11.93 (1.1) | yes | 
| p8-7 | 8.0e+04 | +31.80 (2.1) | -41.83 (0.9) | yes | 
| p8-8 | 8.0e+04 | -2.54 (2.1) | -45.57 (0.9) | yes | 
| p8-9 | 8.0e+04 | -39.27 (2.2) | -50.67 (0.9) | yes | 
| p8-T | 8.0e+04 | -51.15 (1.9) | -57.31 (0.8) | yes | 
| p8-A | 3.2e+05 | -66.55 (0.8) | -68.39 (0.4) | yes | 
| p7-2 | 8.0e+04 | -12.93 (2.6) | -28.38 (1.0) | yes | 
| p7-3 | 8.0e+04 | -5.76 (2.8) | -24.45 (1.0) | yes | 
| p7-4 | 8.0e+04 | +7.32 (2.9) | -21.06 (1.0) | yes | 
| p7-5 | 8.0e+04 | +16.33 (2.9) | -16.50 (1.0) | yes | 
| p7-6 | 8.0e+04 | +24.66 (2.9) | -12.31 (1.1) | yes | 
| p7-7 | 8.0e+04 | -4.49 (2.1) | -31.70 (1.0) | yes | 
| p7-8 | 3.2e+05 | -39.47 (1.0) | -36.94 (0.5) | no | 
| p7-9 | 8.0e+04 | -57.27 (2.1) | -43.25 (0.9) | no | 
| p7-T | 8.0e+04 | -68.16 (1.8) | -51.02 (0.9) | no | 
| p7-A | 8.0e+04 | -81.66 (1.6) | -62.96 (0.8) | no | 
| p6-2 | 8.0e+04 | -20.02 (2.5) | -24.94 (1.0) | yes | 
| p6-3 | 8.0e+04 | -13.43 (2.9) | -22.86 (1.0) | yes | 
| p6-4 | 8.0e+04 | -1.32 (2.9) | -20.97 (1.0) | yes | 
| p6-5 | 8.0e+04 | +8.77 (2.9) | -16.47 (1.0) | yes | 
| p6-6 | 8.0e+04 | +18.02 (2.9) | -11.89 (1.1) | yes | 
| p6-7 | 8.0e+04 | -25.77 (2.0) | -21.03 (1.0) | no | 
| p6-8 | 8.0e+04 | -42.37 (2.0) | -26.97 (1.0) | no | 
| p6-9 | 8.0e+04 | -61.32 (1.9) | -33.82 (1.0) | no | 
| p6-T | 8.0e+04 | -73.60 (1.7) | -42.98 (0.9) | no | 
| p6-A | 8.0e+04 | -81.35 (1.5) | -57.46 (0.8) | no | 
| p5-2 | 8.0e+04 | -29.29 (2.3) | +35.74 (2.0) | no | 
| p5-3 | 8.0e+04 | -21.10 (2.5) | +40.83 (2.0) | no | 
| p5-4 | 8.0e+04 | -14.02 (2.8) | +46.13 (2.0) | no | 
| p5-5 | 8.0e+04 | -2.07 (2.8) | +51.38 (2.0) | no | 
| p5-6 | 8.0e+04 | +7.69 (2.8) | +55.49 (2.0) | no | 
| p5-7 | 8.0e+04 | -29.78 (1.8) | +38.74 (2.0) | no | 
| p5-8 | 8.0e+04 | -45.59 (1.8) | +29.74 (2.0) | no | 
| p5-9 | 8.0e+04 | -63.38 (1.7) | +15.31 (2.0) | no | 
| p5-T | 8.0e+04 | -75.56 (1.7) | -5.45 (1.0) | no | 
| p5-A | 8.0e+04 | -83.48 (1.4) | -28.71 (1.0) | no | 
| p4-2 | 8.0e+04 | -18.34 (2.4) | -2.08 (1.0) | no | 
| p4-3 | 8.0e+04 | -10.15 (2.7) | -0.10 (1.0) | no | 
| p4-4 | 8.0e+04 | -1.53 (2.9) | +4.20 (1.0) | no | 
| p4-5 | 1.3e+06 | +8.32 (0.7) | +6.94 (0.3) | yes | 
| p4-6 | 8.0e+04 | +18.54 (2.9) | +9.10 (1.0) | yes | 
| p4-7 | 8.0e+04 | -17.13 (2.0) | +8.29 (1.0) | no | 
| p4-8 | 8.0e+04 | -32.41 (2.0) | -5.80 (1.0) | no | 
| p4-9 | 8.0e+04 | -52.03 (2.0) | -21.40 (1.0) | no | 
| p4-T | 8.0e+04 | -65.35 (1.7) | -30.80 (1.0) | no | 
| p4-A | 8.0e+04 | -75.64 (1.5) | -48.85 (0.9) | no | 
| p3-2 | 2.0e+07 | -13.55 (0) | -13.84 (0) | yes | 
| p3-3 | 3.2e+05 | -4.62 (1.3) | -10.55 (0.5) | yes | 
| p3-4 | 8.0e+04 | +4.29 (2.7) | -7.50 (1.0) | yes | 
| p3-5 | 8.0e+04 | +12.98 (2.9) | -2.79 (1.0) | yes | 
| p3-6 | 8.0e+04 | +22.13 (2.9) | +0.27 (1.0) | yes | 
| p3-7 | 8.0e+04 | -5.64 (2.0) | -14.75 (1.0) | yes | 
| p3-8 | 3.2e+05 | -23.16 (1.0) | -21.60 (0.5) | no | 
| p3-9 | 8.0e+04 | -43.38 (2.0) | -29.61 (1.0) | no | 
| p3-T | 8.0e+04 | -59.02 (1.7) | -38.86 (0.9) | no | 
| p3-A | 8.0e+04 | -70.21 (1.5) | -54.10 (0.9) | no | 
| p2-2 | 8.0e+04 | -7.83 (2.3) | -11.55 (1.0) | yes | 
| p2-3 | 8.0e+04 | -1.63 (2.5) | -8.20 (1.0) | yes | 
| p2-4 | 8.0e+04 | +5.17 (2.7) | -4.49 (1.0) | yes | 
| p2-5 | 8.0e+04 | +16.51 (2.7) | -1.33 (1.0) | yes | 
| p2-6 | 8.0e+04 | +25.65 (2.9) | +2.57 (1.0) | yes | 
| p2-7 | 8.0e+04 | +0.70 (2.0) | -8.63 (1.0) | yes | 
| p2-8 | 1.3e+06 | -17.84 (0.5) | -15.96 (0.2) | no | 
| p2-9 | 8.0e+04 | -40.15 (2.0) | -23.95 (1.0) | no | 
| p2-T | 8.0e+04 | -53.36 (1.8) | -34.25 (0.9) | no | 
| p2-A | 8.0e+04 | -67.42 (1.5) | -51.15 (0.9) | no | 



### Detailed explanation

We want to derive the basic strategy from scratch, i.e. without making any assumptions. What we are going to do is to play a large (more on what _large_ means below) number of hands by fixing our first two cards and the dealer upcard and sequentially standing, doubling or hitting the first card. Then we will compare the results for the three cases and select as the proper strategy the best one of the three possible choices.

Standing and doubling are easy plays, because after we stand or double down then the dealer plays accordingly to the rules: she hits until seventeen, possibly hitting soft seventeen.  But if we hit on our hand, we might need to make another decision whether to stand or hit again. As we do not want to assume anything, we have to play in such an order that if we do need to make another decision, we already know which is the best one. 

#### Hard hands

So we start by arranging the shoe so that the user gets hard twenty (i.e. two faces) and the dealer gets successively upcards of two to ace. So we play each combination of dealer upcard (ten) three times each playing either

 1. always standing
 2. always doubling
 3. always hitting
 
In general the first two plays are easy, because the game stops either after standing or after receiving only one card. The last one might lead to further hitting, but since we are starting with a hard twenty, that would either give the player twenty one or a bust. In any case, the game also ends.
So we play a certain number of hands (say one thousand hands) each of these three plays for each of the ten upcard faces and record the outcome. The correct play for hard twenty against each of the ten upcards is the play that gave the better result, which is of course standing.

Next, we do the same for a hard nineteen. In this case, the hitting play might not end after one card is drawn (i.e. we hit on nineteen and get and ace). But if that was the case, we would already know what the best play is from the previous step so we play accordingly and we stand. Repeating this procedure down to hard four we can build the basic strategy table for any hard total against any dealer upcard.

#### Soft hands

We can now switch to analyze soft hands. Starting from soft twenty (i.e. an ace and a nine) we do the same we did for the hard case. The only difference is that when hitting, we might end either in another soft hand which we would already analyzed because we start from twenty and go down, or in a hard hand, which we also already analyzed so we can play accordingly.

#### Pairs

When dealing with pairs, we have to decide whether to split or not. When we do not split, we end up in one of the already-analyzed cases: either a soft twelve of any even hard hand. When we split, we might end in a hard or soft hand (already analyzed) or in a new pair. But since the new pair can be only the same pair we are analyzing, we have to treat it like we treated the first pair: either to split it or not, so we know how to deal with it.  

#### Number of hands

The output is the expected value\ $e$ of the bankroll, which is a random variable with an associated uncertainty\ $\Delta e$ (i.e. a certain numbers of standard deviations). For example, if we received only blackjacks, the expected value would be 1.5 (provided blackjacks pay\ 3 to\ 2 of course). If we busted all of our hands without doubling or splitting, the expected value would be -1. In order to say that the best strategy is, let’s say stand and not hitting or doubling down, we have to make sure that $e_h-\Delta e_h > e_s+\Delta e_s$ and $e_h-\Delta e_h > e_d+\Delta e_d$. If there is no play that can give a better expected value than the other two taking into account the uncertainties, then we have to play more hands in order to reduce the random uncertainty.


### Implementation

The steps above can be written in a [Bash](https://en.wikipedia.org/wiki/Bash_%28Unix_shell%29) script that

 * loops over hands and upcards,
 * creates a strategy file for each possible play hit, double or stand (or split or not),
 * runs [Libre Blackjack](https://www.seamplex.com/blackjack),
 * checks the results and picks the best play,
 * updates the strategy file

```bash
##!/bin/bash

   n0=80000
n_max=9000000

RED="\033[0;31m"
GREEN="\033[0;32m"

BROWN="\033[0;33m"
MAGENTA="\e[0;35m"
CYAN="\e[0;36m"

NC="\033[0m" ## No Color

for i in grep awk printf blackjack; do
 if [ -z "$(which $i)" ]; then
  echo "error: $i not installed"
  exit 1
 fi
done

debug=0

declare -A strategy
declare -A ev

declare -A min
min["hard"]=4   ## from 20 to 4 in hards
min["soft"]=12  ## from 20 to 12 in softs

rm -f table.md hard.html soft.html pair.html

## --------------------------------------------------------------
## start with standing
cp hard-stand.txt hard.txt
cp soft-stand.txt soft.txt

cat << EOF >> table.md
|  Hand  |  \$n\$  |  Stand [%]  |  Double [%]  |  Hit [%] |   Play    |
|:------:|:-----:|:-----------:|:------------:|:--------:|:---------:|
EOF


for type in hard soft; do
 for hand in $(seq 20 -1 ${min[${type}]}); do
 
  ## choose two random cards that make up the player's assumed total
  if [ ${type} = "hard" ]; then
   t="h"
   card1=11
   card2=11
   while test $card1 -gt 10 -o $card2 -gt 10; do
    card1=$((${RANDOM} % (${hand}-3) + 2))
    card2=$((${hand} - ${card1}))
   done
  elif [ ${type} = "soft" ]; then
   t="s"
   ## one card is an ace
   card1=1
   card2=$((${hand} - 10 - ${card1}))
  fi

  cat << EOF >> ${type}.html
 <tr>
  <td>${t}${hand}</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
EOF
  
  for upcard in $(seq 2 9) T A; do
  
   if [ "x$upcard" = "xT" ]; then
     upcard_n=10
   elif [ "x$upcard" = "xA" ]; then
     upcard_n=1
   else
     upcard_n=$(($upcard))
   fi
 
   n=${n0}   ## start with n0 hands
   best="x"  ## x means don't know what to so, so play
   
   while [ "${best}" = "x" ]; do
    ## tell the user which combination we are trying and how many we will play
    echo -ne "${t}${hand}-${upcard} ($card1 $card2)\t"$(printf %.1e ${n})
    for play in s d h; do
     
     ## start with options.conf as a template and add some custom stuff
     cp options.conf blackjack.conf
     cat << EOF >> blackjack.conf
hands = ${n}
player = internal
arranged_cards = ${card1}, $((${upcard_n} + 13)), $((${card2} + 26))
report = ${t}${hand}-${upcard}-${play}.yaml
##log = ${t}${hand}-${upcard}-${play}.log
EOF
 
     ## read the current strategy
     while read w p2 p3 p4 p5 p6 p7 p8 p9 pT pA; do
      ## w already has the "h" or the "s"
      strategy[${w},2]=$p2   
      strategy[${w},3]=$p3
      strategy[${w},4]=$p4    
      strategy[${w},5]=$p5    
      strategy[${w},6]=$p6    
      strategy[${w},7]=$p7    
      strategy[${w},8]=$p8    
      strategy[${w},9]=$p9    
      strategy[${w},T]=$pT    
      strategy[${w},A]=$pA    
     done < ${type}.txt
     
     ## override the read strategy with the explicit play: s, d or h
     strategy[${t}${hand},${upcard}]=${play}
     
     ## save the new (temporary) strategy
     rm -f ${type}.txt
     for h in $(seq 20 -1 ${min[${type}]}); do
      echo -n "${t}${h}  " >> ${type}.txt
      
      ## extra space if h < 10
      if [ ${h} -lt 10 ]; then
       echo -n " " >> ${type}.txt
      fi 
      
      for u in $(seq 2 9) T A; do
       echo -n "${strategy[${t}${h},${u}]}  " >> ${type}.txt
      done
      echo >> ${type}.txt
     done

     ## debug, comment for production
     if [ "${debug}" != "0" ]; then
      cp ${type}.txt ${t}${hand}-${upcard}-${play}.str
     fi
    
     ## ensamble the full bs.txt with no pairing
     cat hard.txt soft.txt pair-no.txt > bs.txt
     
     ## play!
     blackjack
    
     ## evaluate the results
     ev[${t}${hand},${upcard},${play}]=$(grep mean ${t}${hand}-${upcard}-${play}.yaml | awk '{printf("%g", $2)}')
     error[${t}${hand},${upcard},${play}]=$(grep error ${t}${hand}-${upcard}-${play}.yaml | awk '{printf("%g", $2)}')
     
    done
   
    ## choose the best one
    ev_s=$(echo ${ev[${t}${hand},${upcard},s]} | awk '{printf("%+.2f", 100*$1)}')
    ev_d=$(echo ${ev[${t}${hand},${upcard},d]} | awk '{printf("%+.2f", 100*$1)}')
    ev_h=$(echo ${ev[${t}${hand},${upcard},h]} | awk '{printf("%+.2f", 100*$1)}')
   
    
    if [ ${n} -le ${n_max} ]; then 
     ## if we still have room, take into account errors
     error_s=$(echo ${error[${t}${hand},${upcard},s]} | awk '{printf("%.1f", 100*$1)}')
     error_d=$(echo ${error[${t}${hand},${upcard},d]} | awk '{printf("%.1f", 100*$1)}')
     error_h=$(echo ${error[${t}${hand},${upcard},h]} | awk '{printf("%.1f", 100*$1)}')
    else
     ## instead of running infinite hands, above a threshold asume errors are zero
     error_s=0
     error_d=0
     error_h=0
    fi  
 
    echo -ne "\t${ev_s}\t(${error_s})"
    echo -ne "\t${ev_d}\t(${error_d})"
    echo -ne "\t${ev_h}\t(${error_h})"
   
    if   (( $(echo ${ev_s} ${error_s} ${ev_d} ${error_d} | awk '{print (($1-$2) > ($3+$4))}') )) &&
         (( $(echo ${ev_s} ${error_s} ${ev_h} ${error_h} | awk '{print (($1-$2) > ($3+$4))}') )); then
         
     best="s"
     color=${BROWN}
     best_string="stand"
     
    elif (( $(echo ${ev_d} ${error_d} ${ev_s} ${error_s} | awk '{print (($1-$2) > ($3+$4))}') )) &&
         (( $(echo ${ev_d} ${error_d} ${ev_h} ${error_h} | awk '{print (($1-$2) > ($3+$4))}') )); then
         
     best="d"
     color=${CYAN}
     best_string="double"
     
    elif (( $(echo ${ev_h}-${error_h} ${ev_s} ${error_s} | awk '{print (($1-$2) > ($3+$4))}') )) &&
         (( $(echo ${ev_h}-${error_h} ${ev_d} ${error_d} | awk '{print (($1-$2) > ($3+$4))}') )); then
         
     best="h"
     color=${MAGENTA}
     best_string="hit"
         
    else
    
     best="x"
     color=${NC}
     best_string="uncertain"
     
     n=$((${n} * 4))
     
    fi
    
    echo -e ${color}"\t"${best_string}${NC}
    
   done

   strategy[${t}${hand},${upcard}]=${best}
   
   
   
   echo "| ${t}${hand}-${upcard} | $(printf %.1e ${n}) | ${ev_s} (${error_s}) | ${ev_h} (${error_h}) | ${ev_d} (${error_d}) | ${best_string} | " >> table.md
    
   echo " <!-- ${upcard} -->" >> ${type}.html
   echo " <td>" >> ${type}.html
   echo ${ev_s} ${error_s} | awk -f html_cell.awk >> ${type}.html
   echo ${ev_h} ${error_h} | awk -f html_cell.awk >> ${type}.html
   echo ${ev_d} ${error_d} | awk -f html_cell.awk >> ${type}.html
   echo " </td>" >> ${type}.html
   
   
   ## save the strategy again with the best strategy
   rm -f ${type}.txt
   for h in $(seq 20 -1 ${min[${type}]}); do
    echo -n "${t}${h}  " >> ${type}.txt
    
    ## extra space if h < 10
    if [ ${h} -lt 10 ]; then
     echo -n " " >> ${type}.txt
    fi 
    
    for u in $(seq 2 9) T A; do
     echo -n "${strategy[${t}${h},${u}]}  " >> ${type}.txt
    done
    
    echo >> ${type}.txt
    
   done
  done
  
##   echo "</tr>" >> ${type}.html
  
 done
done


cat << EOF >> table.md


|  Hand  |  \$n\$  |   Yes [%]  |   No [%]   |
|:------:|:-------:|:----------:|:----------:|
EOF

## --------------------------------------------------------------------
## pairs
type="pair"
t="p"
cp pair-no.txt pair.txt

for hand in A T $(seq 9 -1 2); do
 if [ "${hand}" = "A" ]; then
  pair=1
 elif [ "${hand}" = "T" ]; then
  pair=10
 else
  pair=$((${hand}))
 fi
  
##  cat << EOF >> ${type}.html
##  <tr>
##   <td>${t}${hand}</td>
##   <td>
##    <div class="text-right">y<span class="d-none d-lg-inline">es</span></div>
##    <div class="text-right">n<span class="d-none d-lg-inline">o</span></div>
##   </td>
## EOF
  
 for upcard in $(seq 2 9) T A; do
  if [ "$upcard" = "T" ]; then
    upcard_n=10
  elif [ "$upcard" = "A" ]; then
    upcard_n=1
  else
    upcard_n=$(($upcard))
  fi
 
  n=${n0}   ## start with n0 hands
  best="x"  ## x means don't know what to so, so play
   
  while [ "${best}" = "x" ]; do
   ## tell the user which combination we are trying and how many we will play
   echo -ne "${t}${hand}-${upcard}\t\t$(printf %.0e ${n})"
   
   for play in y n; do
    
    ## start with options.conf as a template and add some custom stuff
    cp options.conf blackjack.conf
    cat << EOF >> blackjack.conf
hands = ${n}
player = internal
arranged_cards = ${pair}, $((${upcard_n} + 13)), $((${pair} + 26))
report = ${t}${hand}-${upcard}-${play}.yaml
## log = ${t}${hand}-${upcard}-${play}.log
EOF
 
    ## read the current strategy
    while read w p2 p3 p4 p5 p6 p7 p8 p9 pT pA; do
     ## w already has the "p"
     strategy[${w},2]=$p2   
     strategy[${w},3]=$p3
     strategy[${w},4]=$p4    
     strategy[${w},5]=$p5    
     strategy[${w},6]=$p6    
     strategy[${w},7]=$p7    
     strategy[${w},8]=$p8    
     strategy[${w},9]=$p9    
     strategy[${w},T]=$pT    
     strategy[${w},A]=$pA    
    done < ${type}.txt
     
    ## override the read strategy with the explicit play: y or n
    strategy[${t}${hand},${upcard}]=${play}
     
    ## save the new (temporary) strategy
    rm -f ${type}.txt
    for h in A T $(seq 9 -1 2); do
     echo -n "${t}${h}   " >> ${type}.txt
     for u in $(seq 2 9) T A; do
      echo -n "${strategy[${t}${h},${u}]}  " >> ${type}.txt
     done
     echo >> ${type}.txt
    done
     
    if [ "${debug}" != "0" ]; then
     cp ${type}.txt ${t}${hand}-${upcard}-${play}.str
    fi  
    
    ## ensamble the full bs.txt
    cat hard.txt soft.txt pair.txt > bs.txt

    ## play!
    blackjack

    ## evaluate the results
    ev[${t}${hand},${upcard},${play}]=$(grep mean ${t}${hand}-${upcard}-${play}.yaml | awk '{printf("%g", $2)}')
    error[${t}${hand},${upcard},${play}]=$(grep error ${t}${hand}-${upcard}-${play}.yaml | awk '{printf("%g", $2)}')
    
   done
   
   ## choose the best one
   ev_y=$(echo ${ev[${t}${hand},${upcard},y]} | awk '{printf("%+.2f", 100*$1)}')
   ev_n=$(echo ${ev[${t}${hand},${upcard},n]} | awk '{printf("%+.2f", 100*$1)}')
   
   if [ $n -le ${n_max} ]; then 
    ## if we still have room, take into account errors
    error_y=$(echo ${error[${t}${hand},${upcard},y]} | awk '{printf("%.1f", 100*$1)}')
    error_n=$(echo ${error[${t}${hand},${upcard},n]} | awk '{printf("%.1f", 100*$1)}')
   else
    ## instead of running infinite hands, above a threshold asume errors are zero
    error_y=0
    error_n=0
   fi  
 
   echo -ne "\t${ev_y}\t(${error_y})"
   echo -ne "\t${ev_n}\t(${error_n})"
   
   if   (( $(echo ${ev_y} ${error_y} ${ev_n} ${error_n} | awk '{print (($1-$2) > ($3+$4))}') )); then
   
    best="y"
    color=${GREEN}
    best_string="yes"
    
   elif (( $(echo ${ev_n} ${error_n} ${ev_y} ${error_y} | awk '{print (($1-$2) > ($3+$4))}') )); then
   
    best="n"
    color=${RED}
    best_string="no"
   
   else
   
    best="x"
    color=${NC}
    best_string="uncertain"
    
    n=$((${n} * 4))
    
   fi
   
   echo -e ${color}"\t"${best_string}${NC}
  done

  echo "| ${t}${hand}-${upcard} | $(printf %.1e ${n}) | ${ev_y} (${error_y}) | ${ev_n} (${error_n}) | ${best_string} | " >> table.md
  
  echo " <!-- ${upcard} -->" >> ${type}.html
  echo " <td>" >> ${type}.html
  echo ${ev_y} ${error_y} | awk -f html_cell.awk >> ${type}.html
  echo ${ev_n} ${error_n} | awk -f html_cell.awk >> ${type}.html
  echo " </td>" >> ${type}.html
  
  
  strategy[${t}${hand},${upcard}]=${best}
   
  ## save the strategy again with the best strategy
  rm -f ${type}.txt
  for h in A T $(seq 9 -1 2); do
   echo -n "${t}${h}   " >> ${type}.txt
   for u in $(seq 2 9) T A; do
    echo -n "${strategy[${t}${h},${u}]}  " >> ${type}.txt
   done
   echo >> ${type}.txt
  done
 done
done

 
cat header.txt hard.txt header.txt soft.txt header.txt pair.txt > bs.txt
rm -f hard.txt soft.txt pair.txt blackjack.conf
if [ "${debug}" == "0" ]; then
 rm -f *.yaml
 rm -f *.str
 rm -f *.log
fi
 

```





## Wizard’s ace-five strategy


So far the house always had the edge. it is now the turn for the player to be (a little bit) up. This Python player uses the Wizard of Odds’s  simple card-counting strategy called the Ace/Five count:^[<https://wizardofodds.com/games/blackjack/ace-five-count/>]

 1. Establish what your minimum and maximum bets will be. Usually the maximum will be 8, 16, or 32 times the minimum bet, or any power of 2, but you can use whatever bet spread you wish.
 2. At the beginning of each shoe, start with your minimum bet, and a count of zero.
 3. For each five observed, add one to the count.
 4. For each ace observed, subtract one from the count.
 5. If the count is greater than or equal to two, then double your last bet, up to your maximum bet.
 6. If the count is less than or equal to one, then make the minimum bet.
 7. Use basic strategy for all playing decisions.


Instead of using basic strategy, the player uses the Wizard’s simple strategy which is far easier to memorize (and to code):^[https://wizardofodds.com/games/blackjack/basics/##wizards-simple-strategy]
 
 * Always

   1. Hit hard 8 or less.
   2. Stand on hard 17 or more.
   3. Hit on soft 15 or less.
   4. Stand on soft 19 or more.
   5. With 10 or 11, double if you have more than the dealer’s up card (treating a dealer ace as 11 points), otherwise hit.
   6. Surrender 16 against 10.
   7. Split eights and aces. 

 * If the player hand does not fit one of the above "always" rules, and the dealer has a 2 to 6 up, then play as follows:

   1. Double on 9.
   2. Stand on hard 12 to 16.
   3. Double soft 16 to 18.
   4. Split 2’s, 3’s, 6’s, 7’s, and 9’s. 

 * If the player hand does not fit one of the above "always" rules, and the dealer has a 7 to A up, then hit. 

 
![The Wizard of Odds’ simple strategy](wizard_strategy.png)


```
##!/usr/bin/python3

## plays the wizard's ace-five count
## <http://wizardofodds.com/games/blackjack/appendix/17/>
## with the simple strategy at
## <http://wizardofodds.com/games/blackjack/appendix/21/>
##
##
##  Player's hand     Dealer's upcard
## 
##  -- hard --------------------------------
##                    2 to 6     7 to A
##  4 to 8              H          H   
##  9                   D          H   
##  10 or 11        D with more than dealer
##  12 to 16            S          H
##  17 to 21            S          S
##
##  -- soft --------------------------------
##                    2 to 6     7 to A
##  13 to 15            H          H   
##  16 to 18            D          H   
##  19 to 21            S          S
##
##  -- split -------------------------------
##                    2 to 6     7 to A
##  22,33,66,77,99      Y          N   
##  88,AA               Y          Y   
##  44,55,TT            N          N
##
##  Plus:
##    1. surrender 16 vs 10
##    2. never take insurance
##    3. if not allowed to double, stand with soft 18
##

import sys 
import fileinput

max_bet = 8
debug = False

n_player_cards = 0
count = 0
bet = 1


for linenl in fileinput.input():
  line = linenl.rstrip()
  if debug:
    print("<- %s" % line, file = sys.stderr) 
  
  if line == "bye":
    sys.exit(0)
    
  elif line == "shuffling":
    count = 0
    bet = 1
        
  elif line[:8] == "new_hand": 
    n_player_cards = 0

  elif line == "insurance?": 
    print("no", flush = True)
    if debug:
      print("<- no", file = sys.stderr) 
        
  elif line == "bet?":
    if count <= 1:
      bet = 1
    elif bet < max_bet:
      bet *= 2
    print(bet, flush = True)
    ##print("1", flush = True)
    
  elif line[:15] == "player_split_ok":    
    n_player_cards = 1

  elif line[:5] == "card_":
    tokens = line.split()
    if (6 > 1):
      card = tokens[1][0]
    else:
      card = ""   ## the dealer's hole card
      
    ## count aces and fives
    if card == "A":
      count -= 1
      if debug:
        print("ACE, count is %d" % count, file = sys.stderr) 
    elif card == "5":
      count += 1
      if debug:
        print("FIVE, count is %d" % count, file = sys.stderr) 

    if line[:11] == "card_player":
      n_player_cards += 1
      if n_player_cards == 1:
        card_player_first = card
      elif n_player_cards == 2:
        card_player_second = card
                      
  elif line[:5] == "play?":
    tokens = line.split()
    player = int(tokens[1])
    dealer = abs(int(tokens[2]))
    action = "quit"
    
    if n_player_cards == 2 and card_player_first == card_player_second and \
        ((card_player_first == "8" or card_player_first == "A") or \
         (dealer < 7 and \
            (card_player_first == "2" or \
             card_player_first == "3" or \
             card_player_first == "6" or \
             card_player_first == "7" or \
             card_player_first == "9"))):  ## --- split------------------------------------
      action = "split"                     ## always split aces and 8s but 2,3,6,7 & 9 only against 6 or less
      
    else:
      if player > 0:                       ## --- hard ------------------------------------
        if player < 9:         
          action = "hit"                   ## hit 4 to 8 against anything
        elif player == 9:
          if dealer < 7: 
            if n_player_cards == 2:
              action = "double"            ## double 9 against 2 to 6
            else:
              action = "hit"               ## else hit
          else:
            action = "hit"                 ## hit 9 against 7 to A
        elif player < 12:
          if player > dealer:
            if n_player_cards == 2:
              action = "double"            ## double with 10 or 11 and more than dealer
            else:
              action = "hit"
          else:
            action = "hit"                 ## otherwise hit
        elif player < 17:
          if dealer < 7:
            action = "stand"               ## stand with 12 to 16 against 2 to 6
          else:
            action = "hit"                 ## hit with 12 to 16 against 7 to A
        else:
          action = "stand"                 ## stand with hard 17 or more
      else:
        ## soft
        player = abs(player)
        if player < 16:        
          action = "hit"                   ## hit every soft hand less than 16
        elif player < 19:
          if dealer < 7:
            if n_player_cards == 2:
              action = "double"            ## double soft 16 to 18 againt 2 to 6
            elif player == 18:
              action = "stand"             ## stand with soft 18
            else:
              action = "hit"               ## hit soft 17
          else:
            action = "hit"                 ## hit soft 16 to 18 against 7 to A
        else:
          action = "stand"                 ## stand with soft 19 or more
          
    print(action, flush = True)
    if debug:
      print("-> %s" % action, file = sys.stderr) 
    
  elif line == "invalid_command":
    print("I sent an invalid command!", file = sys.stderr) 
  


```

Of course there is a catch. Even though this strategy has a positive expectation in the long run, the betting spread adds a lot of dispersion to the results so the number of hands needed to get an uncertainty below the actual expected mean is very large. This, summed to the fact that we are using an interpreted language and a verbose dealer (the player needs to know which cards are being dealt so she can keep the ace/five count) over standard input and output means a few minutes to run. Also, note that counting cards make sense only when playing a shoe game, so a positive number of decks needs to be explicitly given.

```terminal
##!/bin/sh
if test -z "$1"; then n=1e5; else n=$1; fi; rm -f f;  mkfifo f;
cat f | blackjack --decks=1 --verbose=true -n${n} | python3 ace-five.py > f
```

In any case, here are the results:

```yaml
result: "(+0.5 ± 0.2) %"
mean: 0.00460405
error: 0.00196465
hands: 1e+07
bankroll: 46040.5
bustsPlayer: 0.156041
bustsDealer: 0.236745
wins: 0.444615
pushes: 0.0834705
losses: 0.489667

```

> **Exercise:** explore what the effect of the number of decks.




