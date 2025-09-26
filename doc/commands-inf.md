
* `shuffling` (@sec:shuffling)
* `new_hand` (@sec:new_hand)
* `bet_negative` (@sec:bet_negative)
* `bet_maximum` (@sec:bet_maximum)
* `bet_zero` (@sec:bet_zero)
* `card_player` (@sec:card_player)


#### `shuffling` {#sec:shuffling}

The dealer informs that he is shuffling the decks.
This only happens when a non-zero value for the option `decks` is set.
If `decks = 0` (or the command-line option `-d0` is passed), then cards
are drawn randomnly from an infinite set of cards and there is no need to shuffle.

**Examples**

~~~
shuffling
~~~

#### `new_hand` $n$ $b$ {#sec:new_hand}

The dealer states that a new hand is starting. The integer $n$ gives
the number of the hand that is about to start (first hand is $n=1$).
The decimal number $b$ states the player's bankroll before placing
the bet in the hand that is about to start.
Even though bets have to be integers, pay offs might be non-integer
such as when winning a natural (e.g. $3/2 = 1.5$ or $6/5$ = 1.2).    

**Examples**

~~~
new_hand 1 0
new_hand 22 -8
new_hand 24998 -7609.5
~~~

#### `bet_negative` {#sec:bet_negative}

The dealer complains that the bet the placer placed is invalid.
Only positive integer numbers are allowed.
The player will receive a new `bet?` message.

**Examples**

~~~
bet_negative
~~~

#### `bet_maximum` {#sec:bet_maximum}

The dealer complains that the bet the placer placed is invalid.
The bet is larger than the maximum wager allowed by `maximum_bet`.
The player will receive a new `bet?` message.

**Examples**

~~~
bet_maximum
~~~

#### `bet_zero` {#sec:bet_zero}

The dealer complains that the bet the placer placed is invalid.
Only positive integer numbers are allowed.
The player will receive a new `bet?` message.

**Examples**

~~~
bet_zero
~~~

#### `card_player` $rs$ {#sec:card_player}

The dealer informs that the player has been dealt a card.
The card is given as a two-character ASCII representation where
the first character $r$ indicates the rank and the second 
character $s$ gives the suit.

| Character |  Rank            |
|:---------:|------------------|
|    `A`    | Ace              |
|    `2`    | Deuce            |
|    `3`    | Three            |
|    `4`    | Four             |
|    `5`    | Five             |
|    `6`    | Six              |
|    `7`    | Seven            |
|    `8`    | Eight            |
|    `9`    | Nine             |
|    `T`    | Ten              |
|    `J`    | Jack             |
|    `Q`    | Queen            |
|    `J`    | King             |

| Character |  Suit            |
|:---------:|------------------|
|    `C`    | ♣ Clubs          |
|    `D`    | ♦ Diamonds       |
|    `H`    | ♥ Hearts         |
|    `S`    | ♠ Spades         |

**Examples**

~~~
card_player 9C 
card_player JD 
card_player QC 
card_player KS 
card_player TD
card_player 6H 
~~~


