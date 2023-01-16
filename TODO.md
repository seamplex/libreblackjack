 * re-write docs in pandoc + filters instead of m4
 * cpu and wall time in report
 * check that the card distribution is uniform
 * initial bankroll
 * report
   * format (yaml, json, markdown table)
   * verbosity (extra small, small, medium, large, extra large)
 * flag to see if a conf string was used or not
 * progress bar
 * max_splits through conf (default 3)
 * double after split
 * name of the game the dealer deals
 * name of the games the player can play
 * dealers
   * ENHC
   * blackjack switch
   * blackjack under?
   * siete y medio?
   * card war
   * between
 * players
   * internal reko
   * runtime-linked in a shared object
 
 * multithreading
 * optimize using const and restrict 

 * questions
    1. how to play?
    2. what is the expected value? page 41 griffin
 * mimic the dealer -5.5% single s17
 * referencess in griffin chapter 2 to blackjack history
 * definition of bs p42
 * thorp wiki/page
 * remove cards from deck
 * +10 par de 5, 12 hard 12, -12 par de aces
 * eric farmer's blog  <https://possiblywrong.wordpress.com/>
 * bankroll history, group by 10, 100, 1000, etc
 * betting systems, positive, negative, martingale fibonacci
 * kelly when ev is pos
 * analize martingale, bankroll needed to avoid ruin 90% and 95% of the times
 * impossibilty of gambling systems, von mises <https://en.wikipedia.org/wiki/Impossibility_of_a_gambling_system>
 * model shuffle master
 * re-compute results from the wizard
 * martingale and positive progression
 * basic strategy vs horrible player at the same time
 * websockets ux (sini?)
 * tournaments
 * conf
    - bankroll_history_file_path
    - removed_cards
 * dbus?


## Links 

 * https://www.qfit.com/book/ModernBlackjackPage44.htm
 * https://www.qfit.com/book/ModernBlackjackPage68.htm
 * https://www.beatblackjack.org/en/simulate/
 * https://www.blackjackinfo.com/free-blackjack-combinatorial-analyzer/
 * http://www.blackjack-scams.com/html/prog__systems.html
 * https://curlie.org/Games/Gambling/Blackjack
 * https://www.blackjackincolor.com/
 * https://www.blackjackinfo.com/
 * https://www.gnu.org/help/evaluation.html 
 * https://github.com/imneme/pcg-c
 * https://wizardofodds.com/gambling/betting-systems/cancellation/appendix/
 * split eights http://centronsoftware.com/john_patrick/tips.asp?id=10
 * http://centronsoftware.com/john_patrick/tips.asp?id=9
 * http://mikefarah.github.io/yq/
 * https://gist.github.com/pkuczynski/8665367
 
