---
title: Libre Blackjack
subtitle:  A free blackjack back end
desc: a completely free-as-in-freedom blackjack engine designed to study and analyze the game statistics using different playing strategies ranging from simple card counting up to other complex algorithms based on artificial intelligence.
author: Jeremy Theler
date: December 20, 2020
version: libreblackjackversion
infoname: blackjack
lang: en-US
---

# Overview

include(overview.md)

# Running `blackjack`

## Invocation

The format for running the `blackjack` program is:

```terminal
blackjack [options] [path_to_conf_file]
```

If no configuration file is given, a file named `blackjack.conf` in the current directory is used, provided it exists.
With no options and no configuration file, `blackjack` starts in interactive mode and it is ready to start a blackjack game.

The `blackjack` executable supports the following options:


include(help.md)

All the options which can be given in the configuration file can be passed as a command-line argument. For example, running

```terminal
blackjack --decks=4 --no_insurance=true
```

is equivalent to using a configuration file with

```ini
decks = 4
no_insurance = true
```


## Interactive game

If `blackjack` is attached to an interactive TTY (i.e. neither the standard input nor outputs are redirected), an interactive game is triggered. First thing the program will do is to ask for a bet:

```terminal
$ blackjack
LibreBlackjack v0.2+Δ
a free & open blackjack engine


Starting new hand #1 with bankroll 0
 <-- Bet?
 > 
```

So the user should enter a number, say "1" and then press Enter and then a game will be dealt:

```terminal
Player's card is Q♠
Dealer's up card is 10♠
Player's card is Q♣
Dealer's hole card is dealt
No blackjacks
 -- Dealer's hand:  --------
 _____    _____   
|10   |  |#####|  
|     |  |#####|  
|  ♠  |  |#####|  
|     |  |#####|  
|___10|  |#####|  
    Value: 10
 -- Player's hand --------
 _____    _____   
|Q    |  |Q    |  
|     |  |     |  
|  ♠  |  |  ♣  |  
|     |  |     |  
|____Q|  |____Q|  
    Value: 20
 <-- Play? 20 10
 > 
```

A flat-betting game can be played by passing `--flat_bet=true` (or `--flat_bet=2` to bet two units each hand) through the command line:


```terminal
$ blackjack --flat_bet=true
gtheler@tom:~/codigos/libreblackjack++/doc$ blackjack --flat_bet=true
LibreBlackjack v0.2.5-g9c5893b
a free & open blackjack engine


Starting new hand #1 with bankroll 0
Player's card is 7♠
Dealer's up card is 2♣
Player's card is 3♣
Dealer's hole card is dealt
 -- Dealer's hand:  --------
 _____    _____   
|2    |  |#####|  
|     |  |#####|  
|  ♣  |  |#####|  
|     |  |#####|  
|____2|  |#####|  
    Value: 2
 -- Player's hand --------
 _____    _____   
|7    |  |3    |  
|     |  |     |  
|  ♠  |  |  ♣  |  
|     |  |     |  
|____7|  |____3|  
    Value: 10
 <-- Play? 10 2
 > 
```

The user can quit by either typing `quit` (or `q`) or hitting Ctrl-D.


# Blackjack basic rules


> I overhear a lot of bad gambling advice in the casinos. Perhaps the most frequent is this one, "The object of blackjack is to get as close to 21 as possible, without going over." No! The object of blackjack is to beat the dealer. To beat the dealer the player must first not bust (go over 21) and second either outscore the dealer or have the dealer bust.
>
> The Wizard of Odds, <https://wizardofodds.com/games/blackjack/basics/#rules>

