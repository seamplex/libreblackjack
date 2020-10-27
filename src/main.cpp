/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - main function
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

#include "base.h"
#include "blackjack.h"
#include "stdinout.h"

int main(int argc, char **argv) {
  
  Dealer *dealer = nullptr;
  Player *player = nullptr;
  
  // TODO: read the args/conf to know what kind of dealer and player we are having
  // TODO: pass args/conf to the constructor
  dealer = new Blackjack();
  player = new StdInOut();
  
  std::cout << "Let's play" << std::endl;
  dealer->setNextAction(DealerAction::StartNewHand);
  
  Command command{Command::None};
  
  while (!dealer->finished()) {
    dealer->setInputNeeded(false);
    dealer->deal();
    if (dealer->getInputNeeded()) {
      do {
        // TODO: check for too many errors meaning dealer and player do not understand each other
        player->play(&command, nullptr);        
      } while (dealer->process(command) <= 0);
    }
  }
  
  // TODO: write report
  
  delete player;
  delete dealer;
  
}
