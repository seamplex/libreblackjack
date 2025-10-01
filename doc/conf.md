
* `dealer` (@sec:dealer)
* `max_incorrect_commands` (@sec:max_incorrect_commands)
* `player` (@sec:player)


# `dealer = ` *game* {#sec:dealer}

Defines the game the dealer will deal.
Currently, the only valid choice is `blackjack`.

**Default**
`blackjack`

**Examples**

~~~
dealer = blackjack
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


