
* `blackjack_pays` (@sec:blackjack_pays)
* `cards` (@sec:cards)
* `cards_file` (@sec:cards_file)
* `dealer` (@sec:dealer)
* `decks` (@sec:decks)
* `flat_bet` (@sec:flat_bet)
* `hands` (@sec:hands)
* `maximum_bet` (@sec:maximum_bet)
* `max_incorrect_commands` (@sec:max_incorrect_commands)
* `new_hand_reset_cards` (@sec:new_hand_reset_cards)
* `no_insurance` (@sec:no_insurance)
* `number_of_burnt_cards` (@sec:number_of_burnt_cards)
* `penetration` (@sec:penetration)
* `penetration_sigma` (@sec:penetration_sigma)
* `player` (@sec:player)
* `quit_when_arranged_cards_run_out` (@sec:quit_when_arranged_cards_run_out)
* `rng_seed` (@sec:rng_seed)
* `rules` (@sec:rules)
* `shuffle_every_hand` (@sec:shuffle_every_hand)


# `blackjack_pays = ` $r$ {#sec:blackjack_pays}

Defines how much a natural pays.
The real number $r$ has to be a decimal number such as `1.5` or `1.2`.

**Default**
$1.5$

**Examples**

~~~
blackjack_pays = 1.5
blackjack_pays = 1.2
~~~

# `cards = ` $\text{list of cards}$ {#sec:cards}

If this option is given, the dealer draws the cards specified on the list.
In the first hand, the order of the dealt cards

 1. Player’s first card
 2. Dealer’s first card
 3. Player’s second card
 4. ...

