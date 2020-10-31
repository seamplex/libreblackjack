#include <iostream>

#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif


#include "blackjack.h"
#include "stdinout.h"

StdInOut::StdInOut(void) {
}


int StdInOut::play() {
  
  std::cin >> input_buffer;

  // TODO: check EOF
/*  
  if (input_buffer == "hit" || input_buffer == "h") {
    *command = Command::Hit;
  } else {
    *command = Command::None;
  }
*/
  return 0;
}
