## Internal player

If `blackjack` is called with the `-i` option, it uses an *internal*
player to play against itself. By default it plays basic strategy. Run

``` {.terminal}
blackjack -i
```

and you will get the following report with the results of playing one
million hands with basic strategy.

``` {.yaml}
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


## Always stand

To play Blackjack as an "always-stander" run the following command:

``` {.terminal}
yes stand | blackjack -n1e5 --flat_bet=true --no_insurance=true > /dev/null
```

The UNIX command `yes stand` writes the string "stand" repeteadly to the
standard output, which is piped to the executable `blackjack` (assumed
to be installed system-wide). The arguments tell Libre Blackjack to play
one hundred thousand hands (`-n1e5`) using a flat bet (`flat_bet`, it
defaults to a unit bet in each hand) and without asking for insurance if
the dealer shows an ace (`no_insurance`). As there is no
`blackjack.conf` file, the rules are---as expected---the default ones
(see the documentation for details).

The `/dev/null` part is important, otherwise Libre Blackjack will think
that there is a human at the other side of the table and will

1.  run slower (it will add explicit time delays to mimic an actual
    human dealer), and
2.  give all the details of the dealt hands in the terminal as ASCII
    (actually UTF-8) art

This example is only one-way (i.e. the player ignores what the dealer
says) so it is better to redirect the standard output to `/dev/null` to
save execution time. The results are written as a
[YAML](http://yaml.org/)-formatted data to `stderr` by default once the
hands are over, so they will show up in the terminal nevertheless. This
format is human-friendly (far more than JSON) so it can be easily
parsed, but it also allows complex objects to be represented (arrays,
lists, etc.).

``` {.yaml}
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

> **Exercise:** verify that the analytical probability of getting a
> natural playing with a single deck (for both the dealer and the
> player) is 32/663 = 0.04826546...


## No-bust strategy

This directory shows how to play a "no-bust" strategy, i.e. not hitting
any hand higher or equal to hard twelve with Libre Blackjack. The
communication between the player and the back end is through standard
input and output. The player reads from its standard input
Libre Blackjack's commands and writes to its standard output the playing
commands. In order to do this a FIFO (a.k.a. named pipe) is needed. So
first, we create it (if it is not already created):

``` {.terminal}
mkfifo fifo
```

Then we execute `blackjack`, piping its output to the player (say
`no-bust.pl`) and reading the standard input from `fifo`, whilst at the
same time we redirect the player's standard output to `fifo`:

``` {.terminal}
rm -f fifo; mkfifo fifo
blackjack -n1e5 < fifo | ./no-bust.pl > fifo
```

As this time the player is coded in an interpreted langauge, it is far
smarter than the previous `yes`-based player. So the player can handle
bets and insurances, and there is not need to pass the options
`--flat_bet` nor `--no_insurance` (though they can be passed anyway).
Let us take a look at the Perl implementation:

``` {.perl}
#!/usr/bin/perl
# this is needed to avoid deadlock with the fifo
STDOUT->autoflush(1);

while ($command ne "bye") {
  # do not play more than a number of commands
  # if the argument -n was not passed to blackjack
  if ($i++ == 1234567) {
    print "quit\n";
  }
  
  # read and process the commands
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

The very same player may be implemented as a shell script:

``` {.bash}
#!/bin/sh

i=0
while read command
do
  i=$((i+1))
  if test ${i} -ge 12345; then
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

To check these two players give the same results, make them play against
Libre Blackjack with the same seed (say one) and send the YAML report to
two different files:

``` {.terminal}
blackjack -n1e5 --rng_seed=1 --report_file_path=perl.yml  < fifo | ./no-bust.pl  > fifo
blackjack -n1e5 --rng_seed=1 --report_file_path=shell.yml < fifo | ./no-bust.awk > fifo
diff perl.yml shell.yml 
```

As expected, the reports are the same. They just differ in the speed
because the shell script is orders of magnitude slower than its
Perl-based counterpart.

> **Exercise:** modify the players so they always insure aces and see if
> it improves or degrades the result.


## Mimic the dealer

This example implements a "mimic-the-dealer strategy," i.e. hits if the
hand totals less than seventeen and stands on eighteen or more. The
player stands on hard seventeen but hits on soft seventeen.

This time, the configuration file `blackjack.conf` is used. If a file
with this name exists in the directory where `blackjack` is executed, it
is read and parsed. The options should be fairly self descriptive. See
the \[configuration file\] section of the manual for a detailed
explanation of the variables and values that can be entered. In
particular, we ask to play one hundred thousand hands at a six-deck game
where the dealer hits soft seventeens. If the random seed is set to a
fixed value so each execution will lead to the very same sequence of
cards.

