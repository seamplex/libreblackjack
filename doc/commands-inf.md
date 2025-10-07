
* `bet_maximum` (@sec:bet_maximum)
* `bet_negative` (@sec:bet_negative)
* `bet_zero` (@sec:bet_zero)
* `card_dealer` (@sec:card_dealer)
* `card_dealer_hole` (@sec:card_dealer_hole)
* `card_dealer_up` (@sec:card_dealer_up)
* `card_player` (@sec:card_player)
* `dealer_blackjack` (@sec:dealer_blackjack)
* `new_hand` (@sec:new_hand)
* `player_blackjack_also` (@sec:player_blackjack_also)
* `player_double_invalid` (@sec:player_double_invalid)
* `player_split_invalid` (@sec:player_split_invalid)
* `player_split_ok` (@sec:player_split_ok)
* `player_wins_insurance` (@sec:player_wins_insurance)
* `shuffling` (@sec:shuffling)


# `bet_maximum` {#sec:bet_maximum}

The dealer complains that the bet the placer placed is invalid.
The bet is larger than the maximum wager allowed by `maximum_bet`.
The player will receive a new `bet?` message.

**Examples**

~~~
bet_maximum
~~~

# `bet_negative` {#sec:bet_negative}

The dealer complains that the bet the placer placed is invalid.
Only positive integer numbers are allowed.
The player will receive a new `bet?` message.

**Examples**

~~~
bet_negative
~~~

# `bet_zero` {#sec:bet_zero}

The dealer complains that the bet the placer placed is invalid.
Only positive integer numbers are allowed.
The player will receive a new `bet?` message.

**Examples**

~~~
bet_zero
~~~

# `card_dealer` $rs$ {#sec:card_dealer}

The dealer informs that the dealer has been dealt a card.
The card is given as the two-character ASCII representation discussed
in @sec:card_player.

**Examples**

~~~
card_dealer TH
card_dealer JC
card_dealer 5D
card_dealer 5H
card_dealer QH
~~~

# `card_dealer_hole` $rs$ {#sec:card_dealer_hole}

The dealer informs what his hole card is.
This message is issued only if playing the american rules, i.e.
with `enhc = false` or `ahc = true`.
The card is given as the two-character ASCII representation discussed
in @sec:card_player.

**Examples**

~~~
card_dealer_hole KH
card_dealer_hole AC
card_dealer_hole 4H
card_dealer_hole 5D
card_dealer_hole 7H
~~~

# `card_dealer_up` $rs$ {#sec:card_dealer_up}

The dealer informs that the dealer has been dealt the up card (i.e.
the first card facing up). This message is issued only once per hand.
The card is given as the two-character ASCII representation discussed
in @sec:card_player.

**Examples**

~~~
card_dealer_up KD
card_dealer_up 7H
card_dealer_up KH
card_dealer_up QD
card_dealer_up 6C
~~~

# `card_player` $rs$ `[` $h$ `]` {#sec:card_player}

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

The optional argument $h$ indicates the id of the player's hand
being dealt. If it not present, that means the base hand.
When performing a splitting on the base hand, the original hand
has id equal to zero and the new hand has id equal to one.
Subsequent splits trigger new hands with sequential ids.

**Examples**

~~~
card_player 9C 
card_player JD 
card_player QC
card_player KS
card_player TD 1
card_player 6H 2 
~~~

# `dealer_blackjack` {#sec:dealer_blackjack}

The dealer informs that he has blackjack.

**Examples**

~~~
dealer_blackjack
~~~

# `new_hand` $n$ $b$ {#sec:new_hand}

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

# `player_blackjack_also` {#sec:player_blackjack_also}

The dealer informs that both he and the user have blackjack.

**Examples**

~~~
player_blackjack_also
~~~

# `player_double_invalid` {#sec:player_double_invalid}

The dealer complains that the doubling-down request cannot be
fulfilled. Doubling down is only possible when exactly two
cards have been dealt in a hand and the `doa` or `do9` option is met.
The player will receive a new `play?` message.

**Examples**

~~~
player_double_invalid
~~~

# `player_split_invalid` {#sec:player_split_invalid}

The dealer complains that the split request cannot be
fulfilled. Splitting is only possible when exactly two
cards with the same rank have been dealt in a hand.
The player will receive a new `play?` message.

**Examples**

~~~
player_split_invalid
~~~

# `player_split_ok` $k$ {#sec:player_split_ok}

The dealer informs that the split request was successfully be
fulfilled. The integer $k$ indicates the id of the split hand (the
first hand has an id equal to one so $k>1$ when splitting).

**Examples**

~~~
player_split_ok 2
player_split_ok 3
~~~

# `player_wins_insurance` {#sec:player_wins_insurance}

The dealer informs that the user won the insurance.

**Examples**

~~~
player_wins_insurance
~~~

# `shuffling` {#sec:shuffling}

The dealer informs that he is shuffling the decks.
This only happens when a non-zero value for the option `decks` is set.
If `decks = 0` (or the command-line option `-d0` is passed), then cards
are drawn randomnly from an infinite set of cards and there is no need to shuffle.

**Examples**

~~~
shuffling
~~~


