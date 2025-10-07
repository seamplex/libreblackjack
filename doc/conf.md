
* `dealer` (@sec:dealer)
* `decks` (@sec:decks)
* `flat_bet` (@sec:flat_bet)
* `hands` (@sec:hands)
* `maximum_bet` (@sec:maximum_bet)
* `max_incorrect_commands` (@sec:max_incorrect_commands)
* `no_insurance` (@sec:no_insurance)
* `player` (@sec:player)
* `rules` (@sec:rules)


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


