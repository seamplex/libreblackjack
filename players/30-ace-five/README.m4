define(case_title, Wizard’s ace-five strategy)
---
title: case_title
...

# case_title

> Difficulty: case_difficulty/100

So far the house always had the edge. it is now the turn for the player to be (a little bit) up. This Python player uses the Wizard of Odds’s  simple card-counting strategy called the Ace/Five count:^[<https://wizardofodds.com/games/blackjack/ace-five-count/>]

 1. Establish what your minimum and maximum bets will be. Usually the maximum will be 8, 16, or 32 times the minimum bet, or any power of 2, but you can use whatever bet spread you wish.
 2. At the beginning of each shoe, start with your minimum bet, and a count of zero.
 3. For each five observed, add one to the count.
 4. For each ace observed, subtract one from the count.
 5. If the count is greater than or equal to two, then double your last bet, up to your maximum bet.
 6. If the count is less than or equal to one, then make the minimum bet.
 7. Use basic strategy for all playing decisions.


Instead of using basic strategy, the player uses the Wizard’s simple strategy which is far easier to memorize (and to code):^[https://wizardofodds.com/games/blackjack/basics/#wizards-simple-strategy]
 
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
include(ace-five.py)
```

Of course there is a catch. Even though this strategy has a positive expectation in the long run, the betting spread adds a lot of dispersion to the results so the number of hands needed to get an uncertainty below the actual expected mean is very large. This, summed to the fact that we are using an interpreted language and a verbose dealer (the player needs to know which cards are being dealt so she can keep the ace/five count) over standard input and output means a few minutes to run. Also, note that counting cards make sense only when playing a shoe game, so a positive number of decks needs to be explicitly given.

```terminal
include(run.sh)dnl
```

In any case, here are the results where the expectation is positive and the error is less than the absolute value of the mean:

```yaml
include(report.yaml)
```

> **Exercise:** explore what the effect of the number of decks.


case_nav