where the ellipsis dots indicate continuation of the game (i.e. dealer's hole card for `ahc` or player’s hit card for `enhc).

These cards will be the ones specified on the list in the prescribed order.
Each card is given by a two-character string, explained in @tbl:rank and @tbl:suit respectively.
Cards should be separated by spaces.  

The dealer will continue drawing from the list of arranged cards until either

  a. there are no more cards in the list, in which case the dealer will continue drawing cards from either
    i. a shoe with the already-dealt cards removed, if `decks` is non-zero, or
    ii. a set of infinite cards , if `decks` is zero.
  b. the hand is over and `new_hand_reset_cards` is `true`, or
  c. `quit_when_arranged_cards_run_out` is true, in which case the program exits.

**Default**
Empty list

**Examples**

~~~
cards = TH JD 6C
cards = 2S 5D QS AC
cards = 8D QH TC 2C KD 7S 8S TD AH 5C
~~~

# `cards_file = ` $\text{path to file}$ {#sec:cards_file}

This option is exactly the same as `cards` but the cards are given in a text file instead 
of directly in the configuration file.

**Default**
No path

**Examples**

~~~
cards_file = cards.txt
cards_file = ../arranged_cards.txt
cards = /var/games/cards.txt
~~~

# `dealer = ` *game* {#sec:dealer}

Defines the game the dealer will deal.
Currently, the only valid choice is `blackjack`.

**Default**
`blackjack`

**Examples**

~~~
dealer = blackjack
~~~

# `decks = ` $n$ {#sec:decks}

Sets the number of decks used in the game.
If $n$ is zero, the program draws cards from an infinte set.
For a finite $n$, the cards are drawn from a shoe.

**Default**
$0$

**Examples**

~~~
decks = 1
decks = 2
decks = 8
~~~

# `flat_bet = ` $b$ {#sec:flat_bet}

Tells both the dealer and the player that the betting scheme is flat or not.
The dealer will not ask for bets and the internal player, if asked, always says `1`.
The value can be either `false` or `true` or `0` or `1`.

**Default**
$false$

**Examples**

~~~
flat_bet = false
flat_bet = true
flat_bet = 1
~~~

# `hands = ` $n$ {#sec:hands}

Sets the number of hands to play before quiting.
If $n$ is zero, the program keeps playing until it receives the command `quit`.
Otherwise it plays $n$ hands and quits.
This parameter can be set on the command line with the option `-n`$n$ or `--hands=`$n$.

**Default**
$0$

**Examples**

~~~
hands = 1
hands = 1000000
hands = 1e6
~~~

# `maximum_bet = ` $n$ {#sec:maximum_bet}

Sets a limit on the size of the bet.
If a bet larger than the limit is placed, the dealer answers
`bet_maximum` and asks again. A value of 0 means no limit.
This is only useful when modeling variable-betting schemes.

**Default**
$0$

**Examples**

~~~
maximum_bet = 0
maximum_bet = 1
maximum_bet = 20
~~~

# `max_incorrect_commands = ` $n$ {#sec:max_incorrect_commands}

Tells the dealer how many consecutive incorrect or invalid commands to accept before quitting.
A finite value of $n$ avoids infinite loops where the player sends commands
that do not make sense (such as garbage) or that are not valid (such as doubling when not allowed).

**Default**
10

**Examples**

~~~
max_incorrect_commands = 20
~~~

# `new_hand_reset_cards_out = ` $b$ {#sec:new_hand_reset_cards}

If the are arranged cards (either with `cards` or `cards_file`) and $b$ is `true`
then when a hand is finished, the next hand starts with the arranged cards from the very beginning.
Otherwise, if there are enough arranged cards, they will be drawn from the list in the
specified order. If there are no more cards in the arranged list, cards will be drawn from
a randomnly-shuffled shoe or from an infinite set of random cards, depending on the value of `decks`.
This setting only makes sense when arranging cards with either `cards` or `cards_file`.

A usage for $b = \text{true}$ is to give just three cards in `cards` (say the dealer upcard and
the two player’s cards) to study what happens randonly after this intial condition.
In this scenario, all hands will start with the three prescribed cards and the rest of the hand
will be random, either from a shoe with the three already-dealt cards missing or from an infinite set of cards.

A usage for $b = \text{false}$ is to study what happens when the same shoe is played under
different circunstances. In this case, the program should be run several times with the same
arranged cards (most likely using `cards_file` because there are expected a lot of cards to be arranged)
and different hitting/standing strategies to compare outcomes.
If the actual dealt cards are not important but only reproducibility, it is easier to fix `rng_seed`.

**Default**
`true`

**Examples**

~~~
new_hand_reset_cards = false
new_hand_reset_cards = true
~~~

# `no_insurance = ` $b$ {#sec:no_insurance}

If $b$ is `true`, the dealer will not ask for insurance and assume
the player will never take it when the dealer shows an ace.
The value can be either `false` or `true` or `0` or `1`.

**Default**
$false$

**Examples**

~~~
no_insurance = false
no_insurance = true
no_insurance = 1
~~~

# `number_of_burnt_cards = ` $n$ {#sec:number_of_burnt_cards}

Indicates the number $n$ of cards that have to be burnt after shuffling a shoe.
This value only makes sense when playing shoe games, i.e. non-zero `decks`.

**Default**
$0$

**Examples**

~~~
number_of_burnt_cards = 0
number_of_burnt_cards = 1
number_of_burnt_cards = 2
~~~

# `penetration = ` $r$ {#sec:penetration}

When playing a shoe game, sets the penetration of the shoe.
That is to say, the fraction $0 < $r$ < 1$ of the total number of cards on the decks
that have to be played before re-shuffling the shoe.
If the penetration is achieved in the middle of a hand (i.e. the cut card is dealt),
the hand is finished and the shoe is shuffled before the next hand.
Note that if the penetration is too large (i.e. $r lesssim 1$) the shoe might
run out of cards triggering an error and exiting the program

**Default**
`0.75`

**Examples**

~~~
penetration = 0.75
penetration = 0.5
penetration = 0.85
~~~

# `penetration_sigma = ` $r$ {#sec:penetration_sigma}

If $r \neq 0$ then the penetration given in `penetration` is not deterministic but random.
That is to say, the actual penetration fraction will be sampled from a 
gaussian random number generator after shuffling the shoe. This variable `penetration_sigma`
 controls the standard deviation of the distribution.

**Default**
`0`

**Examples**

~~~
penetration_sigma = 0.01
penetration_sigma = 0.05
penetration_sigma = 0.1
~~~

# `player =  ` *player* {#sec:player}

Defines which player will be playing against the dealer.
Currently, the only valid choices are

 * `tty`: the game starts in an interactive mode where the dealer's messages and dealt cards
are printed in the terminal, and the user is asked to issue her commands through the keyboard.
This player is usually used to test if the configuration settings (i.e. `enhc` or `cards_file`)
work as expected, although it can be used to just play ASCII blackjack.
 * `stdio`: the dealer writes messages into the standard output and reads
back commands from the standard input. With proper redirection (and possibly FIFO devices), this
option can be used to have an ad-hoc player to programatically play blackjack.
See @sec-players for examples.
 * `internal`: the dealer plays against an internal player already programmed in
Libre Blackjack that bets flat, never takes insurance and follows the basic strategy. 
The strategy can be changed by setting the configuration variable `strategy_file`.
This player is chosen if `-i` is passed in the command line.

**Default**
If neither the standard input nor output of the executable `blackjack` is re-directed, the default is `tty`.
If at least one of them is re-directed or piped, the default is `stdio`.

**Examples**

~~~
player = tty
player = stdio
player = internal
~~~

# `quit_when_arranged_cards_run_out = ` $b$ {#sec:quit_when_arranged_cards_run_out}

If the are arranged cards (either with `cards` or `cards_file`) and $b$ is `true`
then the program quits when the list of cards ends.
If it is false, the dealer continues drawing cards from a randomnly-shuffled shoe
(where the first cards have been arranged) or from an infinite set of random cards,
depending on the value of `decks` until the either dealer receives a `quit` message or
the maximum number of hands given in `hands` have been played.
This setting only makes sense when arranging cards with either `cards` or `cards_file`.

**Default**
`false`

**Examples**

~~~
quit_when_arranged_cards_run_out = false
quit_when_arranged_cards_run_out = true
~~~

# `rng_seed = ` $n$ {#sec:rng_seed}

This option sets the seed of the random number generator used by the dealer to draw cards.
This is used to get deterministic results. That is to say, the cards draw by two dealers using
the same seed (and the same number of decks) will be the same.
It is not possible to guess what the cards will be given a certain seed $n$.
But the cards will be the same for two executions of the program with the same seed $n$.
If this option is not set, the seed itself is as random as possible.

**Default**
Entropic non-deterministic random seed from C++'s `std::random_device` (most likely `/dev/random`).

**Examples**

~~~
rng_seed = 1
rng_seed = 123456
~~~

# `rules = [ ahc | enhc ] [ h17 | s17 ] [ das | ndas ] [ doa | do9 ]` {#sec:rules}

Defines the rules of the game.

| Rule                                     |   Yes   |   No    |
|:-----------------------------------------|:-------:|:-------:|
| Dealer peeks for blackjack               |  `ahc`  |  `enhc` |
| Dealer has to hit a soft 17              |  `h17`  |  `s17`  |
| Player can double after splitting        |  `das`  |  `ndas` |
| Player can double on any first two cards |  `doa`  |  `do9`  |

 * When playing `ahc` (default), the dealer has a hole card.
 If the upcard is an ace, he checks for possible blackjack before
 allowing for the player to split nor double down.
 When playing `enhc` the dealer does not draw a hole card and
 check for blackjack after the player has played.
 * The `do9` rules means that the player can only double if the
 first two cards sum up nine, ter or eleven.


**Default**
Empty, meaning `ahc`, `h17`, `das`, `doa`.

**Examples**

~~~
rules = ahc h17 das doa
rules = enhc s17 ndas
rules = s17 do9
~~~

# `shuffle_every_hand = ` $b$ {#sec:shuffle_every_hand}

Defines whether the dealer has to re-shuffle the shoe after finishing a hand or not.
If $b$ is `true`, each hand starts from a fresh shoe of size `decks`.
If $b$ is `false`, the shoe is only re-shuffled when the fraction given in `penetration` is achieved.
This setting only makes sense when playing a shoe game, i.e. non-zero `decks`.

**Default**
`false`

**Examples**

~~~
shuffle_every_hand = false
shuffle_every_hand = true
~~~


