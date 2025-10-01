
* `bet?` (@sec:bet)
* `insurance?` (@sec:insurance)
* `play?` (@sec:play)


# `bet?` {#sec:bet}

The dealer asks the user the amount to wage in the hand that is about to start.
The player should send a positive integer in response.
first hand has an id equal to one so $k>1$ when splitting).

**Examples**

~~~
bet?
~~~

# `insurance?` {#sec:insurance}

The dealer asks the user if she wants to insure her hand when the dealer's
upcards is an ace.
This message is only sent if `no_insurance` and `always_insure` are both false.
The player should answer either `yes` (or `y`) or `no` (or `n`).

**Examples**

~~~
insurance?
~~~

# `play?` $p$ $d$ {#sec:play}

The dealer asks the user to play, i.e. to choose wether to

 * `pair` (or `p`)
 * `double` (or `d`)
 * `hit` (or `h`)
 * `stand` (or `s`)

given that the value of the player's hand id $p$ and that the value of the dealer's hand is $d$,
where $p$ and $d$ are integers. If $p$ is negative, the hand is soft with a value equal to $|p|$.

**Examples**

~~~
play? 17 10
play? 20 10
play? -17 3
play? -19 7
play? 16 10
play? -16 10
play? 16 5
play? 7 7
~~~