Now, there are two options that tell Libre Blackjack how the player is
going to talk to the backend: `player2dealer` and `dealer2player`. The
first one sets the communication mechanism from the player to the dealer
(by default is `blackjack`'s standard input), and the second one sets
the mechanism from the dealer to the player (by default `blackjack`'s
standard output). In this case, the configuration file reads:

``` {.ini}
h17 = true
```

This means that two FIFOs (a.k.a. named pipes) are to be used for
communication, `player2dealer` from the player to the dealer and
`dealer2player` for the dealer to the player. If these FIFOs do not
exist, they are created by `blackjack` upon execution.

The player this time is implemented as an awk script, whose input should
be read from `dealer2player` and whose output should be written to
`player2dealer`. To run the game, execute `blackjack` in one terminal
making sure the current directory is where the `blackjack.conf` file
exists. It should print a message telling that it is waiting for someone
to be at the other side of the named pipes:

``` {.terminal}
$ blackjack
[...]
waiting for dealer2player buffered fifo 'dealer2player'...
```

In another terminal run the player

``` {.terminal}
$ ./mimic-the-dealer.awk < dealer2player > player2dealer
```

Both dealer and player may be run in the same terminal putting the first
one on the background:

``` {.terminal}
rm -f d2p p2d; mkfifo d2p p2d
gawk -f mimic-the-dealer.awk < d2p > p2d &
blackjack -n1e5 > d2p < p2d 
```

To understand the decisions taken by the player, we have to remember
that when Libre Blackjack receives the command `count` asking for the
current player's count, it returns a positive number for hard hands and
a negative number for soft hands. The instructions `fflush()` are needed
in order to avoid deadlocks on the named pipes:

``` {.awk}
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

``` {.yaml}
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

> **Exercise:** modify the player and the configuration file so both the
> dealer and the player may stand on soft seventeen. Analyze the four
> combinations (player h17 - dealer h17, player h17 - dealer s17, player
> s17 - dealer h17, player s17 - dealer s17)


## Derivation of the basic strategy

### Quick run

Execute the `run.sh` script. It should take a few minutes:

``` {.terminal}
$ ./run.sh
h20-2 (10 10)   8.0e+04 +63.23  (1.1)   -171.17 (1.1)   -85.32  (0.5)   stand
h20-3 (10 10)   8.0e+04 +64.54  (1.1)   -171.50 (1.1)   -85.50  (0.5)   stand
h20-4 (10 10)   8.0e+04 +65.55  (1.1)   -170.33 (1.1)   -85.50  (0.5)   stand
h20-5 (10 10)   8.0e+04 +66.65  (1.1)   -171.25 (1.1)   -85.51  (0.5)   stand
h20-6 (10 10)   8.0e+04 +67.80  (1.1)   -171.07 (1.1)   -85.59  (0.5)   stand
h20-7 (10 10)   8.0e+04 +77.44  (1.1)   -170.53 (1.1)   -85.44  (0.5)   stand
h20-8 (10 10)   8.0e+04 +79.11  (1.1)   -170.08 (1.1)   -85.02  (0.6)   stand
h20-9 (10 10)   8.0e+04 +75.77  (1.1)   -170.31 (1.1)   -84.87  (0.6)   stand
[...]
p2-6            8e+04   +24.78  (2.9)   +3.07   (1.0)   yes
p2-7            8e+04   +1.48   (2.0)   -8.90   (1.0)   yes
p2-8            8e+04   -17.57  (2.0)   -16.33  (1.0)   uncertain
p2-8            3e+05   -17.88  (1.0)   -16.10  (0.5)   no
p2-9            8e+04   -38.73  (2.0)   -24.38  (1.0)   no
p2-T            8e+04   -54.45  (1.8)   -34.92  (0.9)   no
p2-A            8e+04   -67.11  (1.5)   -51.59  (0.9)   no
```

A new text file called `bs.txt` with the strategy should be created from
scratch:

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
    h11  d  d  d  d  d  d  d  d  d  d  
    h10  d  d  d  d  d  d  d  d  h  h  
    h9   h  d  d  d  d  h  h  h  h  h  
    h8   h  h  h  h  h  h  h  h  h  h  
    h7   h  h  h  h  h  h  h  h  h  h  
    h6   h  h  h  h  h  h  h  h  h  h  
    h5   h  h  h  h  h  h  h  h  h  h  
    h4   h  h  h  h  h  h  h  h  h  h  
    #    2  3  4  5  6  7  8  9  T  A
    s20  s  s  s  s  s  s  s  s  s  s  
    s19  s  s  s  s  d  s  s  s  s  s  
    s18  d  d  d  d  d  s  s  h  h  h  
    s17  h  d  d  d  d  h  h  h  h  h  
    s16  h  h  d  d  d  h  h  h  h  h  
    s15  h  h  d  d  d  h  h  h  h  h  
    s14  h  h  h  d  d  h  h  h  h  h  
    s13  h  h  h  h  d  h  h  h  h  h  
    s12  h  h  h  h  d  h  h  h  h  h  
    #    2  3  4  5  6  7  8  9  T  A
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

### Full table with results

The script computes the expected value of each combination

1.  Player's hand (hard, soft and pair)
2.  Dealer upcard
3.  Hit, double or stand (for hard and soft hands) and splitting or not
    (for pairs)

The results are given as the expected value in percentage with the
uncertainty (one standard deviation) in the last significant digit.

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
<div class="text-center " style='background-color: rgb(102,233,139)'>+47(2)</div><div class="text-center " style='background-color: rgb(170,189,152)'>+7(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(91,237,136)'>+53(2)</div><div class="text-center " style='background-color: rgb(167,192,152)'>+9(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(86,239,134)'>+56(2)</div><div class="text-center " style='background-color: rgb(163,195,152)'>+11(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(77,243,131)'>+61(2)</div><div class="text-center " style='background-color: rgb(158,199,151)'>+15(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(66,246,127)'>+66(2)</div><div class="text-center " style='background-color: rgb(148,207,150)'>+21(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(104,232,140)'>+46(2)</div><div class="text-center " style='background-color: rgb(154,202,151)'>+17(1)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center " style='background-color: rgb(123,222,145)'>+35(2)</div><div class="text-center " style='background-color: rgb(166,193,152)'>+10(1)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center " style='background-color: rgb(145,209,149)'>+23(2)</div><div class="text-center text-white" style='background-color: rgb(180,180,152)'>-0(1)</div> </td>
 <!-- T -->
 <td>
<div class="text-center " style='background-color: rgb(168,191,152)'>+8(2)</div><div class="text-center text-white" style='background-color: rgb(199,158,151)'>-14(1)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(210,143,149)'>-24(2)</div><div class="text-center text-white" style='background-color: rgb(222,125,145)'>-34.8(9)</div> </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(190,169,152)'>-7(4)</div><div class="text-center " style='background-color: rgb(72,244,129)'>+63.3(7)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(177,182,152)'>+2(4)</div><div class="text-center " style='background-color: rgb(70,245,129)'>+64.3(7)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(157,200,151)'>+15(4)</div><div class="text-center " style='background-color: rgb(67,245,128)'>+65.8(7)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(141,211,149)'>+25(4)</div><div class="text-center " style='background-color: rgb(65,246,127)'>+66.8(7)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(120,224,144)'>+37(4)</div><div class="text-center " style='background-color: rgb(63,246,126)'>+67.8(7)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(166,193,152)'>+9(3)</div><div class="text-center " style='background-color: rgb(46,250,120)'>+76.8(6)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(210,143,149)'>-24(2)</div><div class="text-center " style='background-color: rgb(41,251,118)'>+79.3(6)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(242,80,132)'>-59(3)</div><div class="text-center " style='background-color: rgb(47,250,120)'>+75.9(6)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(251,44,119)'>-78(3)</div><div class="text-center " style='background-color: rgb(109,230,141)'>+43.3(7)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(253,29,113)'>-85(2)</div><div class="text-center " style='background-color: rgb(164,194,152)'>+11(1)</div> </td>
 <!-- 2 -->
 <td>
<div class="text-center " style='background-color: rgb(149,206,150)'>+20(2)</div><div class="text-center " style='background-color: rgb(163,195,152)'>+11(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(141,212,149)'>+25(2)</div><div class="text-center " style='background-color: rgb(160,198,151)'>+13(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(127,220,146)'>+33(2)</div><div class="text-center " style='background-color: rgb(154,202,151)'>+17(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(117,226,143)'>+39(2)</div><div class="text-center " style='background-color: rgb(151,205,150)'>+19(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(105,232,140)'>+46(2)</div><div class="text-center " style='background-color: rgb(145,209,149)'>+23(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(122,223,145)'>+36(2)</div><div class="text-center " style='background-color: rgb(115,227,143)'>+40.3(9)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center " style='background-color: rgb(143,210,149)'>+24(2)</div><div class="text-center " style='background-color: rgb(164,194,152)'>+10.6(8)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(189,170,152)'>-7(2)</div><div class="text-center text-white" style='background-color: rgb(204,152,150)'>-18(1)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(224,121,144)'>-37(2)</div><div class="text-center text-white" style='background-color: rgb(210,143,149)'>-24(1)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(234,101,139)'>-48.1(8)</div><div class="text-center text-white" style='background-color: rgb(232,104,140)'>-46.3(4)</div> </td>
 <!-- 2 -->
 <td>
<div class="text-center " style='background-color: rgb(172,188,152)'>+6(2)</div><div class="text-center text-white" style='background-color: rgb(215,136,148)'>-28(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(160,198,151)'>+13(2)</div><div class="text-center text-white" style='background-color: rgb(211,142,149)'>-24(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(149,206,150)'>+20(2)</div><div class="text-center text-white" style='background-color: rgb(206,148,150)'>-21(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(135,216,147)'>+29(2)</div><div class="text-center text-white" style='background-color: rgb(201,156,151)'>-16(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(118,225,143)'>+39(3)</div><div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(128,220,146)'>+33(2)</div><div class="text-center text-white" style='background-color: rgb(228,113,142)'>-41.1(9)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(183,177,152)'>-2(2)</div><div class="text-center text-white" style='background-color: rgb(232,104,140)'>-46.3(9)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(226,116,143)'>-40(2)</div><div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51.0(9)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,94,137)'>-52(2)</div><div class="text-center text-white" style='background-color: rgb(240,83,133)'>-57.4(8)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(246,66,127)'>-66.6(8)</div><div class="text-center text-white" style='background-color: rgb(247,62,126)'>-68.3(4)</div> </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(199,159,151)'>-14(3)</div><div class="text-center text-white" style='background-color: rgb(215,135,147)'>-29(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(187,173,152)'>-5(3)</div><div class="text-center text-white" style='background-color: rgb(212,140,148)'>-26(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(172,188,152)'>+6(3)</div><div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(156,200,151)'>+16(3)</div><div class="text-center text-white" style='background-color: rgb(201,155,151)'>-16(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(140,212,148)'>+26(3)</div><div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(187,172,152)'>-5(2)</div><div class="text-center text-white" style='background-color: rgb(219,129,146)'>-32(1)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(226,116,143)'>-40(1)</div><div class="text-center text-white" style='background-color: rgb(224,120,144)'>-37.2(5)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,82,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(229,110,141)'>-42.9(9)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(247,62,126)'>-68(2)</div><div class="text-center text-white" style='background-color: rgb(236,96,137)'>-50.7(9)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(252,36,116)'>-82(2)</div><div class="text-center text-white" style='background-color: rgb(244,72,129)'>-63.5(8)</div> </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(203,153,151)'>-18(2)</div><div class="text-center text-white" style='background-color: rgb(213,139,148)'>-26(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(195,163,152)'>-11(3)</div><div class="text-center text-white" style='background-color: rgb(210,143,149)'>-24(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(180,179,152)'>-0(3)</div><div class="text-center text-white" style='background-color: rgb(206,149,150)'>-21(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(168,191,152)'>+8(3)</div><div class="text-center text-white" style='background-color: rgb(202,155,151)'>-17(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(152,204,150)'>+18(3)</div><div class="text-center text-white" style='background-color: rgb(197,161,152)'>-13(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(213,139,148)'>-26(2)</div><div class="text-center text-white" style='background-color: rgb(208,147,150)'>-22(1)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(228,112,142)'>-42(2)</div><div class="text-center text-white" style='background-color: rgb(214,137,148)'>-27(1)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(243,76,131)'>-61(2)</div><div class="text-center text-white" style='background-color: rgb(221,126,145)'>-34(1)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(249,50,121)'>-74(2)</div><div class="text-center text-white" style='background-color: rgb(229,110,141)'>-43.0(9)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(252,35,116)'>-82(1)</div><div class="text-center text-white" style='background-color: rgb(240,83,133)'>-57.4(8)</div> </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,135,147)'>-29(2)</div><div class="text-center " style='background-color: rgb(122,223,144)'>+36(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(205,150,150)'>-20(2)</div><div class="text-center " style='background-color: rgb(115,227,143)'>+40(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(3)</div><div class="text-center " style='background-color: rgb(105,232,140)'>+46(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(187,173,152)'>-5(3)</div><div class="text-center " style='background-color: rgb(93,237,136)'>+52(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(167,191,152)'>+8(3)</div><div class="text-center " style='background-color: rgb(83,241,133)'>+58(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(217,133,147)'>-30(2)</div><div class="text-center " style='background-color: rgb(118,225,144)'>+38(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(232,105,140)'>-46(2)</div><div class="text-center " style='background-color: rgb(133,217,147)'>+30(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(244,72,129)'>-63(2)</div><div class="text-center " style='background-color: rgb(159,199,151)'>+14(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(250,48,121)'>-75(2)</div><div class="text-center text-white" style='background-color: rgb(187,172,152)'>-5(1)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(252,33,115)'>-83(1)</div><div class="text-center text-white" style='background-color: rgb(215,135,147)'>-29(1)</div> </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(204,152,150)'>-18(2)</div><div class="text-center text-white" style='background-color: rgb(183,176,152)'>-2(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(195,163,152)'>-11(3)</div><div class="text-center " style='background-color: rgb(180,180,152)'>+0(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(181,179,152)'>-1(3)</div><div class="text-center " style='background-color: rgb(175,185,152)'>+3(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(168,191,152)'>+8.3(7)</div><div class="text-center " style='background-color: rgb(170,190,152)'>+7.1(3)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(151,205,150)'>+19(3)</div><div class="text-center " style='background-color: rgb(167,192,152)'>+9(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,155,151)'>-16(2)</div><div class="text-center " style='background-color: rgb(168,191,152)'>+8(1)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(220,128,146)'>-33(2)</div><div class="text-center text-white" style='background-color: rgb(188,171,152)'>-6(1)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,96,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(1)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(245,67,128)'>-66(2)</div><div class="text-center text-white" style='background-color: rgb(218,132,147)'>-31(1)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(249,50,121)'>-75(2)</div><div class="text-center text-white" style='background-color: rgb(234,99,138)'>-49.2(9)</div> </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(198,159,151)'>-13.64(1)</div><div class="text-center text-white" style='background-color: rgb(198,159,151)'>-13.80(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(187,172,152)'>-5(3)</div><div class="text-center text-white" style='background-color: rgb(194,164,152)'>-10(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(174,185,152)'>+4(3)</div><div class="text-center text-white" style='background-color: rgb(190,169,152)'>-7(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(161,197,152)'>+13(3)</div><div class="text-center text-white" style='background-color: rgb(184,175,152)'>-3(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(145,209,149)'>+23(3)</div><div class="text-center " style='background-color: rgb(179,180,152)'>+0(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(187,173,152)'>-5(2)</div><div class="text-center text-white" style='background-color: rgb(200,158,151)'>-15(1)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(210,144,149)'>-23(1)</div><div class="text-center text-white" style='background-color: rgb(208,147,150)'>-21.7(5)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(230,109,141)'>-44(2)</div><div class="text-center text-white" style='background-color: rgb(216,134,147)'>-29(1)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,81,132)'>-59(2)</div><div class="text-center text-white" style='background-color: rgb(226,117,143)'>-39.0(9)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(247,59,125)'>-70(2)</div><div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54.6(8)</div> </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(192,167,152)'>-9(1)</div><div class="text-center text-white" style='background-color: rgb(195,163,152)'>-11.5(5)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(183,177,152)'>-2(2)</div><div class="text-center text-white" style='background-color: rgb(192,167,152)'>-9(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(171,188,152)'>+6(3)</div><div class="text-center text-white" style='background-color: rgb(187,173,152)'>-5(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(156,201,151)'>+16(3)</div><div class="text-center text-white" style='background-color: rgb(182,178,152)'>-1(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(142,211,149)'>+25(3)</div><div class="text-center " style='background-color: rgb(175,184,152)'>+3(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(178,182,152)'>+1(2)</div><div class="text-center text-white" style='background-color: rgb(192,167,152)'>-9(1)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(203,153,151)'>-18(1)</div><div class="text-center text-white" style='background-color: rgb(201,156,151)'>-16.1(5)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(226,118,143)'>-39(2)</div><div class="text-center text-white" style='background-color: rgb(211,142,149)'>-24(1)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(222,124,145)'>-34.9(9)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(246,65,127)'>-67(2)</div><div class="text-center text-white" style='background-color: rgb(236,94,137)'>-51.6(9)</div> </td>

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
<div class="text-center " style='background-color: rgb(71,244,129)'>+64(2)</div><div class="text-center " style='background-color: rgb(153,203,150)'>+18(1)</div><div class="text-center " style='background-color: rgb(124,222,145)'>+35(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(69,245,128)'>+65(2)</div><div class="text-center " style='background-color: rgb(149,206,150)'>+20(1)</div><div class="text-center " style='background-color: rgb(116,226,143)'>+40(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(67,245,128)'>+66(2)</div><div class="text-center " style='background-color: rgb(144,209,149)'>+23(1)</div><div class="text-center " style='background-color: rgb(106,231,140)'>+45(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(65,246,127)'>+67(2)</div><div class="text-center " style='background-color: rgb(140,213,148)'>+26(1)</div><div class="text-center " style='background-color: rgb(95,236,137)'>+51(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(63,247,126)'>+68(2)</div><div class="text-center " style='background-color: rgb(136,215,148)'>+28(1)</div><div class="text-center " style='background-color: rgb(87,239,134)'>+56(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(44,251,119)'>+77(2)</div><div class="text-center " style='background-color: rgb(141,212,149)'>+25(1)</div><div class="text-center " style='background-color: rgb(115,227,143)'>+40(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center " style='background-color: rgb(41,251,118)'>+79(2)</div><div class="text-center " style='background-color: rgb(149,206,150)'>+20(1)</div><div class="text-center " style='background-color: rgb(136,215,148)'>+28(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center " style='background-color: rgb(47,250,120)'>+76(2)</div><div class="text-center " style='background-color: rgb(163,195,152)'>+11(1)</div><div class="text-center " style='background-color: rgb(158,199,151)'>+15(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center " style='background-color: rgb(109,230,141)'>+44(2)</div><div class="text-center text-white" style='background-color: rgb(187,172,152)'>-5(1)</div><div class="text-center text-white" style='background-color: rgb(191,167,152)'>-9(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center " style='background-color: rgb(164,194,152)'>+11(2)</div><div class="text-center text-white" style='background-color: rgb(215,136,148)'>-28(1)</div><div class="text-center text-white" style='background-color: rgb(220,127,146)'>-33(2)</div> </td>
 <tr>
  <td>s19</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center " style='background-color: rgb(118,225,144)'>+38(2)</div><div class="text-center " style='background-color: rgb(162,196,152)'>+12(1)</div><div class="text-center " style='background-color: rgb(144,210,149)'>+24(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(116,227,143)'>+40(2)</div><div class="text-center " style='background-color: rgb(157,200,151)'>+15(1)</div><div class="text-center " style='background-color: rgb(133,217,147)'>+30(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(111,229,142)'>+42(2)</div><div class="text-center " style='background-color: rgb(153,203,151)'>+18(1)</div><div class="text-center " style='background-color: rgb(124,222,145)'>+35(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(109,230,141)'>+44(1)</div><div class="text-center " style='background-color: rgb(149,206,150)'>+20.1(5)</div><div class="text-center " style='background-color: rgb(114,228,142)'>+41(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(106,231,140)'>+45.4(2)</div><div class="text-center " style='background-color: rgb(144,209,149)'>+23.1(1)</div><div class="text-center " style='background-color: rgb(104,232,140)'>+46.2(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(75,243,130)'>+62(2)</div><div class="text-center " style='background-color: rgb(147,208,150)'>+22(1)</div><div class="text-center " style='background-color: rgb(130,219,146)'>+32(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center " style='background-color: rgb(80,242,132)'>+59(2)</div><div class="text-center " style='background-color: rgb(157,200,151)'>+15(1)</div><div class="text-center " style='background-color: rgb(149,206,150)'>+20(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center " style='background-color: rgb(135,215,147)'>+28(2)</div><div class="text-center " style='background-color: rgb(179,181,152)'>+1(1)</div><div class="text-center text-white" style='background-color: rgb(191,168,152)'>-8(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(182,177,152)'>-2(2)</div><div class="text-center text-white" style='background-color: rgb(201,156,151)'>-16(1)</div><div class="text-center text-white" style='background-color: rgb(216,135,147)'>-29(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(204,152,150)'>-18(2)</div><div class="text-center text-white" style='background-color: rgb(222,123,145)'>-35.4(9)</div><div class="text-center text-white" style='background-color: rgb(233,103,139)'>-47(2)</div> </td>
 <tr>
  <td>s18</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center " style='background-color: rgb(164,195,152)'>+11.03(1)</div><div class="text-center " style='background-color: rgb(171,188,152)'>+5.99(1)</div><div class="text-center " style='background-color: rgb(163,195,152)'>+11.48(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(160,198,151)'>+14(1)</div><div class="text-center " style='background-color: rgb(167,192,152)'>+8.8(5)</div><div class="text-center " style='background-color: rgb(153,203,151)'>+18(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(156,201,151)'>+16(2)</div><div class="text-center " style='background-color: rgb(163,195,152)'>+11(1)</div><div class="text-center " style='background-color: rgb(145,209,149)'>+23(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(150,206,150)'>+20(2)</div><div class="text-center " style='background-color: rgb(158,199,151)'>+14(1)</div><div class="text-center " style='background-color: rgb(135,216,147)'>+29(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(145,209,149)'>+23(2)</div><div class="text-center " style='background-color: rgb(156,200,151)'>+16(1)</div><div class="text-center " style='background-color: rgb(125,222,145)'>+35(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(116,226,143)'>+40(2)</div><div class="text-center " style='background-color: rgb(154,202,151)'>+17(1)</div><div class="text-center " style='background-color: rgb(146,209,149)'>+22(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center " style='background-color: rgb(164,194,152)'>+10(2)</div><div class="text-center " style='background-color: rgb(173,186,152)'>+4(1)</div><div class="text-center text-white" style='background-color: rgb(185,175,152)'>-4(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(204,151,150)'>-19(2)</div><div class="text-center text-white" style='background-color: rgb(193,165,152)'>-10(1)</div><div class="text-center text-white" style='background-color: rgb(216,134,147)'>-29(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(210,143,149)'>-24(2)</div><div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(1)</div><div class="text-center text-white" style='background-color: rgb(226,117,143)'>-39(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(232,103,139)'>-47(2)</div><div class="text-center text-white" style='background-color: rgb(228,112,142)'>-41.6(9)</div><div class="text-center text-white" style='background-color: rgb(242,80,132)'>-59(2)</div> </td>
 <tr>
  <td>s17</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(201,156,151)'>-15.66(1)</div><div class="text-center text-white" style='background-color: rgb(181,179,152)'>-0.55(1)</div><div class="text-center text-white" style='background-color: rgb(181,179,152)'>-0.87(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(1)</div><div class="text-center " style='background-color: rgb(176,183,152)'>+2.4(5)</div><div class="text-center " style='background-color: rgb(172,187,152)'>+5(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(191,168,152)'>-8(2)</div><div class="text-center " style='background-color: rgb(173,187,152)'>+5(1)</div><div class="text-center " style='background-color: rgb(162,196,152)'>+12(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(187,173,152)'>-5(2)</div><div class="text-center " style='background-color: rgb(167,192,152)'>+9(1)</div><div class="text-center " style='background-color: rgb(153,203,151)'>+18(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(181,179,152)'>-1(2)</div><div class="text-center " style='background-color: rgb(165,194,152)'>+10(1)</div><div class="text-center " style='background-color: rgb(142,211,149)'>+24(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(194,165,152)'>-10(2)</div><div class="text-center " style='background-color: rgb(172,187,152)'>+5(1)</div><div class="text-center text-white" style='background-color: rgb(180,179,152)'>-0(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(225,118,144)'>-38(2)</div><div class="text-center text-white" style='background-color: rgb(190,170,152)'>-7(1)</div><div class="text-center text-white" style='background-color: rgb(213,139,148)'>-26(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(228,112,142)'>-42(2)</div><div class="text-center text-white" style='background-color: rgb(200,157,151)'>-15(1)</div><div class="text-center text-white" style='background-color: rgb(227,115,143)'>-40(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(232,105,140)'>-46(2)</div><div class="text-center text-white" style='background-color: rgb(212,140,148)'>-26(1)</div><div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(246,66,127)'>-66(2)</div><div class="text-center text-white" style='background-color: rgb(232,104,140)'>-46.3(9)</div><div class="text-center text-white" style='background-color: rgb(247,61,125)'>-69(2)</div> </td>
 <tr>
  <td>s16</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(216,135,147)'>-29(2)</div><div class="text-center text-white" style='background-color: rgb(183,176,152)'>-2(1)</div><div class="text-center text-white" style='background-color: rgb(189,171,152)'>-6(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,141,149)'>-25(1)</div><div class="text-center " style='background-color: rgb(179,181,152)'>+0.6(5)</div><div class="text-center text-white" style='background-color: rgb(181,178,152)'>-1(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(207,147,150)'>-21(2)</div><div class="text-center " style='background-color: rgb(175,184,152)'>+3(1)</div><div class="text-center " style='background-color: rgb(170,189,152)'>+7(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(201,156,151)'>-16(2)</div><div class="text-center " style='background-color: rgb(170,189,152)'>+7(1)</div><div class="text-center " style='background-color: rgb(160,198,151)'>+14(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(197,161,152)'>-12(2)</div><div class="text-center " style='background-color: rgb(168,191,152)'>+8(1)</div><div class="text-center " style='background-color: rgb(150,206,150)'>+20(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,102,139)'>-47(2)</div><div class="text-center text-white" style='background-color: rgb(182,178,152)'>-1(1)</div><div class="text-center text-white" style='background-color: rgb(204,152,150)'>-19(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(189,170,152)'>-7(1)</div><div class="text-center text-white" style='background-color: rgb(218,131,147)'>-31(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(199,158,151)'>-15(1)</div><div class="text-center text-white" style='background-color: rgb(232,104,140)'>-46(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,83,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(213,138,148)'>-27(1)</div><div class="text-center text-white" style='background-color: rgb(239,87,134)'>-55(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(249,54,123)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(231,107,141)'>-44.4(9)</div><div class="text-center text-white" style='background-color: rgb(248,56,124)'>-71(2)</div> </td>
 <tr>
  <td>s15</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(216,134,147)'>-29(2)</div><div class="text-center text-white" style='background-color: rgb(181,179,152)'>-1(1)</div><div class="text-center text-white" style='background-color: rgb(188,172,152)'>-6(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-25(2)</div><div class="text-center " style='background-color: rgb(175,184,152)'>+3(1)</div><div class="text-center text-white" style='background-color: rgb(182,178,152)'>-1(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(206,148,150)'>-20.6(5)</div><div class="text-center " style='background-color: rgb(172,188,152)'>+5.6(3)</div><div class="text-center " style='background-color: rgb(170,189,152)'>+6.4(5)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,155,151)'>-17(1)</div><div class="text-center " style='background-color: rgb(167,192,152)'>+8.9(5)</div><div class="text-center " style='background-color: rgb(161,197,151)'>+13(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center " style='background-color: rgb(165,194,152)'>+10(1)</div><div class="text-center " style='background-color: rgb(151,205,150)'>+19(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,101,139)'>-48(2)</div><div class="text-center " style='background-color: rgb(174,186,152)'>+4(1)</div><div class="text-center text-white" style='background-color: rgb(205,150,150)'>-19(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(184,176,152)'>-3(1)</div><div class="text-center text-white" style='background-color: rgb(218,131,147)'>-31(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(195,163,152)'>-11(1)</div><div class="text-center text-white" style='background-color: rgb(233,103,139)'>-47(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,83,133)'>-57(2)</div><div class="text-center text-white" style='background-color: rgb(210,143,149)'>-24(1)</div><div class="text-center text-white" style='background-color: rgb(239,88,135)'>-55(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(229,110,141)'>-42.7(9)</div><div class="text-center text-white" style='background-color: rgb(248,56,124)'>-72(2)</div> </td>
 <tr>
  <td>s14</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,135,147)'>-29(2)</div><div class="text-center " style='background-color: rgb(178,182,152)'>+1(1)</div><div class="text-center text-white" style='background-color: rgb(189,170,152)'>-7(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,141,149)'>-25(2)</div><div class="text-center " style='background-color: rgb(174,185,152)'>+4(1)</div><div class="text-center text-white" style='background-color: rgb(180,179,152)'>-0(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(2)</div><div class="text-center " style='background-color: rgb(169,190,152)'>+8(1)</div><div class="text-center " style='background-color: rgb(172,187,152)'>+5(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(201,155,151)'>-16(1)</div><div class="text-center " style='background-color: rgb(164,194,152)'>+10.6(5)</div><div class="text-center " style='background-color: rgb(161,197,151)'>+13(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center " style='background-color: rgb(162,196,152)'>+12(1)</div><div class="text-center " style='background-color: rgb(150,205,150)'>+20(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,102,139)'>-47(2)</div><div class="text-center " style='background-color: rgb(169,190,152)'>+8(1)</div><div class="text-center text-white" style='background-color: rgb(204,152,150)'>-18(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center " style='background-color: rgb(178,181,152)'>+1(1)</div><div class="text-center text-white" style='background-color: rgb(218,131,147)'>-31(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(239,88,135)'>-55(2)</div><div class="text-center text-white" style='background-color: rgb(191,168,152)'>-8(1)</div><div class="text-center text-white" style='background-color: rgb(232,104,140)'>-46(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,83,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(207,147,150)'>-21(1)</div><div class="text-center text-white" style='background-color: rgb(239,88,135)'>-55(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(227,115,143)'>-40.0(9)</div><div class="text-center text-white" style='background-color: rgb(248,56,123)'>-72(2)</div> </td>
 <tr>
  <td>s13</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(216,135,147)'>-29(2)</div><div class="text-center " style='background-color: rgb(174,185,152)'>+4(1)</div><div class="text-center text-white" style='background-color: rgb(190,169,152)'>-8(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(212,141,149)'>-25(2)</div><div class="text-center " style='background-color: rgb(169,190,152)'>+7(1)</div><div class="text-center text-white" style='background-color: rgb(180,180,152)'>-0(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(2)</div><div class="text-center " style='background-color: rgb(166,193,152)'>+9(1)</div><div class="text-center " style='background-color: rgb(173,187,152)'>+5(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,155,151)'>-16.49(1)</div><div class="text-center " style='background-color: rgb(161,197,151)'>+12.84(1)</div><div class="text-center " style='background-color: rgb(161,197,151)'>+12.70(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,161,152)'>-12(2)</div><div class="text-center " style='background-color: rgb(159,198,151)'>+14(1)</div><div class="text-center " style='background-color: rgb(151,204,150)'>+19(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,102,139)'>-47(2)</div><div class="text-center " style='background-color: rgb(161,197,152)'>+13(1)</div><div class="text-center text-white" style='background-color: rgb(204,152,150)'>-18(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center " style='background-color: rgb(172,187,152)'>+5(1)</div><div class="text-center text-white" style='background-color: rgb(219,130,146)'>-32(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(185,174,152)'>-4(1)</div><div class="text-center text-white" style='background-color: rgb(232,104,139)'>-47(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,83,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(202,155,151)'>-17(1)</div><div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,56,124)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(225,119,144)'>-38.0(9)</div><div class="text-center text-white" style='background-color: rgb(247,59,125)'>-70(2)</div> </td>
 <tr>
  <td>s12</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,135,148)'>-28(2)</div><div class="text-center " style='background-color: rgb(169,190,152)'>+7(1)</div><div class="text-center text-white" style='background-color: rgb(188,171,152)'>-6(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(212,140,148)'>-26(2)</div><div class="text-center " style='background-color: rgb(166,193,152)'>+10(1)</div><div class="text-center text-white" style='background-color: rgb(180,180,152)'>-0(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(2)</div><div class="text-center " style='background-color: rgb(161,197,151)'>+13(1)</div><div class="text-center " style='background-color: rgb(171,188,152)'>+6(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,155,151)'>-16(1)</div><div class="text-center " style='background-color: rgb(157,200,151)'>+15.2(5)</div><div class="text-center " style='background-color: rgb(160,198,151)'>+13(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,163,152)'>-12(2)</div><div class="text-center " style='background-color: rgb(157,200,151)'>+15(1)</div><div class="text-center " style='background-color: rgb(150,205,150)'>+20(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,102,139)'>-47(2)</div><div class="text-center " style='background-color: rgb(155,201,151)'>+16(1)</div><div class="text-center text-white" style='background-color: rgb(202,154,151)'>-17(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,94,137)'>-52(2)</div><div class="text-center " style='background-color: rgb(166,193,152)'>+9(1)</div><div class="text-center text-white" style='background-color: rgb(218,132,147)'>-31(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(180,180,152)'>-0(1)</div><div class="text-center text-white" style='background-color: rgb(231,107,141)'>-44(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,83,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(199,159,151)'>-14(1)</div><div class="text-center text-white" style='background-color: rgb(240,84,133)'>-57(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(249,54,123)'>-73(2)</div><div class="text-center text-white" style='background-color: rgb(222,125,145)'>-34.5(9)</div><div class="text-center text-white" style='background-color: rgb(248,58,124)'>-71(2)</div> </td>

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
<div class="text-center " style='background-color: rgb(72,244,129)'>+63(1)</div><div class="text-center text-white" style='background-color: rgb(253,29,113)'>-85.3(5)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-171(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(70,245,128)'>+65(1)</div><div class="text-center text-white" style='background-color: rgb(253,28,113)'>-85.5(5)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-172(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(68,245,128)'>+66(1)</div><div class="text-center text-white" style='background-color: rgb(253,28,113)'>-85.5(5)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-170(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(66,246,127)'>+67(1)</div><div class="text-center text-white" style='background-color: rgb(253,28,113)'>-85.5(5)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-171(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(63,246,126)'>+68(1)</div><div class="text-center text-white" style='background-color: rgb(253,28,113)'>-85.6(5)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-171(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(44,251,119)'>+77(1)</div><div class="text-center text-white" style='background-color: rgb(253,29,113)'>-85.4(5)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-171(1)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center " style='background-color: rgb(41,251,118)'>+79(1)</div><div class="text-center text-white" style='background-color: rgb(253,29,113)'>-85.0(6)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-170(1)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center " style='background-color: rgb(48,250,120)'>+76(1)</div><div class="text-center text-white" style='background-color: rgb(253,30,114)'>-84.9(6)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-170(1)</div> </td>
 <!-- T -->
 <td>
<div class="text-center " style='background-color: rgb(109,230,141)'>+43(1)</div><div class="text-center text-white" style='background-color: rgb(253,28,113)'>-85.9(5)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-164(1)</div> </td>
 <!-- A -->
 <td>
<div class="text-center " style='background-color: rgb(165,194,152)'>+10(1)</div><div class="text-center text-white" style='background-color: rgb(254,20,110)'>-89.7(5)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-149(1)</div> </td>
 <tr>
  <td>h19</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center " style='background-color: rgb(120,224,144)'>+38(1)</div><div class="text-center text-white" style='background-color: rgb(249,52,122)'>-73.5(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-147(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(116,226,143)'>+39(1)</div><div class="text-center text-white" style='background-color: rgb(249,54,123)'>-72.6(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-145(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(112,228,142)'>+42(1)</div><div class="text-center text-white" style='background-color: rgb(249,53,123)'>-72.9(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-145(1)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(109,230,141)'>+44(1)</div><div class="text-center text-white" style='background-color: rgb(249,53,123)'>-73.0(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-145(1)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(106,231,140)'>+45(1)</div><div class="text-center text-white" style='background-color: rgb(249,54,123)'>-72.6(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-145(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(75,243,130)'>+62(2)</div><div class="text-center text-white" style='background-color: rgb(248,57,124)'>-71.1(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-143(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center " style='background-color: rgb(80,241,132)'>+59(2)</div><div class="text-center text-white" style='background-color: rgb(248,57,124)'>-71.2(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-143(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center " style='background-color: rgb(135,215,147)'>+29(2)</div><div class="text-center text-white" style='background-color: rgb(248,55,123)'>-71.9(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-143(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(183,177,152)'>-2(1)</div><div class="text-center text-white" style='background-color: rgb(250,49,121)'>-75.1(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-143(1)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(203,153,150)'>-18(1)</div><div class="text-center text-white" style='background-color: rgb(252,37,117)'>-81.0(6)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-132(1)</div> </td>
 <tr>
  <td>h18</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center " style='background-color: rgb(164,195,152)'>+11(2)</div><div class="text-center text-white" style='background-color: rgb(243,75,130)'>-61.9(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-124(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center " style='background-color: rgb(160,197,151)'>+13(2)</div><div class="text-center text-white" style='background-color: rgb(244,74,130)'>-62.5(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-125(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center " style='background-color: rgb(154,202,151)'>+17(2)</div><div class="text-center text-white" style='background-color: rgb(243,75,130)'>-61.9(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-124(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center " style='background-color: rgb(151,205,150)'>+19(2)</div><div class="text-center text-white" style='background-color: rgb(243,74,130)'>-62.2(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-124(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center " style='background-color: rgb(146,208,149)'>+22(2)</div><div class="text-center text-white" style='background-color: rgb(243,75,131)'>-61.5(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-122(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center " style='background-color: rgb(116,226,143)'>+40(2)</div><div class="text-center text-white" style='background-color: rgb(241,80,132)'>-58.9(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-119(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center " style='background-color: rgb(164,195,152)'>+11(2)</div><div class="text-center text-white" style='background-color: rgb(242,80,132)'>-59.2(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-119(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(204,152,150)'>-18(2)</div><div class="text-center text-white" style='background-color: rgb(243,75,130)'>-61.7(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-123(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-24(2)</div><div class="text-center text-white" style='background-color: rgb(246,64,126)'>-67.4(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-127(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(232,104,140)'>-46(1)</div><div class="text-center text-white" style='background-color: rgb(249,50,121)'>-74.7(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-118(1)</div> </td>
 <tr>
  <td>h17</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(201,156,151)'>-16(2)</div><div class="text-center text-white" style='background-color: rgb(238,90,135)'>-53.8(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-107(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center text-white" style='background-color: rgb(238,91,136)'>-53.3(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-107(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(191,168,152)'>-8(2)</div><div class="text-center text-white" style='background-color: rgb(237,92,136)'>-52.7(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-106(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(186,173,152)'>-5(2)</div><div class="text-center text-white" style='background-color: rgb(237,92,136)'>-52.6(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-104(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(180,180,152)'>-0(2)</div><div class="text-center text-white" style='background-color: rgb(236,94,137)'>-51.6(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-105(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(195,164,152)'>-11(2)</div><div class="text-center text-white" style='background-color: rgb(234,99,138)'>-48.7(9)</div><div class="text-center text-white" style='background-color: rgb(254,7,104)'>-96(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(225,119,144)'>-38(2)</div><div class="text-center text-white" style='background-color: rgb(236,95,137)'>-50.9(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-102(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(229,111,142)'>-42(2)</div><div class="text-center text-white" style='background-color: rgb(239,87,134)'>-55.6(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-110(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(232,103,139)'>-47(2)</div><div class="text-center text-white" style='background-color: rgb(243,75,130)'>-61.7(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-116(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(246,66,127)'>-66(1)</div><div class="text-center text-white" style='background-color: rgb(248,57,124)'>-71.0(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-111(1)</div> </td>
 <tr>
  <td>h16</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(216,135,147)'>-29(2)</div><div class="text-center text-white" style='background-color: rgb(233,102,139)'>-47.3(9)</div><div class="text-center text-white" style='background-color: rgb(254,9,105)'>-95(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-24(2)</div><div class="text-center text-white" style='background-color: rgb(232,103,139)'>-46.5(9)</div><div class="text-center text-white" style='background-color: rgb(254,14,107)'>-93(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(2)</div><div class="text-center text-white" style='background-color: rgb(232,104,140)'>-46.2(9)</div><div class="text-center text-white" style='background-color: rgb(254,16,108)'>-92(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,154,151)'>-17(2)</div><div class="text-center text-white" style='background-color: rgb(231,106,140)'>-45.2(9)</div><div class="text-center text-white" style='background-color: rgb(254,21,110)'>-89(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,163,152)'>-12(2)</div><div class="text-center text-white" style='background-color: rgb(230,108,141)'>-44.2(9)</div><div class="text-center text-white" style='background-color: rgb(254,22,110)'>-89(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,102,139)'>-47(2)</div><div class="text-center text-white" style='background-color: rgb(228,112,142)'>-41.9(9)</div><div class="text-center text-white" style='background-color: rgb(252,32,114)'>-84(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(232,104,140)'>-46.3(9)</div><div class="text-center text-white" style='background-color: rgb(254,16,108)'>-92(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51.2(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-102(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,83,133)'>-57.57(1)</div><div class="text-center text-white" style='background-color: rgb(240,83,133)'>-57.55(1)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-107.33(1)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72(1)</div><div class="text-center text-white" style='background-color: rgb(247,62,126)'>-68.3(7)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-105(1)</div> </td>
 <tr>
  <td>h15</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,136,148)'>-28(2)</div><div class="text-center text-white" style='background-color: rgb(228,112,142)'>-41.6(9)</div><div class="text-center text-white" style='background-color: rgb(252,34,115)'>-83(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-24(2)</div><div class="text-center text-white" style='background-color: rgb(228,113,142)'>-41.0(9)</div><div class="text-center text-white" style='background-color: rgb(252,38,117)'>-81(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(2)</div><div class="text-center text-white" style='background-color: rgb(227,116,143)'>-39.8(9)</div><div class="text-center text-white" style='background-color: rgb(251,40,118)'>-80(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,155,151)'>-17(2)</div><div class="text-center text-white" style='background-color: rgb(225,119,144)'>-38(1)</div><div class="text-center text-white" style='background-color: rgb(250,45,120)'>-77(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(197,161,152)'>-13(2)</div><div class="text-center text-white" style='background-color: rgb(224,120,144)'>-37(1)</div><div class="text-center text-white" style='background-color: rgb(250,47,120)'>-76(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,101,139)'>-48(2)</div><div class="text-center text-white" style='background-color: rgb(224,120,144)'>-37.4(9)</div><div class="text-center text-white" style='background-color: rgb(249,51,122)'>-74(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,96,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(228,113,142)'>-41.0(9)</div><div class="text-center text-white" style='background-color: rgb(253,30,114)'>-85(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(233,103,139)'>-47.0(9)</div><div class="text-center text-white" style='background-color: rgb(254,9,105)'>-95(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,83,133)'>-57(2)</div><div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54.2(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-101(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(249,54,123)'>-72(1)</div><div class="text-center text-white" style='background-color: rgb(246,66,127)'>-66.5(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-102(1)</div> </td>
 <tr>
  <td>h14</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,136,148)'>-28(2)</div><div class="text-center text-white" style='background-color: rgb(223,122,144)'>-36(1)</div><div class="text-center text-white" style='background-color: rgb(248,56,123)'>-72(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-25(2)</div><div class="text-center text-white" style='background-color: rgb(222,125,145)'>-35(1)</div><div class="text-center text-white" style='background-color: rgb(248,58,124)'>-70(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(2)</div><div class="text-center text-white" style='background-color: rgb(220,127,146)'>-33(1)</div><div class="text-center text-white" style='background-color: rgb(247,63,126)'>-68(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,155,151)'>-17(2)</div><div class="text-center text-white" style='background-color: rgb(219,130,146)'>-32(1)</div><div class="text-center text-white" style='background-color: rgb(245,69,128)'>-65(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center text-white" style='background-color: rgb(217,132,147)'>-30(1)</div><div class="text-center text-white" style='background-color: rgb(242,77,131)'>-61(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,101,139)'>-48(2)</div><div class="text-center text-white" style='background-color: rgb(219,130,146)'>-32(1)</div><div class="text-center text-white" style='background-color: rgb(246,66,127)'>-67(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,96,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(224,121,144)'>-36.9(9)</div><div class="text-center text-white" style='background-color: rgb(250,47,120)'>-76(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,90,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(230,108,141)'>-43.8(9)</div><div class="text-center text-white" style='background-color: rgb(253,25,112)'>-87(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,83,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51.0(9)</div><div class="text-center text-white" style='background-color: rgb(254,11,106)'>-94(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(244,72,129)'>-63.3(8)</div><div class="text-center text-white" style='background-color: rgb(254,4,103)'>-98(2)</div> </td>
 <tr>
  <td>h13</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(216,135,147)'>-29(1)</div><div class="text-center text-white" style='background-color: rgb(218,131,147)'>-30.9(5)</div><div class="text-center text-white" style='background-color: rgb(243,76,131)'>-61(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-25(2)</div><div class="text-center text-white" style='background-color: rgb(216,134,147)'>-29(1)</div><div class="text-center text-white" style='background-color: rgb(241,82,133)'>-58(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(2)</div><div class="text-center text-white" style='background-color: rgb(214,137,148)'>-27(1)</div><div class="text-center text-white" style='background-color: rgb(239,88,135)'>-55(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(201,155,151)'>-16(2)</div><div class="text-center text-white" style='background-color: rgb(212,140,148)'>-26(1)</div><div class="text-center text-white" style='background-color: rgb(237,93,136)'>-52(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center text-white" style='background-color: rgb(211,143,149)'>-24(1)</div><div class="text-center text-white" style='background-color: rgb(233,101,139)'>-48(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,101,139)'>-48(2)</div><div class="text-center text-white" style='background-color: rgb(213,139,148)'>-26(1)</div><div class="text-center text-white" style='background-color: rgb(241,81,132)'>-58(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(219,129,146)'>-32(1)</div><div class="text-center text-white" style='background-color: rgb(247,60,125)'>-70(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,90,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(226,117,143)'>-39.0(9)</div><div class="text-center text-white" style='background-color: rgb(252,36,116)'>-82(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,82,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(233,103,139)'>-46.8(9)</div><div class="text-center text-white" style='background-color: rgb(253,23,111)'>-88(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(249,54,123)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(242,78,131)'>-60.4(8)</div><div class="text-center text-white" style='background-color: rgb(254,13,107)'>-93(2)</div> </td>
 <tr>
  <td>h12</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(216,134,147)'>-29(2)</div><div class="text-center text-white" style='background-color: rgb(212,140,148)'>-26(1)</div><div class="text-center text-white" style='background-color: rgb(235,98,138)'>-49(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-25(1)</div><div class="text-center text-white" style='background-color: rgb(210,144,149)'>-23.3(5)</div><div class="text-center text-white" style='background-color: rgb(233,102,139)'>-47(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(206,148,150)'>-20.6(5)</div><div class="text-center text-white" style='background-color: rgb(208,147,150)'>-21.5(3)</div><div class="text-center text-white" style='background-color: rgb(229,110,141)'>-42.8(5)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,155,151)'>-16(2)</div><div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(1)</div><div class="text-center text-white" style='background-color: rgb(225,118,143)'>-38(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center text-white" style='background-color: rgb(203,154,151)'>-17(1)</div><div class="text-center text-white" style='background-color: rgb(220,127,146)'>-33(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,102,139)'>-47(2)</div><div class="text-center text-white" style='background-color: rgb(208,146,150)'>-22(1)</div><div class="text-center text-white" style='background-color: rgb(235,97,137)'>-50(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(214,138,148)'>-27(1)</div><div class="text-center text-white" style='background-color: rgb(243,76,131)'>-61(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(221,125,145)'>-34.4(9)</div><div class="text-center text-white" style='background-color: rgb(249,52,122)'>-74(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,82,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(229,110,141)'>-42.7(9)</div><div class="text-center text-white" style='background-color: rgb(252,36,116)'>-81(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(249,54,123)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(240,84,134)'>-56.8(8)</div><div class="text-center text-white" style='background-color: rgb(254,21,110)'>-89(2)</div> </td>
 <tr>
  <td>h11</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(216,135,147)'>-29(2)</div><div class="text-center " style='background-color: rgb(144,210,149)'>+23(1)</div><div class="text-center " style='background-color: rgb(102,233,139)'>+47(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-25(2)</div><div class="text-center " style='background-color: rgb(140,213,148)'>+26(1)</div><div class="text-center " style='background-color: rgb(93,237,136)'>+52(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(2)</div><div class="text-center " style='background-color: rgb(135,215,147)'>+29(1)</div><div class="text-center " style='background-color: rgb(84,240,133)'>+57(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,155,151)'>-17(2)</div><div class="text-center " style='background-color: rgb(132,217,147)'>+31(1)</div><div class="text-center " style='background-color: rgb(74,243,130)'>+62(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,163,152)'>-12(2)</div><div class="text-center " style='background-color: rgb(128,220,146)'>+33(1)</div><div class="text-center " style='background-color: rgb(66,246,127)'>+66(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,101,139)'>-48(2)</div><div class="text-center " style='background-color: rgb(133,217,147)'>+30(1)</div><div class="text-center " style='background-color: rgb(105,232,140)'>+46(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(237,93,136)'>-52(2)</div><div class="text-center " style='background-color: rgb(143,210,149)'>+24(1)</div><div class="text-center " style='background-color: rgb(122,223,145)'>+36(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center " style='background-color: rgb(156,201,151)'>+16(1)</div><div class="text-center " style='background-color: rgb(145,209,149)'>+23(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,82,133)'>-58(2)</div><div class="text-center " style='background-color: rgb(175,184,152)'>+3(1)</div><div class="text-center " style='background-color: rgb(167,192,152)'>+9(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72.22(1)</div><div class="text-center text-white" style='background-color: rgb(210,143,149)'>-23.66(1)</div><div class="text-center text-white" style='background-color: rgb(210,143,149)'>-23.64(1)</div> </td>
 <tr>
  <td>h10</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,136,148)'>-28(2)</div><div class="text-center " style='background-color: rgb(152,204,150)'>+18(1)</div><div class="text-center " style='background-color: rgb(122,223,144)'>+36(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-24(2)</div><div class="text-center " style='background-color: rgb(148,207,150)'>+21(1)</div><div class="text-center " style='background-color: rgb(114,227,142)'>+41(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(2)</div><div class="text-center " style='background-color: rgb(145,209,149)'>+23(1)</div><div class="text-center " style='background-color: rgb(103,232,139)'>+47(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(201,156,151)'>-16(2)</div><div class="text-center " style='background-color: rgb(140,212,148)'>+26(1)</div><div class="text-center " style='background-color: rgb(97,235,138)'>+50(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center " style='background-color: rgb(136,215,148)'>+28(1)</div><div class="text-center " style='background-color: rgb(85,240,134)'>+57(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,101,139)'>-48(2)</div><div class="text-center " style='background-color: rgb(139,213,148)'>+26(1)</div><div class="text-center " style='background-color: rgb(117,226,143)'>+39(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,96,137)'>-51(2)</div><div class="text-center " style='background-color: rgb(150,205,150)'>+20(1)</div><div class="text-center " style='background-color: rgb(137,215,148)'>+28(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(239,88,135)'>-55(2)</div><div class="text-center " style='background-color: rgb(163,195,152)'>+11(1)</div><div class="text-center " style='background-color: rgb(157,200,151)'>+15(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,82,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(187,172,152)'>-5(1)</div><div class="text-center text-white" style='background-color: rgb(192,167,152)'>-9(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(215,136,148)'>-28(1)</div><div class="text-center text-white" style='background-color: rgb(221,126,145)'>-34(2)</div> </td>
 <tr>
  <td>h9</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,135,147)'>-29(1)</div><div class="text-center " style='background-color: rgb(170,190,152)'>+7.1(5)</div><div class="text-center " style='background-color: rgb(172,187,152)'>+5(1)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-25(1)</div><div class="text-center " style='background-color: rgb(165,193,152)'>+10.0(5)</div><div class="text-center " style='background-color: rgb(162,196,152)'>+12(1)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(2)</div><div class="text-center " style='background-color: rgb(161,197,151)'>+13(1)</div><div class="text-center " style='background-color: rgb(152,204,150)'>+18(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(201,155,151)'>-16(2)</div><div class="text-center " style='background-color: rgb(156,201,151)'>+16(1)</div><div class="text-center " style='background-color: rgb(143,210,149)'>+24(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(197,161,151)'>-13(2)</div><div class="text-center " style='background-color: rgb(152,204,150)'>+18(1)</div><div class="text-center " style='background-color: rgb(132,217,147)'>+31(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(234,101,139)'>-48(2)</div><div class="text-center " style='background-color: rgb(154,202,151)'>+17(1)</div><div class="text-center " style='background-color: rgb(164,194,152)'>+11(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center " style='background-color: rgb(164,194,152)'>+10(1)</div><div class="text-center text-white" style='background-color: rgb(182,177,152)'>-2(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-55(2)</div><div class="text-center text-white" style='background-color: rgb(187,173,152)'>-5(1)</div><div class="text-center text-white" style='background-color: rgb(217,133,147)'>-30(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,84,133)'>-57(2)</div><div class="text-center text-white" style='background-color: rgb(208,146,149)'>-22(1)</div><div class="text-center text-white" style='background-color: rgb(236,96,137)'>-51(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(227,116,143)'>-39.8(9)</div><div class="text-center text-white" style='background-color: rgb(243,74,130)'>-62(2)</div> </td>
 <tr>
  <td>h8</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,135,147)'>-29(2)</div><div class="text-center text-white" style='background-color: rgb(183,177,152)'>-2(1)</div><div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,143,149)'>-24(2)</div><div class="text-center " style='background-color: rgb(179,181,152)'>+1(1)</div><div class="text-center text-white" style='background-color: rgb(197,160,151)'>-13(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(2)</div><div class="text-center " style='background-color: rgb(174,185,152)'>+4(1)</div><div class="text-center text-white" style='background-color: rgb(187,172,152)'>-5(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(201,156,151)'>-16(2)</div><div class="text-center " style='background-color: rgb(169,190,152)'>+7(1)</div><div class="text-center text-white" style='background-color: rgb(181,179,152)'>-1(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(1)</div><div class="text-center " style='background-color: rgb(165,194,152)'>+10.2(5)</div><div class="text-center " style='background-color: rgb(168,191,152)'>+8(1)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,103,139)'>-47(2)</div><div class="text-center " style='background-color: rgb(167,192,152)'>+9(1)</div><div class="text-center text-white" style='background-color: rgb(205,150,150)'>-20(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,96,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(188,171,152)'>-6(1)</div><div class="text-center text-white" style='background-color: rgb(232,105,140)'>-46(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(207,147,150)'>-21(1)</div><div class="text-center text-white" style='background-color: rgb(248,56,123)'>-72(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,83,133)'>-57(2)</div><div class="text-center text-white" style='background-color: rgb(218,131,147)'>-31(1)</div><div class="text-center text-white" style='background-color: rgb(250,47,120)'>-76(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,56,123)'>-72(2)</div><div class="text-center text-white" style='background-color: rgb(234,99,138)'>-49.2(9)</div><div class="text-center text-white" style='background-color: rgb(253,22,110)'>-89(2)</div> </td>
 <tr>
  <td>h7</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(216,134,147)'>-29(2)</div><div class="text-center text-white" style='background-color: rgb(194,164,152)'>-10(1)</div><div class="text-center text-white" style='background-color: rgb(230,109,141)'>-43(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(210,143,149)'>-24(2)</div><div class="text-center text-white" style='background-color: rgb(190,169,152)'>-8(1)</div><div class="text-center text-white" style='background-color: rgb(223,123,145)'>-36(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(207,148,150)'>-21(2)</div><div class="text-center text-white" style='background-color: rgb(185,174,152)'>-4(1)</div><div class="text-center text-white" style='background-color: rgb(213,139,148)'>-26(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,155,151)'>-16(2)</div><div class="text-center " style='background-color: rgb(180,180,152)'>+0(1)</div><div class="text-center text-white" style='background-color: rgb(206,148,150)'>-21(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(197,161,152)'>-13(2)</div><div class="text-center " style='background-color: rgb(176,184,152)'>+3(1)</div><div class="text-center text-white" style='background-color: rgb(193,165,152)'>-10(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(234,100,138)'>-48(2)</div><div class="text-center text-white" style='background-color: rgb(189,170,152)'>-7(1)</div><div class="text-center text-white" style='background-color: rgb(242,80,132)'>-59(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(207,147,150)'>-21(1)</div><div class="text-center text-white" style='background-color: rgb(253,30,114)'>-85(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(239,88,135)'>-55(2)</div><div class="text-center text-white" style='background-color: rgb(216,135,147)'>-29(1)</div><div class="text-center text-white" style='background-color: rgb(254,10,106)'>-95(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,83,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(224,121,144)'>-37.0(9)</div><div class="text-center text-white" style='background-color: rgb(254,7,104)'>-96(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(249,54,123)'>-72(1)</div><div class="text-center text-white" style='background-color: rgb(239,88,135)'>-55.1(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-107(1)</div> </td>
 <tr>
  <td>h6</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,135,147)'>-29(2)</div><div class="text-center text-white" style='background-color: rgb(200,157,151)'>-15(1)</div><div class="text-center text-white" style='background-color: rgb(238,89,135)'>-55(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-25(2)</div><div class="text-center text-white" style='background-color: rgb(195,164,152)'>-11(1)</div><div class="text-center text-white" style='background-color: rgb(233,102,139)'>-48(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(2)</div><div class="text-center text-white" style='background-color: rgb(191,167,152)'>-8(1)</div><div class="text-center text-white" style='background-color: rgb(226,117,143)'>-39(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(201,156,151)'>-16(2)</div><div class="text-center text-white" style='background-color: rgb(185,174,152)'>-4(1)</div><div class="text-center text-white" style='background-color: rgb(218,131,146)'>-31(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center text-white" style='background-color: rgb(181,179,152)'>-1(1)</div><div class="text-center text-white" style='background-color: rgb(208,147,150)'>-22(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,102,139)'>-48(2)</div><div class="text-center text-white" style='background-color: rgb(202,155,151)'>-17(1)</div><div class="text-center text-white" style='background-color: rgb(254,20,110)'>-90(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(211,142,149)'>-24(1)</div><div class="text-center text-white" style='background-color: rgb(254,0,102)'>-100(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(239,88,135)'>-55(2)</div><div class="text-center text-white" style='background-color: rgb(218,130,146)'>-31(1)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-106(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(240,83,133)'>-57(2)</div><div class="text-center text-white" style='background-color: rgb(228,113,142)'>-41.1(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-105(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(249,54,123)'>-73(1)</div><div class="text-center text-white" style='background-color: rgb(240,85,134)'>-56.2(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-113(1)</div> </td>
 <tr>
  <td>h5</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(216,134,147)'>-29(2)</div><div class="text-center text-white" style='background-color: rgb(199,158,151)'>-15(1)</div><div class="text-center text-white" style='background-color: rgb(239,86,134)'>-56(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-24(2)</div><div class="text-center text-white" style='background-color: rgb(195,163,152)'>-11(1)</div><div class="text-center text-white" style='background-color: rgb(236,96,137)'>-51(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(2)</div><div class="text-center text-white" style='background-color: rgb(190,168,152)'>-8(1)</div><div class="text-center text-white" style='background-color: rgb(227,114,143)'>-40(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(201,155,151)'>-16(2)</div><div class="text-center text-white" style='background-color: rgb(185,174,152)'>-4(1)</div><div class="text-center text-white" style='background-color: rgb(220,127,146)'>-33(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center text-white" style='background-color: rgb(180,179,152)'>-0(1)</div><div class="text-center text-white" style='background-color: rgb(212,141,149)'>-25(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,102,139)'>-48(2)</div><div class="text-center text-white" style='background-color: rgb(200,157,151)'>-15(1)</div><div class="text-center text-white" style='background-color: rgb(254,8,105)'>-96(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,95,137)'>-51(2)</div><div class="text-center text-white" style='background-color: rgb(209,145,149)'>-23(1)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-102(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,90,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(216,134,147)'>-29(1)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-109(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,82,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(226,117,143)'>-39.0(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-108(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72(1)</div><div class="text-center text-white" style='background-color: rgb(239,88,135)'>-54.9(8)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-112(1)</div> </td>
 <tr>
  <td>h4</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
 <!-- 2 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(215,135,147)'>-29(2)</div><div class="text-center text-white" style='background-color: rgb(199,159,151)'>-14(1)</div><div class="text-center text-white" style='background-color: rgb(240,84,133)'>-57(2)</div> </td>
 <!-- 3 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(211,142,149)'>-25(2)</div><div class="text-center text-white" style='background-color: rgb(194,164,152)'>-11(1)</div><div class="text-center text-white" style='background-color: rgb(235,98,138)'>-50(2)</div> </td>
 <!-- 4 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(206,149,150)'>-20(2)</div><div class="text-center text-white" style='background-color: rgb(189,170,152)'>-7(1)</div><div class="text-center text-white" style='background-color: rgb(228,113,142)'>-42(2)</div> </td>
 <!-- 5 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(202,155,151)'>-17(2)</div><div class="text-center text-white" style='background-color: rgb(185,175,152)'>-4(1)</div><div class="text-center text-white" style='background-color: rgb(219,129,146)'>-32(2)</div> </td>
 <!-- 6 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(196,162,152)'>-12(2)</div><div class="text-center " style='background-color: rgb(178,182,152)'>+2(1)</div><div class="text-center text-white" style='background-color: rgb(211,142,149)'>-24(2)</div> </td>
 <!-- 7 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(233,101,139)'>-48(2)</div><div class="text-center text-white" style='background-color: rgb(198,160,151)'>-13(1)</div><div class="text-center text-white" style='background-color: rgb(254,9,105)'>-95(2)</div> </td>
 <!-- 8 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(236,94,137)'>-52(2)</div><div class="text-center text-white" style='background-color: rgb(205,150,150)'>-20(1)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-102(2)</div> </td>
 <!-- 9 -->
 <td>
<div class="text-center text-white" style='background-color: rgb(238,89,135)'>-54(2)</div><div class="text-center text-white" style='background-color: rgb(215,137,148)'>-28(1)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-108(2)</div> </td>
 <!-- T -->
 <td>
<div class="text-center text-white" style='background-color: rgb(241,82,133)'>-58(2)</div><div class="text-center text-white" style='background-color: rgb(224,120,144)'>-37.1(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-107(2)</div> </td>
 <!-- A -->
 <td>
<div class="text-center text-white" style='background-color: rgb(248,55,123)'>-72(1)</div><div class="text-center text-white" style='background-color: rgb(238,91,136)'>-53.5(9)</div><div class="text-center text-white" style='background-color: rgb(255,0,102)'>-114(1)</div> </td>

 </tbody>
</table>
```
|  Hand \| $n$ \| Stand \[%\] \| Double \[%\] \| Hit \[%\] \| Play \|
| h20-2 \| 8.0e+04 \| +63.23 (1.1) \| -85.32 (0.5) \| -171.17 (1.1) \|
  stand \|
| h20-3 \| 8.0e+04 \| +64.54 (1.1) \| -85.50 (0.5) \| -171.50 (1.1) \|
  stand \|
| h20-4 \| 8.0e+04 \| +65.55 (1.1) \| -85.50 (0.5) \| -170.33 (1.1) \|
  stand \|
| h20-5 \| 8.0e+04 \| +66.65 (1.1) \| -85.51 (0.5) \| -171.25 (1.1) \|
  stand \|
| h20-6 \| 8.0e+04 \| +67.80 (1.1) \| -85.59 (0.5) \| -171.07 (1.1) \|
  stand \|
| h20-7 \| 8.0e+04 \| +77.44 (1.1) \| -85.44 (0.5) \| -170.53 (1.1) \|
  stand \|
| h20-8 \| 8.0e+04 \| +79.11 (1.1) \| -85.02 (0.6) \| -170.08 (1.1) \|
  stand \|
| h20-9 \| 8.0e+04 \| +75.77 (1.1) \| -84.87 (0.6) \| -170.31 (1.1) \|
  stand \|
| h20-T \| 8.0e+04 \| +43.47 (1.1) \| -85.86 (0.5) \| -164.14 (1.1) \|
  stand \|
| h20-A \| 8.0e+04 \| +10.32 (1.0) \| -89.73 (0.5) \| -149.01 (1.0) \|
  stand \|
| h19-2 \| 8.0e+04 \| +37.61 (1.4) \| -73.52 (0.7) \| -146.58 (1.4) \|
  stand \|
| h19-3 \| 8.0e+04 \| +39.34 (1.4) \| -72.58 (0.7) \| -145.41 (1.4) \|
  stand \|
| h19-4 \| 8.0e+04 \| +41.60 (1.4) \| -72.88 (0.7) \| -145.28 (1.4) \|
  stand \|
| h19-5 \| 8.0e+04 \| +43.51 (1.4) \| -72.98 (0.7) \| -145.12 (1.4) \|
  stand \|
| h19-6 \| 8.0e+04 \| +45.41 (1.4) \| -72.56 (0.7) \| -144.94 (1.4) \|
  stand \|
| h19-7 \| 8.0e+04 \| +61.77 (1.5) \| -71.07 (0.7) \| -142.94 (1.5) \|
  stand \|
| h19-8 \| 8.0e+04 \| +59.05 (1.5) \| -71.22 (0.7) \| -143.04 (1.5) \|
  stand \|
| h19-9 \| 8.0e+04 \| +28.52 (1.5) \| -71.93 (0.7) \| -143.09 (1.5) \|
  stand \|
| h19-T \| 8.0e+04 \| -1.96 (1.4) \| -75.09 (0.7) \| -142.54 (1.4) \|
  stand \|
| h19-A \| 8.0e+04 \| -17.95 (1.2) \| -80.99 (0.6) \| -131.78 (1.2) \|
  stand \|
| h18-2 \| 8.0e+04 \| +10.97 (1.6) \| -61.93 (0.8) \| -124.23 (1.6) \|
  stand \|
| h18-3 \| 8.0e+04 \| +13.15 (1.6) \| -62.50 (0.8) \| -124.98 (1.6) \|
  stand \|
| h18-4 \| 8.0e+04 \| +16.93 (1.6) \| -61.94 (0.8) \| -123.60 (1.6) \|
  stand \|
| h18-5 \| 8.0e+04 \| +19.25 (1.6) \| -62.22 (0.8) \| -123.74 (1.6) \|
  stand \|
| h18-6 \| 8.0e+04 \| +22.23 (1.6) \| -61.47 (0.8) \| -122.43 (1.6) \|
  stand \|
| h18-7 \| 8.0e+04 \| +39.60 (1.7) \| -58.93 (0.8) \| -118.77 (1.7) \|
  stand \|
| h18-8 \| 8.0e+04 \| +11.02 (1.7) \| -59.17 (0.8) \| -118.63 (1.7) \|
  stand \|
| h18-9 \| 8.0e+04 \| -18.50 (1.6) \| -61.67 (0.8) \| -122.96 (1.6) \|
  stand \|
| h18-T \| 8.0e+04 \| -24.23 (1.5) \| -67.40 (0.8) \| -126.70 (1.5) \|
  stand \|
| h18-A \| 8.0e+04 \| -46.34 (1.3) \| -74.66 (0.7) \| -118.34 (1.3) \|
  stand \|
| h17-2 \| 8.0e+04 \| -16.04 (1.7) \| -53.84 (0.9) \| -107.43 (1.7) \|
  stand \|
| h17-3 \| 8.0e+04 \| -11.84 (1.7) \| -53.28 (0.9) \| -107.23 (1.7) \|
  stand \|
| h17-4 \| 8.0e+04 \| -7.85 (1.8) \| -52.66 (0.9) \| -105.90 (1.8) \|
  stand \|
| h17-5 \| 8.0e+04 \| -4.57 (1.8) \| -52.56 (0.9) \| -103.96 (1.8) \|
  stand \|
| h17-6 \| 8.0e+04 \| -0.04 (1.8) \| -51.63 (0.9) \| -104.55 (1.8) \|
  stand \|
| h17-7 \| 8.0e+04 \| -10.94 (1.8) \| -48.70 (0.9) \| -96.41 (1.8) \|
  stand \|
| h17-8 \| 8.0e+04 \| -37.84 (1.8) \| -50.88 (0.9) \| -101.70 (1.8) \|
  stand \|
| h17-9 \| 8.0e+04 \| -42.50 (1.7) \| -55.60 (0.8) \| -109.70 (1.7) \|
  stand \|
| h17-T \| 8.0e+04 \| -46.71 (1.6) \| -61.73 (0.8) \| -116.13 (1.6) \|
  stand \|
| h17-A \| 8.0e+04 \| -66.37 (1.4) \| -70.97 (0.7) \| -111.14 (1.4) \|
  stand \|
| h16-2 \| 8.0e+04 \| -28.77 (1.8) \| -47.30 (0.9) \| -95.43 (1.8) \|
  stand \|
| h16-3 \| 8.0e+04 \| -24.29 (1.8) \| -46.54 (0.9) \| -92.83 (1.8) \|
  stand \|
| h16-4 \| 8.0e+04 \| -20.04 (1.8) \| -46.18 (0.9) \| -91.87 (1.8) \|
  stand \|
| h16-5 \| 8.0e+04 \| -16.86 (1.8) \| -45.19 (0.9) \| -89.29 (1.8) \|
  stand \|
| h16-6 \| 8.0e+04 \| -11.67 (1.9) \| -44.23 (0.9) \| -88.78 (1.9) \|
  stand \|
| h16-7 \| 8.0e+04 \| -47.23 (1.9) \| -41.86 (0.9) \| -83.70 (1.9) \|
  hit \|
| h16-8 \| 8.0e+04 \| -51.20 (1.8) \| -46.31 (0.9) \| -91.59 (1.8) \|
  hit \|
| h16-9 \| 8.0e+04 \| -54.18 (1.7) \| -51.19 (0.9) \| -102.23 (1.7) \|
  hit \|
| h16-T \| 2.0e+07 \| -57.57 (0) \| -57.55 (0) \| -107.33 (0) \| hit \|
| h16-A \| 8.0e+04 \| -72.19 (1.4) \| -68.30 (0.7) \| -105.39 (1.4) \|
  hit \|
| h15-2 \| 8.0e+04 \| -28.22 (1.9) \| -41.63 (0.9) \| -82.97 (1.9) \|
  stand \|
| h15-3 \| 8.0e+04 \| -24.40 (1.9) \| -41.04 (0.9) \| -80.75 (1.9) \|
  stand \|
| h15-4 \| 8.0e+04 \| -20.61 (1.9) \| -39.80 (0.9) \| -79.58 (1.9) \|
  stand \|
| h15-5 \| 8.0e+04 \| -16.64 (1.9) \| -38.18 (1.0) \| -76.95 (1.9) \|
  stand \|
| h15-6 \| 8.0e+04 \| -12.54 (1.9) \| -37.21 (1.0) \| -75.91 (1.9) \|
  stand \|
| h15-7 \| 8.0e+04 \| -47.73 (1.9) \| -37.35 (0.9) \| -73.94 (1.9) \|
  hit \|
| h15-8 \| 8.0e+04 \| -50.75 (1.9) \| -41.05 (0.9) \| -84.62 (1.9) \|
  hit \|
| h15-9 \| 8.0e+04 \| -54.44 (1.8) \| -47.05 (0.9) \| -95.42 (1.8) \|
  hit \|
| h15-T \| 8.0e+04 \| -57.26 (1.7) \| -54.23 (0.9) \| -100.92 (1.7) \|
  hit \|
| h15-A \| 8.0e+04 \| -72.42 (1.4) \| -66.47 (0.8) \| -101.91 (1.4) \|
  hit \|
| h14-2 \| 8.0e+04 \| -28.04 (1.9) \| -36.34 (1.0) \| -71.61 (1.9) \|
  stand \|
| h14-3 \| 8.0e+04 \| -24.76 (1.9) \| -34.74 (1.0) \| -70.47 (1.9) \|
  stand \|
| h14-4 \| 8.0e+04 \| -20.69 (1.9) \| -33.14 (1.0) \| -68.04 (1.9) \|
  stand \|
| h14-5 \| 8.0e+04 \| -16.68 (2.0) \| -31.75 (1.0) \| -64.69 (2.0) \|
  stand \|
| h14-6 \| 8.0e+04 \| -12.21 (2.0) \| -30.33 (1.0) \| -60.55 (2.0) \|
  stand \|
| h14-7 \| 8.0e+04 \| -47.78 (1.9) \| -31.73 (1.0) \| -66.56 (1.9) \|
  hit \|
| h14-8 \| 8.0e+04 \| -50.59 (1.9) \| -36.85 (0.9) \| -75.91 (1.9) \|
  hit \|
| h14-9 \| 8.0e+04 \| -54.02 (1.8) \| -43.78 (0.9) \| -87.29 (1.8) \|
  hit \|
| h14-T \| 8.0e+04 \| -57.53 (1.7) \| -50.98 (0.9) \| -94.12 (1.7) \|
  hit \|
| h14-A \| 8.0e+04 \| -71.95 (1.5) \| -63.33 (0.8) \| -97.81 (1.5) \|
  hit \|
| h13-2 \| 3.2e+05 \| -28.71 (1.0) \| -30.93 (0.5) \| -61.09 (1.0) \|
  stand \|
| h13-3 \| 8.0e+04 \| -24.71 (2.0) \| -29.25 (1.0) \| -57.88 (2.0) \|
  stand \|
| h13-4 \| 8.0e+04 \| -20.47 (2.0) \| -27.32 (1.0) \| -55.06 (2.0) \|
  stand \|
| h13-5 \| 8.0e+04 \| -16.25 (2.0) \| -25.88 (1.0) \| -52.01 (2.0) \|
  stand \|
| h13-6 \| 8.0e+04 \| -11.78 (2.0) \| -24.12 (1.0) \| -47.83 (2.0) \|
  stand \|
| h13-7 \| 8.0e+04 \| -47.69 (2.0) \| -26.36 (1.0) \| -58.47 (2.0) \|
  hit \|
| h13-8 \| 8.0e+04 \| -51.19 (1.9) \| -32.25 (1.0) \| -69.55 (1.9) \|
  hit \|
| h13-9 \| 8.0e+04 \| -54.03 (1.9) \| -38.96 (0.9) \| -81.88 (1.9) \|
  hit \|
| h13-T \| 8.0e+04 \| -57.83 (1.8) \| -46.80 (0.9) \| -88.48 (1.8) \|
  hit \|
| h13-A \| 8.0e+04 \| -72.42 (1.5) \| -60.37 (0.8) \| -93.03 (1.5) \|
  hit \|
| h12-2 \| 8.0e+04 \| -29.07 (2.0) \| -25.61 (1.0) \| -49.41 (2.0) \|
  hit \|
| h12-3 \| 3.2e+05 \| -24.62 (1.0) \| -23.31 (0.5) \| -47.37 (1.0) \|
  hit \|
| h12-4 \| 1.3e+06 \| -20.56 (0.5) \| -21.52 (0.3) \| -42.79 (0.5) \|
  stand \|
| h12-5 \| 8.0e+04 \| -16.46 (2.0) \| -19.95 (1.0) \| -38.43 (2.0) \|
  stand \|
| h12-6 \| 8.0e+04 \| -12.05 (2.0) \| -17.28 (1.0) \| -33.47 (2.0) \|
  stand \|
| h12-7 \| 8.0e+04 \| -47.30 (2.0) \| -21.83 (1.0) \| -50.18 (2.0) \|
  hit \|
| h12-8 \| 8.0e+04 \| -51.08 (2.0) \| -27.00 (1.0) \| -61.37 (2.0) \|
  hit \|
| h12-9 \| 8.0e+04 \| -54.15 (1.9) \| -34.39 (0.9) \| -73.59 (1.9) \|
  hit \|
| h12-T \| 8.0e+04 \| -57.87 (1.8) \| -42.68 (0.9) \| -81.48 (1.8) \|
  hit \|
| h12-A \| 8.0e+04 \| -72.38 (1.5) \| -56.80 (0.8) \| -89.31 (1.5) \|
  hit \|
| h11-2 \| 8.0e+04 \| -28.87 (2.0) \| +23.35 (1.0) \| +47.39 (2.0) \|
  double \|
| h11-3 \| 8.0e+04 \| -24.76 (2.0) \| +25.98 (1.0) \| +52.29 (2.0) \|
  double \|
| h11-4 \| 8.0e+04 \| -20.67 (2.0) \| +28.62 (1.0) \| +57.21 (2.0) \|
  double \|
| h11-5 \| 8.0e+04 \| -16.73 (1.9) \| +30.55 (1.0) \| +62.32 (1.9) \|
  double \|
| h11-6 \| 8.0e+04 \| -11.69 (1.9) \| +33.02 (1.0) \| +66.25 (1.9) \|
  double \|
| h11-7 \| 8.0e+04 \| -47.69 (2.0) \| +29.68 (1.0) \| +45.78 (2.0) \|
  double \|
| h11-8 \| 8.0e+04 \| -52.13 (2.0) \| +23.74 (1.0) \| +35.93 (2.0) \|
  double \|
| h11-9 \| 8.0e+04 \| -54.25 (2.0) \| +16.06 (1.0) \| +22.66 (2.0) \|
  double \|
| h11-T \| 8.0e+04 \| -57.92 (2.0) \| +3.19 (1.0) \| +9.06 (2.0) \|
  double \|
| h11-A \| 8.2e+07 \| -72.22 (0) \| -23.66 (0) \| -23.64 (0) \| double
  \|
| h10-2 \| 8.0e+04 \| -28.12 (2.0) \| +18.19 (1.0) \| +36.23 (2.0) \|
  double \|
| h10-3 \| 8.0e+04 \| -24.40 (2.0) \| +20.90 (1.0) \| +40.69 (2.0) \|
  double \|
| h10-4 \| 8.0e+04 \| -20.09 (2.0) \| +22.58 (1.0) \| +46.54 (2.0) \|
  double \|
| h10-5 \| 8.0e+04 \| -16.16 (2.0) \| +25.57 (1.0) \| +49.92 (2.0) \|
  double \|
| h10-6 \| 8.0e+04 \| -12.23 (2.0) \| +28.35 (1.0) \| +56.55 (2.0) \|
  double \|
| h10-7 \| 8.0e+04 \| -47.69 (2.0) \| +26.37 (1.0) \| +39.07 (2.0) \|
  double \|
| h10-8 \| 8.0e+04 \| -50.60 (2.0) \| +19.50 (1.0) \| +27.72 (2.0) \|
  double \|
| h10-9 \| 8.0e+04 \| -54.75 (2.0) \| +11.43 (1.0) \| +15.10 (2.0) \|
  double \|
| h10-T \| 8.0e+04 \| -57.90 (1.9) \| -5.37 (1.0) \| -9.06 (1.9) \| hit
  \|
| h10-A \| 8.0e+04 \| -71.92 (1.7) \| -28.02 (1.0) \| -34.12 (1.7) \|
  hit \|
| h9-2 \| 3.2e+05 \| -28.52 (1.0) \| +7.07 (0.5) \| +5.21 (1.0) \| hit
  \|
| h9-3 \| 3.2e+05 \| -24.80 (1.0) \| +9.97 (0.5) \| +11.98 (1.0) \|
  double \|
| h9-4 \| 8.0e+04 \| -20.79 (2.0) \| +12.80 (1.0) \| +18.47 (2.0) \|
  double \|
| h9-5 \| 8.0e+04 \| -16.32 (2.0) \| +15.69 (1.0) \| +23.70 (2.0) \|
  double \|
| h9-6 \| 8.0e+04 \| -12.80 (2.0) \| +18.20 (1.0) \| +30.52 (2.0) \|
  double \|
| h9-7 \| 8.0e+04 \| -48.00 (2.0) \| +17.03 (1.0) \| +10.60 (2.0) \| hit
  \|
| h9-8 \| 8.0e+04 \| -51.36 (2.0) \| +10.40 (1.0) \| -1.84 (2.0) \| hit
  \|
| h9-9 \| 8.0e+04 \| -54.51 (1.9) \| -4.82 (1.0) \| -29.67 (1.9) \| hit
  \|
| h9-T \| 8.0e+04 \| -56.87 (1.9) \| -22.03 (1.0) \| -50.65 (1.9) \| hit
  \|
| h9-A \| 8.0e+04 \| -72.07 (1.6) \| -39.82 (0.9) \| -62.20 (1.6) \| hit
  \|
| h8-2 \| 8.0e+04 \| -28.53 (2.0) \| -2.26 (1.0) \| -20.42 (2.0) \| hit
  \|
| h8-3 \| 8.0e+04 \| -24.15 (2.0) \| +0.65 (1.0) \| -13.16 (2.0) \| hit
  \|
| h8-4 \| 8.0e+04 \| -20.23 (2.1) \| +3.90 (1.0) \| -5.21 (2.1) \| hit
  \|
| h8-5 \| 8.0e+04 \| -16.14 (2.1) \| +7.09 (1.0) \| -0.54 (2.1) \| hit
  \|
| h8-6 \| 3.2e+05 \| -12.14 (1.0) \| +10.20 (0.5) \| +7.84 (1.0) \| hit
  \|
| h8-7 \| 8.0e+04 \| -47.05 (2.0) \| +8.66 (1.0) \| -19.70 (2.0) \| hit
  \|
| h8-8 \| 8.0e+04 \| -50.79 (1.9) \| -5.83 (1.0) \| -45.53 (1.9) \| hit
  \|
| h8-9 \| 8.0e+04 \| -54.13 (1.9) \| -21.18 (1.0) \| -71.69 (1.9) \| hit
  \|
| h8-T \| 8.0e+04 \| -57.28 (1.8) \| -30.79 (1.0) \| -76.30 (1.8) \| hit
  \|
| h8-A \| 8.0e+04 \| -71.71 (1.5) \| -49.20 (0.9) \| -88.72 (1.5) \| hit
  \|
| h7-2 \| 8.0e+04 \| -28.98 (2.0) \| -10.50 (1.0) \| -43.26 (2.0) \| hit
  \|
| h7-3 \| 8.0e+04 \| -24.01 (2.0) \| -7.75 (1.0) \| -35.53 (2.0) \| hit
  \|
| h7-4 \| 8.0e+04 \| -20.99 (2.1) \| -3.92 (1.0) \| -26.25 (2.1) \| hit
  \|
| h7-5 \| 8.0e+04 \| -16.47 (2.1) \| +0.08 (1.0) \| -20.59 (2.1) \| hit
  \|
| h7-6 \| 8.0e+04 \| -12.61 (2.1) \| +2.82 (1.0) \| -10.05 (2.1) \| hit
  \|
| h7-7 \| 8.0e+04 \| -48.34 (1.9) \| -6.79 (1.0) \| -59.20 (1.9) \| hit
  \|
| h7-8 \| 8.0e+04 \| -51.35 (1.8) \| -21.21 (1.0) \| -84.58 (1.8) \| hit
  \|
| h7-9 \| 8.0e+04 \| -54.69 (1.8) \| -28.72 (1.0) \| -94.97 (1.8) \| hit
  \|
| h7-T \| 8.0e+04 \| -57.65 (1.7) \| -37.00 (0.9) \| -96.37 (1.7) \| hit
  \|
| h7-A \| 8.0e+04 \| -72.44 (1.4) \| -55.11 (0.8) \| -106.65 (1.4) \|
  hit \|
| h6-2 \| 8.0e+04 \| -28.57 (2.0) \| -15.43 (1.0) \| -54.59 (2.0) \| hit
  \|
| h6-3 \| 8.0e+04 \| -24.56 (2.0) \| -11.04 (1.0) \| -47.59 (2.0) \| hit
  \|
| h6-4 \| 8.0e+04 \| -20.00 (2.1) \| -8.47 (1.0) \| -38.90 (2.1) \| hit
  \|
| h6-5 \| 8.0e+04 \| -16.08 (2.1) \| -4.01 (1.0) \| -31.24 (2.1) \| hit
  \|
| h6-6 \| 8.0e+04 \| -11.99 (2.1) \| -0.87 (1.0) \| -21.55 (2.1) \| hit
  \|
| h6-7 \| 8.0e+04 \| -47.56 (1.9) \| -16.78 (1.0) \| -89.83 (1.9) \| hit
  \|
| h6-8 \| 8.0e+04 \| -51.12 (1.8) \| -24.33 (1.0) \| -99.55 (1.8) \| hit
  \|
| h6-9 \| 8.0e+04 \| -54.73 (1.8) \| -31.31 (1.0) \| -106.21 (1.8) \|
  hit \|
| h6-T \| 8.0e+04 \| -57.42 (1.7) \| -41.13 (0.9) \| -105.18 (1.7) \|
  hit \|
| h6-A \| 8.0e+04 \| -72.60 (1.4) \| -56.24 (0.8) \| -112.72 (1.4) \|
  hit \|
| h5-2 \| 8.0e+04 \| -29.05 (2.0) \| -14.61 (1.0) \| -56.03 (2.0) \| hit
  \|
| h5-3 \| 8.0e+04 \| -24.45 (2.1) \| -11.41 (1.0) \| -50.65 (2.1) \| hit
  \|
| h5-4 \| 8.0e+04 \| -20.23 (2.1) \| -7.78 (1.0) \| -40.43 (2.1) \| hit
  \|
| h5-5 \| 8.0e+04 \| -16.31 (2.1) \| -3.84 (1.0) \| -33.26 (2.1) \| hit
  \|
| h5-6 \| 8.0e+04 \| -11.79 (2.1) \| -0.37 (1.0) \| -25.05 (2.1) \| hit
  \|
| h5-7 \| 8.0e+04 \| -47.60 (1.9) \| -15.39 (1.0) \| -95.71 (1.9) \| hit
  \|
| h5-8 \| 8.0e+04 \| -51.23 (1.8) \| -22.53 (1.0) \| -101.98 (1.8) \|
  hit \|
| h5-9 \| 8.0e+04 \| -54.04 (1.8) \| -29.29 (1.0) \| -108.81 (1.8) \|
  hit \|
| h5-T \| 8.0e+04 \| -57.95 (1.7) \| -39.00 (0.9) \| -107.75 (1.7) \|
  hit \|
| h5-A \| 8.0e+04 \| -72.13 (1.4) \| -54.85 (0.8) \| -112.28 (1.4) \|
  hit \|
| h4-2 \| 8.0e+04 \| -28.50 (2.0) \| -14.23 (1.0) \| -56.99 (2.0) \| hit
  \|
| h4-3 \| 8.0e+04 \| -24.78 (2.1) \| -10.80 (1.0) \| -49.65 (2.1) \| hit
  \|
| h4-4 \| 8.0e+04 \| -20.49 (2.1) \| -6.87 (1.0) \| -41.52 (2.1) \| hit
  \|
| h4-5 \| 8.0e+04 \| -16.77 (2.1) \| -3.61 (1.0) \| -32.45 (2.1) \| hit
  \|
| h4-6 \| 8.0e+04 \| -11.80 (2.1) \| +1.55 (1.0) \| -24.32 (2.1) \| hit
  \|
| h4-7 \| 8.0e+04 \| -47.75 (1.9) \| -13.39 (1.0) \| -95.12 (1.9) \| hit
  \|
| h4-8 \| 8.0e+04 \| -51.73 (1.8) \| -19.55 (1.0) \| -101.77 (1.8) \|
  hit \|
| h4-9 \| 8.0e+04 \| -54.22 (1.8) \| -27.76 (1.0) \| -108.31 (1.8) \|
  hit \|
| h4-T \| 8.0e+04 \| -58.04 (1.7) \| -37.09 (0.9) \| -107.42 (1.7) \|
  hit \|
| h4-A \| 8.0e+04 \| -72.31 (1.4) \| -53.45 (0.9) \| -114.02 (1.4) \|
  hit \|
| s20-2 \| 8.0e+04 \| +63.62 (2.0) \| +18.06 (1.0) \| +35.23 (2.0) \|
  stand \|
| s20-3 \| 8.0e+04 \| +64.71 (2.0) \| +20.31 (1.0) \| +39.62 (2.0) \|
  stand \|
| s20-4 \| 8.0e+04 \| +65.86 (2.0) \| +23.05 (1.0) \| +45.11 (2.0) \|
  stand \|
| s20-5 \| 8.0e+04 \| +67.00 (2.0) \| +25.96 (1.0) \| +51.24 (2.0) \|
  stand \|
| s20-6 \| 8.0e+04 \| +68.16 (2.0) \| +28.27 (1.0) \| +55.63 (2.0) \|
  stand \|
| s20-7 \| 8.0e+04 \| +77.47 (2.0) \| +25.35 (1.0) \| +40.40 (2.0) \|
  stand \|
| s20-8 \| 8.0e+04 \| +79.10 (2.0) \| +20.23 (1.0) \| +28.37 (2.0) \|
  stand \|
| s20-9 \| 8.0e+04 \| +76.12 (2.0) \| +11.22 (1.0) \| +14.64 (2.0) \|
  stand \|
| s20-T \| 8.0e+04 \| +43.51 (1.9) \| -5.46 (1.0) \| -8.53 (1.9) \|
  stand \|
| s20-A \| 8.0e+04 \| +10.77 (1.7) \| -28.26 (1.0) \| -33.48 (1.7) \|
  stand \|
| s19-2 \| 8.0e+04 \| +38.20 (2.0) \| +11.75 (1.0) \| +23.54 (2.0) \|
  stand \|
| s19-3 \| 8.0e+04 \| +39.82 (2.0) \| +15.29 (1.0) \| +29.62 (2.0) \|
  stand \|
| s19-4 \| 8.0e+04 \| +42.18 (2.0) \| +17.78 (1.0) \| +35.08 (2.0) \|
  stand \|
| s19-5 \| 3.2e+05 \| +43.54 (1.0) \| +20.12 (0.5) \| +40.89 (1.0) \|
  stand \|
| s19-6 \| 5.1e+06 \| +45.35 (0.2) \| +23.10 (0.1) \| +46.20 (0.2) \|
  double \|
| s19-7 \| 8.0e+04 \| +61.59 (2.0) \| +21.74 (1.0) \| +31.70 (2.0) \|
  stand \|
| s19-8 \| 8.0e+04 \| +59.31 (2.0) \| +15.31 (1.0) \| +20.15 (2.0) \|
  stand \|
| s19-9 \| 8.0e+04 \| +28.43 (2.0) \| +0.63 (1.0) \| -8.21 (2.0) \|
  stand \|
| s19-T \| 8.0e+04 \| -1.71 (1.9) \| -16.07 (1.0) \| -28.88 (1.9) \|
  stand \|
| s19-A \| 8.0e+04 \| -18.13 (1.7) \| -35.40 (0.9) \| -46.73 (1.7) \|
  stand \|
| s18-2 \| 2.0e+07 \| +11.03 (0) \| +5.99 (0) \| +11.48 (0) \| double \|
| s18-3 \| 3.2e+05 \| +13.61 (1.0) \| +8.83 (0.5) \| +17.60 (1.0) \|
  double \|
| s18-4 \| 8.0e+04 \| +16.17 (2.0) \| +11.32 (1.0) \| +22.57 (2.0) \|
  double \|
| s18-5 \| 8.0e+04 \| +19.92 (2.0) \| +14.49 (1.0) \| +28.95 (2.0) \|
  double \|
| s18-6 \| 8.0e+04 \| +22.64 (2.0) \| +15.59 (1.0) \| +34.71 (2.0) \|
  double \|
| s18-7 \| 8.0e+04 \| +39.60 (2.0) \| +16.92 (1.0) \| +22.37 (2.0) \|
  stand \|
| s18-8 \| 8.0e+04 \| +10.50 (2.0) \| +4.49 (1.0) \| -3.59 (2.0) \|
  stand \|
| s18-9 \| 8.0e+04 \| -18.75 (2.0) \| -9.78 (1.0) \| -29.28 (2.0) \| hit
  \|
| s18-T \| 8.0e+04 \| -24.01 (1.9) \| -20.91 (1.0) \| -38.83 (1.9) \|
  hit \|
| s18-A \| 8.0e+04 \| -46.67 (1.7) \| -41.65 (0.9) \| -59.24 (1.7) \|
  hit \|
| s17-2 \| 2.0e+07 \| -15.66 (0) \| -0.55 (0) \| -0.87 (0) \| hit \|
| s17-3 \| 3.2e+05 \| -11.98 (1.0) \| +2.36 (0.5) \| +5.48 (1.0) \|
  double \|
| s17-4 \| 8.0e+04 \| -8.31 (2.0) \| +5.04 (1.0) \| +11.89 (2.0) \|
  double \|
| s17-5 \| 8.0e+04 \| -4.91 (2.0) \| +8.85 (1.0) \| +17.72 (2.0) \|
  double \|
| s17-6 \| 8.0e+04 \| -0.74 (2.0) \| +10.12 (1.0) \| +24.25 (2.0) \|
  double \|
| s17-7 \| 8.0e+04 \| -10.35 (2.0) \| +5.08 (1.0) \| -0.47 (2.0) \| hit
  \|
| s17-8 \| 8.0e+04 \| -38.37 (2.0) \| -7.07 (1.0) \| -26.13 (2.0) \| hit
  \|
| s17-9 \| 8.0e+04 \| -41.81 (2.0) \| -15.16 (1.0) \| -40.21 (2.0) \|
  hit \|
| s17-T \| 8.0e+04 \| -45.89 (1.9) \| -25.72 (1.0) \| -50.87 (1.9) \|
  hit \|
| s17-A \| 8.0e+04 \| -66.48 (1.6) \| -46.33 (0.9) \| -69.24 (1.6) \|
  hit \|
| s16-2 \| 8.0e+04 \| -28.80 (2.1) \| -2.39 (1.0) \| -6.32 (2.1) \| hit
  \|
| s16-3 \| 3.2e+05 \| -24.88 (1.0) \| +0.60 (0.5) \| -0.97 (1.0) \| hit
  \|
| s16-4 \| 8.0e+04 \| -21.20 (2.1) \| +3.23 (1.0) \| +6.64 (2.1) \|
  double \|
| s16-5 \| 8.0e+04 \| -15.99 (2.1) \| +6.72 (1.0) \| +13.54 (2.1) \|
  double \|
| s16-6 \| 8.0e+04 \| -12.50 (2.1) \| +8.31 (1.0) \| +19.84 (2.1) \|
  double \|
| s16-7 \| 8.0e+04 \| -47.17 (2.0) \| -1.23 (1.0) \| -18.63 (2.0) \| hit
  \|
| s16-8 \| 8.0e+04 \| -50.91 (2.0) \| -6.98 (1.0) \| -31.09 (2.0) \| hit
  \|
| s16-9 \| 8.0e+04 \| -54.43 (2.0) \| -14.67 (1.0) \| -46.42 (2.0) \|
  hit \|
| s16-T \| 8.0e+04 \| -57.58 (1.9) \| -26.70 (1.0) \| -55.38 (1.9) \|
  hit \|
| s16-A \| 8.0e+04 \| -72.33 (1.6) \| -44.35 (0.9) \| -71.31 (1.6) \|
  hit \|
| s15-2 \| 8.0e+04 \| -29.03 (2.1) \| -0.58 (1.0) \| -5.65 (2.1) \| hit
  \|
| s15-3 \| 8.0e+04 \| -24.71 (2.1) \| +3.31 (1.0) \| -1.28 (2.1) \| hit
  \|
| s15-4 \| 1.3e+06 \| -20.58 (0.5) \| +5.60 (0.3) \| +6.44 (0.5) \|
  double \|
| s15-5 \| 3.2e+05 \| -16.62 (1.0) \| +8.86 (0.5) \| +12.74 (1.0) \|
  double \|
| s15-6 \| 8.0e+04 \| -12.09 (2.1) \| +10.13 (1.0) \| +19.13 (2.1) \|
  double \|
| s15-7 \| 8.0e+04 \| -47.61 (2.0) \| +4.21 (1.0) \| -19.48 (2.0) \| hit
  \|
| s15-8 \| 8.0e+04 \| -51.16 (2.0) \| -2.81 (1.0) \| -30.90 (2.0) \| hit
  \|
| s15-9 \| 8.0e+04 \| -54.31 (2.0) \| -11.18 (1.0) \| -46.93 (2.0) \|
  hit \|
| s15-T \| 8.0e+04 \| -57.49 (1.9) \| -23.85 (1.0) \| -54.93 (1.9) \|
  hit \|
| s15-A \| 8.0e+04 \| -71.98 (1.6) \| -42.68 (0.9) \| -71.58 (1.6) \|
  hit \|
| s14-2 \| 8.0e+04 \| -28.59 (2.1) \| +1.42 (1.0) \| -6.73 (2.1) \| hit
  \|
| s14-3 \| 8.0e+04 \| -24.89 (2.1) \| +4.04 (1.0) \| -0.41 (2.1) \| hit
  \|
| s14-4 \| 8.0e+04 \| -20.84 (2.1) \| +7.58 (1.0) \| +5.13 (2.1) \| hit
  \|
| s14-5 \| 3.2e+05 \| -16.36 (1.0) \| +10.61 (0.5) \| +12.72 (1.0) \|
  double \|
| s14-6 \| 8.0e+04 \| -12.19 (2.1) \| +12.30 (1.0) \| +19.62 (2.1) \|
  double \|
| s14-7 \| 8.0e+04 \| -47.42 (2.1) \| +7.66 (1.0) \| -18.33 (2.1) \| hit
  \|
| s14-8 \| 8.0e+04 \| -50.95 (2.0) \| +0.96 (1.0) \| -30.90 (2.0) \| hit
  \|
| s14-9 \| 8.0e+04 \| -54.73 (2.0) \| -7.82 (1.0) \| -46.11 (2.0) \| hit
  \|
| s14-T \| 8.0e+04 \| -57.51 (1.9) \| -21.19 (1.0) \| -55.00 (1.9) \|
  hit \|
| s14-A \| 8.0e+04 \| -72.28 (1.6) \| -40.00 (0.9) \| -71.64 (1.6) \|
  hit \|
| s13-2 \| 8.0e+04 \| -28.76 (2.1) \| +3.98 (1.0) \| -7.66 (2.1) \| hit
  \|
| s13-3 \| 8.0e+04 \| -25.07 (2.1) \| +7.23 (1.0) \| -0.04 (2.1) \| hit
  \|
| s13-4 \| 8.0e+04 \| -20.02 (2.1) \| +9.31 (1.0) \| +4.88 (2.1) \| hit
  \|
| s13-5 \| 2.0e+07 \| -16.49 (0) \| +12.84 (0) \| +12.70 (0) \| hit \|
| s13-6 \| 8.0e+04 \| -12.39 (2.1) \| +13.78 (1.0) \| +18.69 (2.1) \|
  double \|
| s13-7 \| 8.0e+04 \| -47.43 (2.1) \| +12.59 (1.0) \| -18.10 (2.1) \|
  hit \|
| s13-8 \| 8.0e+04 \| -50.86 (2.0) \| +5.47 (1.0) \| -31.53 (2.0) \| hit
  \|
| s13-9 \| 8.0e+04 \| -54.25 (2.0) \| -4.04 (1.0) \| -46.51 (2.0) \| hit
  \|
| s13-T \| 8.0e+04 \| -57.70 (1.9) \| -16.62 (1.0) \| -54.45 (1.9) \|
  hit \|
| s13-A \| 8.0e+04 \| -71.53 (1.7) \| -37.98 (0.9) \| -69.91 (1.7) \|
  hit \|
| s12-2 \| 8.0e+04 \| -28.39 (2.1) \| +7.36 (1.0) \| -6.07 (2.1) \| hit
  \|
| s12-3 \| 8.0e+04 \| -25.70 (2.1) \| +9.50 (1.0) \| -0.03 (2.1) \| hit
  \|
| s12-4 \| 8.0e+04 \| -20.48 (2.1) \| +12.65 (1.0) \| +6.24 (2.1) \| hit
  \|
| s12-5 \| 3.2e+05 \| -16.42 (1.0) \| +15.18 (0.5) \| +13.22 (1.0) \|
  hit \|
| s12-6 \| 8.0e+04 \| -11.66 (2.1) \| +15.42 (1.0) \| +19.62 (2.1) \|
  double \|
| s12-7 \| 8.0e+04 \| -47.27 (2.1) \| +16.31 (1.0) \| -16.99 (2.1) \|
  hit \|
| s12-8 \| 8.0e+04 \| -51.51 (2.0) \| +9.36 (1.0) \| -30.70 (2.0) \| hit
  \|
| s12-9 \| 8.0e+04 \| -54.23 (2.0) \| -0.10 (1.0) \| -44.47 (2.0) \| hit
  \|
| s12-T \| 8.0e+04 \| -57.77 (1.9) \| -14.11 (1.0) \| -57.03 (1.9) \|
  hit \|
| s12-A \| 8.0e+04 \| -72.70 (1.6) \| -34.54 (0.9) \| -70.64 (1.6) \|
  hit \|

|  Hand \| $n$ \| Yes \[%\] \| No \[%\] \|
| pA-2 \| 8.0e+04 \| +47.20 (1.7) \| +6.91 (1.0) \| yes \|
| pA-3 \| 8.0e+04 \| +53.01 (1.7) \| +8.95 (1.0) \| yes \|
| pA-4 \| 8.0e+04 \| +55.98 (1.7) \| +11.49 (1.0) \| yes \|
| pA-5 \| 8.0e+04 \| +60.89 (1.7) \| +14.66 (1.0) \| yes \|
| pA-6 \| 8.0e+04 \| +66.26 (1.7) \| +20.80 (2.1) \| yes \|
| pA-7 \| 8.0e+04 \| +46.06 (1.6) \| +16.94 (1.0) \| yes \|
| pA-8 \| 8.0e+04 \| +35.38 (1.6) \| +9.52 (1.0) \| yes \|
| pA-9 \| 8.0e+04 \| +22.97 (1.6) \| -0.20 (1.0) \| yes \|
| pA-T \| 8.0e+04 \| +8.41 (1.6) \| -14.49 (1.0) \| yes \|
| pA-A \| 8.0e+04 \| -23.73 (1.5) \| -34.77 (0.9) \| yes \|
| pT-2 \| 8.0e+04 \| -7.22 (3.8) \| +63.33 (0.7) \| no \|
| pT-3 \| 8.0e+04 \| +1.85 (3.8) \| +64.34 (0.7) \| no \|
| pT-4 \| 8.0e+04 \| +15.20 (4.0) \| +65.82 (0.7) \| no \|
| pT-5 \| 8.0e+04 \| +24.91 (4.0) \| +66.75 (0.7) \| no \|
| pT-6 \| 8.0e+04 \| +37.30 (4.0) \| +67.83 (0.7) \| no \|
| pT-7 \| 8.0e+04 \| +9.41 (2.6) \| +76.75 (0.6) \| no \|
| pT-8 \| 8.0e+04 \| -23.72 (2.5) \| +79.31 (0.6) \| no \|
| pT-9 \| 8.0e+04 \| -59.32 (2.6) \| +75.92 (0.6) \| no \|
| pT-T \| 8.0e+04 \| -77.52 (2.6) \| +43.30 (0.7) \| no \|
| pT-A \| 8.0e+04 \| -85.16 (2.2) \| +10.82 (1.0) \| no \|
| p9-2 \| 8.0e+04 \| +20.02 (2.2) \| +11.24 (1.0) \| yes \|
| p9-3 \| 8.0e+04 \| +25.25 (2.2) \| +13.40 (1.0) \| yes \|
| p9-4 \| 8.0e+04 \| +33.27 (2.2) \| +16.87 (1.0) \| yes \|
| p9-5 \| 8.0e+04 \| +39.23 (2.2) \| +19.21 (1.0) \| yes \|
| p9-6 \| 8.0e+04 \| +45.68 (2.2) \| +22.91 (1.0) \| yes \|
| p9-7 \| 8.0e+04 \| +36.00 (1.9) \| +40.30 (0.9) \| no \|
| p9-8 \| 8.0e+04 \| +23.61 (1.9) \| +10.59 (0.8) \| yes \|
| p9-9 \| 8.0e+04 \| -6.82 (1.8) \| -18.28 (1.0) \| yes \|
| p9-T \| 8.0e+04 \| -36.80 (1.9) \| -23.83 (1.0) \| no \|
| p9-A \| 3.2e+05 \| -48.13 (0.8) \| -46.28 (0.4) \| no \|
| p8-2 \| 8.0e+04 \| +5.68 (2.4) \| -28.12 (1.0) \| yes \|
| p8-3 \| 8.0e+04 \| +13.29 (2.4) \| -24.45 (1.0) \| yes \|
| p8-4 \| 8.0e+04 \| +20.18 (2.5) \| -20.57 (1.0) \| yes \|
| p8-5 \| 8.0e+04 \| +28.86 (2.5) \| -15.92 (1.0) \| yes \|
| p8-6 \| 8.0e+04 \| +38.58 (2.7) \| -12.18 (1.1) \| yes \|
| p8-7 \| 8.0e+04 \| +32.62 (2.1) \| -41.09 (0.9) \| yes \|
| p8-8 \| 8.0e+04 \| -1.93 (2.1) \| -46.30 (0.9) \| yes \|
| p8-9 \| 8.0e+04 \| -39.65 (2.2) \| -50.97 (0.9) \| yes \|
| p8-T \| 8.0e+04 \| -51.78 (1.9) \| -57.43 (0.8) \| yes \|
| p8-A \| 3.2e+05 \| -66.56 (0.8) \| -68.26 (0.4) \| yes \|
| p7-2 \| 8.0e+04 \| -14.09 (2.6) \| -28.55 (1.0) \| yes \|
| p7-3 \| 8.0e+04 \| -5.00 (2.8) \| -25.59 (1.0) \| yes \|
| p7-4 \| 8.0e+04 \| +5.68 (2.9) \| -20.86 (1.0) \| yes \|
| p7-5 \| 8.0e+04 \| +15.57 (2.9) \| -16.37 (1.0) \| yes \|
| p7-6 \| 8.0e+04 \| +25.52 (2.9) \| -12.25 (1.1) \| yes \|
| p7-7 \| 8.0e+04 \| -5.23 (2.1) \| -31.97 (1.0) \| yes \|
| p7-8 \| 3.2e+05 \| -39.50 (1.0) \| -37.19 (0.5) \| no \|
| p7-9 \| 8.0e+04 \| -57.79 (2.1) \| -42.92 (0.9) \| no \|
| p7-T \| 8.0e+04 \| -68.47 (1.8) \| -50.66 (0.9) \| no \|
| p7-A \| 8.0e+04 \| -81.50 (1.5) \| -63.51 (0.8) \| no \|
| p6-2 \| 8.0e+04 \| -17.82 (2.5) \| -26.09 (1.0) \| yes \|
| p6-3 \| 8.0e+04 \| -11.19 (2.9) \| -23.68 (1.0) \| yes \|
| p6-4 \| 8.0e+04 \| -0.23 (2.9) \| -20.51 (1.0) \| yes \|
| p6-5 \| 8.0e+04 \| +8.19 (2.9) \| -16.81 (1.0) \| yes \|
| p6-6 \| 8.0e+04 \| +18.21 (2.9) \| -12.51 (1.1) \| yes \|
| p6-7 \| 8.0e+04 \| -26.43 (2.0) \| -21.52 (1.0) \| no \|
| p6-8 \| 8.0e+04 \| -41.81 (2.0) \| -27.45 (1.0) \| no \|
| p6-9 \| 8.0e+04 \| -61.03 (1.9) \| -33.82 (1.0) \| no \|
| p6-T \| 8.0e+04 \| -74.39 (1.7) \| -43.03 (0.9) \| no \|
| p6-A \| 8.0e+04 \| -82.19 (1.4) \| -57.43 (0.8) \| no \|
| p5-2 \| 8.0e+04 \| -28.56 (2.3) \| +36.16 (2.0) \| no \|
| p5-3 \| 8.0e+04 \| -19.64 (2.5) \| +40.18 (2.0) \| no \|
| p5-4 \| 8.0e+04 \| -12.32 (2.8) \| +45.78 (2.0) \| no \|
| p5-5 \| 8.0e+04 \| -5.05 (2.8) \| +52.02 (2.0) \| no \|
| p5-6 \| 8.0e+04 \| +8.49 (2.8) \| +57.75 (2.0) \| no \|
| p5-7 \| 8.0e+04 \| -29.66 (1.8) \| +38.20 (2.0) \| no \|
| p5-8 \| 8.0e+04 \| -45.66 (1.8) \| +29.62 (2.0) \| no \|
| p5-9 \| 8.0e+04 \| -63.47 (1.7) \| +14.03 (2.0) \| no \|
| p5-T \| 8.0e+04 \| -75.39 (1.7) \| -5.08 (1.0) \| no \|
| p5-A \| 8.0e+04 \| -83.27 (1.4) \| -28.59 (1.0) \| no \|
| p4-2 \| 8.0e+04 \| -18.11 (2.5) \| -2.44 (1.0) \| no \|
| p4-3 \| 8.0e+04 \| -11.18 (2.7) \| +0.19 (1.0) \| no \|
| p4-4 \| 8.0e+04 \| -0.79 (2.9) \| +3.43 (1.0) \| no \|
| p4-5 \| 1.3e+06 \| +8.28 (0.7) \| +7.07 (0.3) \| yes \|
| p4-6 \| 8.0e+04 \| +18.98 (3.0) \| +8.70 (1.0) \| yes \|
| p4-7 \| 8.0e+04 \| -16.47 (2.0) \| +7.80 (1.0) \| no \|
| p4-8 \| 8.0e+04 \| -32.77 (2.0) \| -6.00 (1.0) \| no \|
| p4-9 \| 8.0e+04 \| -50.81 (2.0) \| -20.81 (1.0) \| no \|
| p4-T \| 8.0e+04 \| -65.67 (1.7) \| -30.72 (1.0) \| no \|
| p4-A \| 8.0e+04 \| -74.66 (1.5) \| -49.18 (0.9) \| no \|
| p3-2 \| 2.0e+07 \| -13.64 (0) \| -13.80 (0) \| yes \|
| p3-3 \| 8.0e+04 \| -5.32 (2.6) \| -10.45 (1.0) \| yes \|
| p3-4 \| 8.0e+04 \| +4.08 (2.7) \| -7.45 (1.0) \| yes \|
| p3-5 \| 8.0e+04 \| +12.53 (2.9) \| -3.04 (1.0) \| yes \|
| p3-6 \| 8.0e+04 \| +22.61 (2.9) \| +0.47 (1.0) \| yes \|
| p3-7 \| 8.0e+04 \| -4.91 (2.0) \| -14.91 (1.0) \| yes \|
| p3-8 \| 3.2e+05 \| -23.20 (1.0) \| -21.69 (0.5) \| no \|
| p3-9 \| 8.0e+04 \| -43.61 (2.0) \| -29.14 (1.0) \| no \|
| p3-T \| 8.0e+04 \| -58.65 (1.7) \| -38.95 (0.9) \| no \|
| p3-A \| 8.0e+04 \| -70.06 (1.5) \| -54.59 (0.8) \| no \|
| p2-2 \| 3.2e+05 \| -8.69 (1.1) \| -11.47 (0.5) \| yes \|
| p2-3 \| 8.0e+04 \| -1.93 (2.5) \| -8.55 (1.0) \| yes \|
| p2-4 \| 8.0e+04 \| +6.19 (2.7) \| -4.95 (1.0) \| yes \|
| p2-5 \| 8.0e+04 \| +15.78 (2.7) \| -1.24 (1.0) \| yes \|
| p2-6 \| 8.0e+04 \| +24.78 (2.9) \| +3.07 (1.0) \| yes \|
| p2-7 \| 8.0e+04 \| +1.48 (2.0) \| -8.90 (1.0) \| yes \|
| p2-8 \| 3.2e+05 \| -17.88 (1.0) \| -16.10 (0.5) \| no \|
| p2-9 \| 8.0e+04 \| -38.73 (2.0) \| -24.38 (1.0) \| no \|
| p2-T \| 8.0e+04 \| -54.45 (1.8) \| -34.92 (0.9) \| no \|
| p2-A \| 8.0e+04 \| -67.11 (1.5) \| -51.59 (0.9) \| no \|

### Detailed explanation

We want to derive the basic strategy from scratch, i.e. without making
any assumption. What we are going to do is to play a large (more on what
*large* means below) number of hands by fixing our first two cards and
the dealer upcard and sequentially standing, doubling or hitting the
first card. Then we will compare the results for the three cases and
select as the proper strategy the best one of the three possible
choices.

Standing and doubling are easy plays, because after we stand or double
down then the dealer plays accordingly to the rules: she hits until
seventeen, possibly hitting soft seventeen. But if we hit on our hand,
we might need to make another decision whether to stand or hit again. As
we do not want to assume anything, we have to play in such an order that
if we do need to make another decision, we already know which is the
best one.

#### Hard hands

So we start by arranging the shoe so that the user gets hard twenty
(i.e. two faces) and the dealer gets successively upcards of two to ace.
So we play each combination of dealer upcard (ten) three times each
playing either

1.  always standing
2.  always doubling
3.  always hitting

In general the first two plays are easy, because the game stops either
after standing or after receiving only one card. The last one might lead
to further hitting, but since we are starting with a hard twenty, that
would either give the player twenty one or a bust. In any case, the game
also ends. So we play a certain number of hands (say one thousand hands)
each of these three plays for each of the ten upcard faces and record
the outcome. The correct play for hard twenty against each of the ten
upcards is the play that gave the better result, which is of course
standing.

Next, we do the same for a hard nineteen. In this case, the hitting play
might not end after one card is drawn (i.e. we hit on nineteen and get
and ace). But if that was the case, we would already know what the best
play is from the previous step so we play accordingly and we stand.
Repeating this procedure down to hard four we can build the basic
strategy table for any hard total against any dealer upcard.

#### Soft hands

We can now switch to analyze soft hands. Starting from soft twenty
(i.e. an ace and a nine) we do the same we did for the hard case. The
only difference is that when hitting, we might end either in another
soft hand which we would already analyzed because we start from twenty
and go down, or in a hard hand, which we also already analyzed so we can
play accordingly.

#### Pairs

When dealing with pairs, we have to decide whether to split or not. When
we do not split, we end up in one of the already-analyzed cases: either
a soft twelve of any even hard hand. When we split, we might end in a
hard or soft hand (already analyzed) or in a new pair. But since the new
pair can be only the same pair we are analyzing, we have to treat it
like we treated the first pair: either to split it or not, so we know
how to deal with it.

#### Number of hands

The output is the expected value $e$ of the bankroll, which is a random
variable with an associated uncertainty $\Delta e$ (i.e. a certain
numbers of standard deviations). For example, if we received only
blackjacks, the expected value would be 1.5 (provided blackjacks pay 3
to 2 of course). If we busted all of our hands without doubling or
splitting, the expected value would be -1. In order to say that the best
strategy is, let's say stand and not hitting or doubling down, we have
to make sure that $e_h-\Delta e_h > e_s+\Delta e_s$ and
$e_h-\Delta e_h > e_d+\Delta e_d$. If there is no play that can give a
better expected value than the other two taking into account the
uncertainties, then we have to play more hands in order to reduce the
random uncertainty.

### Implementation

The steps above can be written in a
[Bash](https://en.wikipedia.org/wiki/Bash_%28Unix_shell%29) script that

-   loops over hands and upcards,
-   creates a strategy file for each possible play hit, double or stand
    (or split or not),
-   runs [Libre Blackjack](https://www.seamplex.com/blackjack),
-   checks the results and picks the best play,
-   updates the strategy file

``` {.bash}
#!/bin/bash

   n0=80000
n_max=9000000

RED="\033[0;31m"
GREEN="\033[0;32m"

BROWN="\033[0;33m"
MAGENTA="\e[0;35m"
CYAN="\e[0;36m"

NC="\033[0m" # No Color

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
min["hard"]=4   # from 20 to 4 in hards
min["soft"]=12  # from 20 to 12 in softs

rm -f table.md hard.html soft.html pair.html

# start with standing
cp hard-stand.txt hard.txt
cp soft-stand.txt soft.txt

cat << EOF >> table.md
|  Hand  |  \$n\$  |  Stand [%]  |  Double [%]  |  Hit [%] |   Play    |
EOF


for type in hard soft; do
 for hand in $(seq 20 -1 ${min[${type}]}); do
 
  # choose two random cards that make up the player's assumed total
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
   # one card is an ace
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
 
   n=${n0}   # start with n0 hands
   best="x"  # x means don't know what to so, so play
   
   while [ "${best}" = "x" ]; do
    # tell the user which combination we are trying and how many we will play
    echo -ne "${t}${hand}-${upcard} ($card1 $card2)\t"$(printf %.1e ${n})
    for play in s d h; do
     
     # start with options.conf as a template and add some custom stuff
     cp options.conf blackjack.conf
     cat << EOF >> blackjack.conf
hands = ${n}
player = internal
arranged_cards = ${card1}, $((${upcard_n} + 13)), $((${card2} + 26))
report = ${t}${hand}-${upcard}-${play}.yaml
#log = ${t}${hand}-${upcard}-${play}.log
EOF
 
     # read the current strategy
     while read w p2 p3 p4 p5 p6 p7 p8 p9 pT pA; do
      # w already has the "h" or the "s"
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
     
     # override the read strategy with the explicit play: s, d or h
     strategy[${t}${hand},${upcard}]=${play}
     
     # save the new (temporary) strategy
     rm -f ${type}.txt
     for h in $(seq 20 -1 ${min[${type}]}); do
      echo -n "${t}${h}  " >> ${type}.txt
      
      # extra space if h < 10
      if [ ${h} -lt 10 ]; then
       echo -n " " >> ${type}.txt
      fi 
      
      for u in $(seq 2 9) T A; do
       echo -n "${strategy[${t}${h},${u}]}  " >> ${type}.txt
      done
      echo >> ${type}.txt
     done

     # debug, comment for production
     if [ "${debug}" != "0" ]; then
      cp ${type}.txt ${t}${hand}-${upcard}-${play}.str
     fi
    
     # ensamble the full bs.txt with no pairing
     cat hard.txt soft.txt pair-no.txt > bs.txt
     
     # play!
     blackjack
    
     # evaluate the results
     ev[${t}${hand},${upcard},${play}]=$(grep mean ${t}${hand}-${upcard}-${play}.yaml | awk '{printf("%g", $2)}')
     error[${t}${hand},${upcard},${play}]=$(grep error ${t}${hand}-${upcard}-${play}.yaml | awk '{printf("%g", $2)}')
     
    done
   
    # choose the best one
    ev_s=$(echo ${ev[${t}${hand},${upcard},s]} | awk '{printf("%+.2f", 100*$1)}')
    ev_d=$(echo ${ev[${t}${hand},${upcard},d]} | awk '{printf("%+.2f", 100*$1)}')
    ev_h=$(echo ${ev[${t}${hand},${upcard},h]} | awk '{printf("%+.2f", 100*$1)}')
   
    
    if [ ${n} -le ${n_max} ]; then 
     # if we still have room, take into account errors
     error_s=$(echo ${error[${t}${hand},${upcard},s]} | awk '{printf("%.1f", 100*$1)}')
     error_d=$(echo ${error[${t}${hand},${upcard},d]} | awk '{printf("%.1f", 100*$1)}')
     error_h=$(echo ${error[${t}${hand},${upcard},h]} | awk '{printf("%.1f", 100*$1)}')
    else
     # instead of running infinite hands, above a threshold asume errors are zero
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
   
   
   # save the strategy again with the best strategy
   rm -f ${type}.txt
   for h in $(seq 20 -1 ${min[${type}]}); do
    echo -n "${t}${h}  " >> ${type}.txt
    
    # extra space if h < 10
    if [ ${h} -lt 10 ]; then
     echo -n " " >> ${type}.txt
    fi 
    
    for u in $(seq 2 9) T A; do
     echo -n "${strategy[${t}${h},${u}]}  " >> ${type}.txt
    done
    
    echo >> ${type}.txt
    
   done
  done
  
#   echo "</tr>" >> ${type}.html
  
 done
done


cat << EOF >> table.md


|  Hand  |  \$n\$  |   Yes [%]  |   No [%]   |
EOF

# pairs
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
  
#  cat << EOF >> ${type}.html
#  <tr>
#   <td>${t}${hand}</td>
#   <td>
#    <div class="text-right">y<span class="d-none d-lg-inline">es</span></div>
#    <div class="text-right">n<span class="d-none d-lg-inline">o</span></div>
#   </td>
# EOF
  
 for upcard in $(seq 2 9) T A; do
  if [ "$upcard" = "T" ]; then
    upcard_n=10
  elif [ "$upcard" = "A" ]; then
    upcard_n=1
  else
    upcard_n=$(($upcard))
  fi
 
  n=${n0}   # start with n0 hands
  best="x"  # x means don't know what to so, so play
   
  while [ "${best}" = "x" ]; do
   # tell the user which combination we are trying and how many we will play
   echo -ne "${t}${hand}-${upcard}\t\t$(printf %.0e ${n})"
   
   for play in y n; do
    
    # start with options.conf as a template and add some custom stuff
    cp options.conf blackjack.conf
    cat << EOF >> blackjack.conf
hands = ${n}
player = internal
arranged_cards = ${pair}, $((${upcard_n} + 13)), $((${pair} + 26))
report = ${t}${hand}-${upcard}-${play}.yaml
# log = ${t}${hand}-${upcard}-${play}.log
EOF
 
    # read the current strategy
    while read w p2 p3 p4 p5 p6 p7 p8 p9 pT pA; do
     # w already has the "p"
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
     
    # override the read strategy with the explicit play: y or n
    strategy[${t}${hand},${upcard}]=${play}
     
    # save the new (temporary) strategy
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
    
    # ensamble the full bs.txt
    cat hard.txt soft.txt pair.txt > bs.txt

    # play!
    blackjack

    # evaluate the results
    ev[${t}${hand},${upcard},${play}]=$(grep mean ${t}${hand}-${upcard}-${play}.yaml | awk '{printf("%g", $2)}')
    error[${t}${hand},${upcard},${play}]=$(grep error ${t}${hand}-${upcard}-${play}.yaml | awk '{printf("%g", $2)}')
    
   done
   
   # choose the best one
   ev_y=$(echo ${ev[${t}${hand},${upcard},y]} | awk '{printf("%+.2f", 100*$1)}')
   ev_n=$(echo ${ev[${t}${hand},${upcard},n]} | awk '{printf("%+.2f", 100*$1)}')
   
   if [ $n -le ${n_max} ]; then 
    # if we still have room, take into account errors
    error_y=$(echo ${error[${t}${hand},${upcard},y]} | awk '{printf("%.1f", 100*$1)}')
    error_n=$(echo ${error[${t}${hand},${upcard},n]} | awk '{printf("%.1f", 100*$1)}')
   else
    # instead of running infinite hands, above a threshold asume errors are zero
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
   
  # save the strategy again with the best strategy
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


