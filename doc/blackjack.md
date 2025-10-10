---
title: Libre Blackjack
subtitle:  A free blackjack back end
desc: a completely free-as-in-freedom blackjack engine designed to study and analyze the game statistics using different playing strategies, ranging from simple card counting up to other complex algorithms based on artificial-intelligence techniques
author: Jeremy Theler
infoname: blackjack
lang: en-US
fontsize: 11pt
mainfont: LinLibertineO
sansfont: Carlito
monofont: DejaVuSansMono
geometry:
- paper=a4paper
- left=2.5cm
- right=2cm
- bottom=3.5cm
- foot=2cm
- top=3.5cm
- head=2cm
colorlinks: true
mathspec: true
documentclass: report
---

# Overview

 * Explain what the goal is.
 * Show some examples.
 * One directory (a.k.a. folder)
 * 1/2 Mhands/second
 * Both free and open source so you get the four freedoms
    0. run
    1. study
    2. change
    3. re-distribute
    
> This manual is like an implicit equation.

## Why

Since blackjack is not straightforward to analyze with analytical statistics equations, a monte-carlo-based approach is needed.

 * compute evs and sds for different rules, decks and strategies
 * study arranged shoes
 * ...


## How

 * FOSS
 * unix

## What

a dealer (he) that knows how to deal blackjack, tells the player (her) what cards are dealt, asks the player for her choices (bet, hit, stand, etc.) and keeps track of the results so as to write a report with ev, sd, etc.


# Running `blackjack`

## Invocation {#sec:invocation}

The `blackjack` program executable follows the POSIX standard. Its usage is:

```include
help.md
```

@Sec:configuration contains the details about the settings which can be used in the configuration file.
As already stated, the options which can be given in the configuration file (explained in @sec:configuration) can be passed as a command-line argument following the POSIX double-dash format `--variable=value`.
For example, running

```terminal
blackjack --decks=4 --no_insurance=true
```

is equivalent to using a configuration file with

```ini
decks = 4
no_insurance = true
```

Proper quotation migh be needed if the value contains spaces. For example,

```terminal
blackjack --internal --cards="TH JS 6D"
```

With no command-line options and no configuration file, `blackjack` starts in interactive mode and it is ready to start a blackjack game (see @sec:interactive for details).

## Results {#sec:results}

TBD

In YAML

`yq`

## Interactive game {#sec:interactive}

TBD

If `blackjack` is attached to an interactive TTY (i.e. neither the standard input nor outputs are redirected) and there is no `player` option in the configuration file, an interactive game is triggered. First thing the program will do is to ask for a bet:

```terminal
xxxxx
```

So the user should enter a number, say "1" and then press Enter and then a game will be dealt:

```terminal
xxxxx
```

A flat-betting game can be played by passing `--flat_bet=true` (or `--flat_bet=2` to bet two units each hand) through the command line:


```terminal
$ blackjack --flat_bet=true
xxxxx
```

The user can quit by either typing `quit` (or `q`) or hitting Ctrl-D.
Write `help` or see @sec:p2d for a description of all the possible commands the player can give to the dealer.


# Playing blackjack {#sec:playing}


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
 
    Stand

    :   Player stands pat with his cards.

    Hit
    
    :   Player draws another card (and more if he wishes). If this card causes the player's total points to exceed 21 (known as "breaking" or "busting") then he loses.
    
    Double
    
    :   Player doubles his bet and gets one, and only one, more card.
    
    Split
    
    :   If the player has a pair, or any two 10-point cards, then he may double his bet and separate his cards into two individual hands. The dealer will automatically give each card a second card. Then, the player may hit, stand, or double         normally. However, when splitting aces, each ace gets only one card. Sometimes doubling after splitting is not allowed. If the player gets a ten and ace after splitting, then it counts as 21 points, not a blackjack. Usually the player may keep re-splitting up to a total of four hands. Sometimes re-splitting aces is not allowed.
    
    Surrender
    
    :   The player forfeits half his wager, keeping the other half, and does not play out his hand. This option is only available on the initial two cards, and depending on casino rules, sometimes it is not allowed at all.
    
 9.  After each player has had his turn, the dealer will turn over his hole card. If the dealer has 16 or less, then he will draw another card. A special situation is when the dealer has an ace and any number of cards totaling six points (known as a "soft 17"). At some tables, the dealer will also hit a soft 17.
 10. If the dealer goes over 21 points, then any player who didn't already bust will win.
 11. If the dealer does not bust, then the higher point total between the player and dealer will win.
 12. Winning wagers pay even money, except a winning player blackjack usually pays 3 to 2. Some casinos have been short-paying blackjacks, which is a rule strongly in the casino's favor.

