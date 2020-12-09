
Libre Blackjack is a blackjack engine that emulates a dealer, deals (digital) cards and understands plain-text commands such as `hit` or `stand`. It's inspired by GNU\ Chess. The basic idea is that one or more players can talk to Libre Blackjack either in an interactive or in an automated way through
 
 * the standard input/output (optionally using named pipes or TCP (web)sockets with `netcat` or `gwsocket`), or
 * C++ methods (optionally loaded at runtime from shared objects---TBD).
  
These players can be actual human players playing in real-time through a front end (a GUI application, a web-based interface, a mobile app, etc.) or robots that implement a certain betting and playing strategy playing (i.e. card counting) as fast as possible to study and analyze game statistics. There is an internal player that reads the strategy from a text file and plays accordingly. Libre Blackjack can also be used to play interactive ASCII blackjack.

## Background

The casino game known as Blackjack has converged to the current mainstream rules since the beginning of the 20th century. Assuming the cards are infinite, the best strategy for the player yields approximately a house edge which is in the order of\ 0.5%. This is a remarkable result, because the rules of the game are not trivial and the overall combination gives a very little margin for the dealer, more than five times smaller than standard single-zero roulette. In 1963, Edward Thorp published his seminal book _Beat the dealer_ where he showed---with the help of the mainframes available at that time---that it is possible to flip the margin to the player's side by taking into account that the chances of dealing the next card of a finite shoe depends on the cards that were already dealt. This was the beginning of the card counting era, and a lot of mathematicians have devoted to the analysis of probabilities in the Blackjack game---and its variations.

> “I am often surprised that when people drive down two-lane roads, they will trust complete strangers in the oncoming lane not to swerve into their lane causing a head-on collision; but they will not trust mathematicians to create the correct strategy for Blackjack.”
>
> Norman Wattenberger, Modern Blackjack, 2009

With Libre Blackjack you do not have to trust other people anymore. You have a free blackjack engine which you can

 0. run as you wish, to see the results of billions of blackjack hands,
 1. study to see how it works and change it if you do not like it,
 2. share it with your friends and colleagues, and
 3. distribute copies of your modified versions.

If you do not know how to program, you have the _freedom_ to hire a programmer to do it for you. That is why Libre Blackjack is _free software_.


Once you trust the blackjack engine is fair, you can model and simulate any blackjack situation you want, playing millions of times a certain hand (say a sixteen against a ten) in different ways (say hitting or standing) with different rules (does the dealer have to hit soft seventeens?) to obtain you own conclusions. You can even build  the basic strategy charts from scratch to convince yourself there is no “flaw.”

The main objective is research and optimization of playing and betting strategies depending on

 * particular table rules (number of decks, hit on soft 17, double after split, etc.), 
 * card counting strategies 
 * risk of ruin
 * removal of cards
 * arranged shoes
 
These automatic players can range from simple no-bust or mimic-the-dealer hitters or standers, up to neural-networks trained players taking into account every card being dealt passing through basic strategy modified by traditional card counting mechanisms.

## A note on the C++ implementation

The first Libre Blackjack version (v0.1) was written in C. This version (v0.2) is a re-implementation of nearly the same functionality but written completely from scratch in C++. I am not a fan of C++ and still prefer old plain C for most of my programming projects, but for the particular case of Libre Blackjack these advantages of C++ over C ought to be noted:

 * the inheritance mechanisms of C++ and virtual methods allows to have generic dealer and player classes from which particular games (dealers) and strategies (players) can be instantiated. This way, Blackjack variations like 
 
    - Spanish 21
    - Down under Blackjack
    - Free Bet Blackjack
    - Blackjack Switch
    
   or even the Spanish “Siete y medio” could be also implemented in the same framework (the card deck should also be changed though). But also playing variations like a dealer that exposes the hole card a certain amount of the time (say 1% or 2% of the hands) could also be studied by extending the base blackjack dealer class.

 * the private members of the C++ classes allow information to be hidden between the dealer and the player, so a far better separation of information can be achieved. This also prevents “cheating” in players by looking at information which is not available for them (such as the dealer’s hole card or the content of the shoe).
 
 * the virtual members of derived players and even be linked to other high-level programming language parsers (such as Python or Julia) allowing to use the vast variety of AI/ML libraries available for these languages to implement advanced playing strategies.
 
 * the usage of STL containers, methods and algorithms allows for a faster and cleaner implementation of cards, hands, decks and shoes.
