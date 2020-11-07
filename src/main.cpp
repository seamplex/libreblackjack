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
#include <unistd.h>

#include "base.h"
#include "conf.h"
#include "blackjack.h"
#include "tty.h"
#include "stdinout.h"

int main(int argc, char **argv) {
  
  Dealer *dealer = nullptr;
  Player *player = nullptr;
  
  Configuration conf(argc, argv);
  
  // TODO: read the args/conf to know what kind of dealer and player we are having
  // TODO: pass args/conf to the constructor
  dealer = new Blackjack(conf);
  if (isatty(1)) {
    player = new Tty(conf);
  } else {
    player = new StdInOut();
  }
  // TODO: player strategy from file

  dealer->nextAction = DealerAction::StartNewHand;
  
  while (!dealer->finished()) {
    dealer->deal(player);
    if (player->actionRequired != PlayerActionRequired::None) {
      do {
        // TODO: check for too many errors meaning dealer and player do not understand each other
        player->play();
      } while (dealer->process(player) <= 0);
    }
  }
  
  // TODO: write report
  
  delete player;
  delete dealer;
  
}