Here are the basic Blackjack rules:

 1. Blackjack may be played with one to eight decks of 52-card decks.
 2. Aces may be counted as 1 or 11 points, 2 to 9 according to pip value, and tens and face cards count as ten points.
 3. The value of a hand is the sum of the point values of the individual cards. Except, a "blackjack" is the highest hand, consisting of an ace and any 10-point card, and it outranks all other 21-point hands.
 4. After the players have bet, the dealer will give two cards to each player and two cards to himself. One of the dealer cards is dealt face up. The facedown card is called the "hole card."
 5.  If the dealer has an ace showing, he will offer a side bet called "insurance." This side wager pays 2 to 1 if the dealer's hole card is any 10-point card. Insurance wagers are optional and may not exceed half the original wager.
 6. If the dealer has a ten or an ace showing (after offering insurance with an ace showing), then he will peek at his facedown card to see if he has a blackjack. If he does, then he will turn it over immediately.
 7. If the dealer does have a blackjack, then all wagers (except insurance) will lose, unless the player also has a blackjack, which will result in a push. The dealer will resolve insurance wagers at this time.
 8. Play begins with the player to the dealer's left. The following are the choices available to the player:
     -   **Stand**: Player stands pat with his cards.
     -   **Hit**: Player draws another card (and more if he wishes). If this card causes the player's total points to exceed 21 (known as "breaking" or "busting") then he loses.
     -   **Double**: Player doubles his bet and gets one, and only one, more card.
     -   **Split**: If the player has a pair, or any two 10-point cards, then he may double his bet and separate his cards into two individual hands. The dealer will automatically give each card a second card. Then, the player may hit, stand, or double         normally. However, when splitting aces, each ace gets only one card. Sometimes doubling after splitting is not allowed. If the player gets a ten and ace after splitting, then it counts as 21 points, not a blackjack. Usually the player may keep
        re-splitting up to a total of four hands. Sometimes re-splitting aces is not allowed.
    -   **Surrender**: The player forfeits half his wager, keeping the other half, and does not play out his hand. This option is only available on the initial two cards, and depending on casino rules, sometimes it is not allowed at all.
 9.  After each player has had his turn, the dealer will turn over his hole card. If the dealer has 16 or less, then he will draw another card. A special situation is when the dealer has an ace and any number of cards totaling six points (known as a "soft 17"). At some tables, the dealer will also hit a soft 17.
 10. If the dealer goes over 21 points, then any player who didn't already bust will win.
 11. If the dealer does not bust, then the higher point total between the player and dealer will win.
 12. Winning wagers pay even money, except a winning player blackjack usually pays 3 to 2. Some casinos have been short-paying blackjacks, which is a rule strongly in the casino's favor.


In Libre Blackjack, the dealer (he) and the player (she) can “talk” through commands which are ASCII strings sent through the standard input/output. In the most basic case, a human player reads commands from the dealer from `blackjack`’s  standard output and writes her commands into the dealer’s standard input. Those commands from the dealer that require a particular action from the player end with a quotation sign such as `bet?`, `insurance?` or `play?`.

All numerical values such as hand totals or bankrolls are given as decimal ASCII strings.

## Commands from the dealer to the player

TBD

## Commands from the player to the dealer

The following commands are available for the player for playing her hand.

include(input-particular.md)


The following are general commands in the sense that they can be sent from the player to the dealer at any moment of the game.

include(input-general.md)





# Configuration file

Libre Blackjacks reads a configuration file that contains
 
 * settings about the rules of the game
    - number of decks,
    - whether if the dealer has to hit soft seventeen or not,
    - blackjack payout,
    - maximum bet allowed,
    - etc...
 * how the player is supposed to play
    - number of hands
    - whether a flat or variable bet is going to be used,
    - etc...
 * if there are any particular shoe arrangement, i.e. a predefined set of cards dealt in a certain order for instance to play one million hands of a sixteen against a dealer’s ten
 * what kind of information is shown in the interactive session
    - if ASCII-art cards are supposed to be shown,
    - a real-time delay to make the game smoother,
    - etc.
 * how the automated player communicates with the dealer
    - using standard input/output,
    - FIFO named pipes,
    - POSIX message queues,
    - POSIX shared memory,
    - etc...
 
The location of the configuration file can be given in the command line. If none is provided, a file named `blackjack.conf` in the current directory is used. If such file does not exists, the defaults values of each variable are used. Individual variables can be set from the command line by passing one or more times the option `--`configuration_variable`[=`*value*`]` in the [invocation].

Comments can be inserted using either a hash `#` or a colon `;`. The following configuration file is the default provided in the main distribution tarball:

```ini
# uncomment the following line to arrange cards
# arranged_cards = 1 5 14 9 27

flat_bet = 1       # do not ask for bets
no_insurance = 1   # do not ask for insurance
decks = 1          # number of decks, negative means infinite
```

## Variables and values

include(conf.md)




## Dumb internal player



# Example automated players

The directory `players` contains a few examples of automated player, which are discussed in the following sections. A script `check.sh` runs some of them and compares the expected value of the bankroll relative to the number of hands which each player obtains with the theoretical expected value (according to the game rules and player’s strategy) within an  allowed statistical uncertainty. This scripts writes the following table (actual values might vary depending on the random nature of the game):

include(../players/check.md)

The columns are

 1. Case name, as discussed below.
 2. Expected theoretical result in absolute units (i.e. -0.01 means -1%)
 3. Actual result obtained by Libre Blackjack in absolute units
 4. Estimated error equal to the standard deviation of the result
 5. Whether the result coincides or not with the theoretical value


include(players.md)



