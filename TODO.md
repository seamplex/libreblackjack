# high importance

 * RSA
 * ES10
 * read strategy from file
 * multiple players
 * clean up command line help, -c or no?
 * rules in report
 
# medium

 * error handler, explain what the valid choices are (i.e. unknown rules)
 * update docs (like feenox)
 * cpu and wall time in report

# old

 * check that the card distribution is uniform
   - i.e. statistiscal tests
 * initial bankroll
 * report
   * add yaml filters to get JSON (`jq`), markdown table (`awk`), etc.
   * verbosity (extra small, small, medium, large, extra large)
 * progress bar
 * max_splits through conf (default 3)
 * name of the game the dealer deals
 * name of the games the player can play
 * dealers
   * blackjack switch
   * blackjack under?
   * siete y medio?
   * card war
   * between
 * players
   * random hit/stand
   * internal reko
   * runtime-linked in a shared object
 * conf
    - bankroll_history_file_path
    - removed_cards
    - chance of getting first cards
 
 * multithreading (not sure)
 * optimize using const and restrict 

 * questions
    1. how to play?
    2. what is the expected value? page 41 griffin

# Stuff    
    
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
 
