/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - standard blackjack dealer
 *
 *  Copyright (C) 2020 jeremy theler
 *
 *  This file is part of Libre Blackjack.
 *
 *  Libre Blackjack is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Libre Blackjack is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Libre Blackjack.  If not, see <http://www.gnu.org/licenses/>.
 *------------------- ------------  ----    --------  --     -       -         -
 */

#include <iostream>

#include "blackjack.h"

Blackjack::Blackjack() {
  std::cout << "I'm your Blackjack dealer!" << std::endl;
}

Blackjack::~Blackjack() {
  std::cout << "Bye bye! We'll play Blackjack again next time." << std::endl;
}

void Blackjack::deal() {
  std::cout << "Here are your cards" << std::endl;
  setInputNeeded(true);
}

// returns zero if it is a common command and we need to ask again
// returns positive if what was asked was answered
// returns negative if what was aked was not asnwered or the command does not apply
int Blackjack::process(Command command) {
  
  switch (command) {
    case Command::Hit:
      std::cout << "ok, you hit" << std::endl;
      finished(true);
      return 1;
      break;
    case Command::None:
      std::cout << "I don't undertand you" << std::endl;
      return 0;
      break;
  }
  
  return 0;
}