To perform monte-carlo simulations, in Libre Blackjack the dealer (he) and the player (she) can “talk” through commands which are ASCII strings sent through an inter-process communcation (IPC) mechanism.
In the most basic case, an automated player reads messages from the dealer from `blackjack`’s  standard output and writes her ASCII commands into the dealer’s standard input.


## Messages from the dealer to the player {#sec:d2p}

 * Messages are ASCII-formatted string composed of tokens separated by spaces.
 * Each message starts with a single token which is either a single English word (e.g. `bet` or `play`) or more than one English words concatenated using a low hyphen `_` (e.g. `new_hand` or `player_card`). That is to say, the first token of the message is a single-token string.
 * The first token might or might not end with a question mark `?` (e.g. `card_player 4H` or `play? 18 4`):
   - Messages with tokens that do not end in a question mark `?` are informative and do not need any response from the player.
   - Interrogative messages strating with tokens than end in a question mark `?` need to be answered by the player. That is to say, after issuing a message as a question the dealer starts listening to the proper communication channel (see @sec:communication) for a valid command from the dealer (detailed in @sec:p2d). 
 * A message might have extra tokens that convey information to the player, e.g. `new_hand 15141 -4587.5`, `card_player 9S`, `play? 16 10`.
 * All numerical values such as hand totals or bankrolls are given as decimal ASCII strings.

### Interrogative messages {#sec:interrogative}

These messages have to answered by the user.
If an invalid answer is received, an informative message (@sec:informative) with a complain will be sent and then the same interrogative message will be re-sent.

```{.include shift-heading-level-by=3}
commands-int.md
```

### Informative messages {#sec:informative}

For basic players, all of the informative messages listed in this section can be ignored.
For advanced players, almost all of the messages can be ignored.

```{.include shift-heading-level-by=3}
commands-inf.md
```



## Commands from the player to the dealer {#sec:p2d}

TBD

The following commands are available for the player for playing her hand.



The following are general commands in the sense that they can be sent from the player to the dealer at any moment of the game.






# Configuration {#sec:configuration}

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
 
The location of the configuration file can be given in the command line. If none is provided, a file named `blackjack.conf` in the current directory is used. If such file does not exists, the defaults values of each variable are used.

> It is thus recommended to run each monte-carlo simulation that needs a particular configuration---and possibly other files such as card arrangements, strategies and/or results---in a different directory (a.k.a. folder) with a `blackjack.conf` file  in it.

Individual variables can be set from the command line by passing one or more times
the option `--configuration_variable[=value]` as explained in @sec:invocation.

Comments can be inserted using either a hash `#` or a colon `;` at any position in the line.
The following configuration file is the default provided in the main distribution tarball:

```ini
# uncomment the following line to arrange cards
# arranged_cards = AC 5H 8S 9D KH

flat_bet = true       # do not ask for bets
no_insurance = true   # do not ask for insurance
decks = 1             # number of decks, negative means infinite
```

## Reference

```{.include shift-heading-level-by=2}
conf.md
```


# Internal players

TBD

# Examples

The directory `players` contains a few examples of automated players, which are discussed in the following sections.




# Communication mechanisms {#sec:communication}

## Standard input & output

## FIFOs

## Message queues

# Tests

# Statistical utilities

# Installation

## From binaries

## From sources

### Tarball

### Git repository


