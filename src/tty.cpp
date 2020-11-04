#include <iostream>
#include <cstring>

#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "blackjack.h"
#include "tty.h"

Tty::Tty(void) {
#ifdef HAVE_LIBREADLINE
  prompt = cyan + " > " + reset;
#endif
}

int Tty::play() {
  
#ifdef HAVE_LIBREADLINE
    
  if ((input_buffer = readline(prompt.c_str())) == nullptr) {
      
    // EOF means "quit"
    actionTaken = PlayerActionTaken::Quit;
    std::cout << std::endl;
    
  } else {

    add_history(input_buffer);
    actionTaken = PlayerActionTaken::None;

  // TODO: convertir a string y usar algo comun para non-readline
    // check common commands first
    if (strcmp(input_buffer, "quit") == 0 || strcmp(input_buffer, "q")== 0) {
      actionTaken = PlayerActionTaken::Quit;
    } else if (strcmp(input_buffer, "help") == 0) {
      actionTaken = PlayerActionTaken::Help;
    } else if (strcmp(input_buffer, "count") == 0 || strcmp(input_buffer, "c")== 0) {
      actionTaken = PlayerActionTaken::Count;
    } else if (strcmp(input_buffer, "upcard") == 0 || strcmp(input_buffer, "u")== 0) {
      actionTaken = PlayerActionTaken::UpcardValue;
    } else if (strcmp(input_buffer, "bankroll") == 0 || strcmp(input_buffer, "b")== 0) {
      actionTaken = PlayerActionTaken::Bankroll;
    } else if (strcmp(input_buffer, "hands") == 0) {
      actionTaken = PlayerActionTaken::Hands;
    } else if (strcmp(input_buffer, "table") == 0) {
      actionTaken = PlayerActionTaken::Table;
    }
    
    if (actionTaken == PlayerActionTaken::None) {
      switch (actionRequired) {

        case PlayerActionRequired::Bet:
          currentBet = atoi(input_buffer);
          actionTaken = PlayerActionTaken::Bet;
        break;

        case PlayerActionRequired::Insurance:
          if (strcmp(input_buffer, "y") == 0 || strcmp(input_buffer, "yes") == 0) {
            actionTaken = PlayerActionTaken::Insure;
          } else if (strcmp(input_buffer, "n") == 0 || strcmp(input_buffer, "no") == 0) {
            actionTaken = PlayerActionTaken::DontInsure;
          } else {
            // TODO: chosse if we allow not(yes) == no
            actionTaken = PlayerActionTaken::None;  
          }
        break;

        case PlayerActionRequired::Play:

          // TODO: sort by higher-expected response first
          if (strcmp(input_buffer, "h") == 0 || strcmp(input_buffer, "hit") == 0) {
            actionTaken = PlayerActionTaken::Hit;
          } else if (strcmp(input_buffer, "s") == 0 || strcmp(input_buffer, "stand") == 0) {
            actionTaken = PlayerActionTaken::Stand;
          } else if (strcmp(input_buffer, "d") == 0 || strcmp(input_buffer, "double") == 0) {
            actionTaken = PlayerActionTaken::Stand;
          } else if (strcmp(input_buffer, "p") == 0 || strcmp(input_buffer, "pair") == 0 || strcmp(input_buffer, "split") == 0) {
            actionTaken = PlayerActionTaken::Split;
          } else {
            actionTaken = PlayerActionTaken::None;
          }
        break;
      }
    }
      
    free(input_buffer);
  }
  
#else

  std::cout << prompt;
  std::cin >> input_buffer;

  // TODO: check EOF
  // TODO: esto puede ir en algo comun para tty y stdout
  if (input_buffer == "hit" || input_buffer == "h") {
    *command = Command::Hit;
  } else {
    *command = Command::None;
  }
  
#endif
  
  return 0;
}
