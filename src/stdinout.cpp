#include <iostream>

#include "blackjack.h"
#include "stdinout.h"

int StdInOut::play(Command *command, int *param) {
  std::cout << "what do you want to do" << std::endl;
    
    
  std::string input_buffer;
  std::cin >> input_buffer;
  
  if (input_buffer == "hit" || input_buffer == "h") {
    *command = Command::Hit;
  } else {
    *command = Command::None;
  }
  
  return 0;
}
