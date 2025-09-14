/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - main function
 *
 *  Copyright (C) 2020,2023 jeremy theler
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

#include "conf.h"
#include "base.h"
#include "blackjack.h"
#include "players/tty.h"
#include "players/stdinout.h"
#include "players/basic.h"
#include "players/informed.h"

int main(int argc, char **argv) {
  
  Dealer *dealer = nullptr;
  Player *player = nullptr;
  
  Configuration conf(argc, argv);


  if (conf.show_version) {
    Libreblackjack::shortversion();
    Libreblackjack::copyright();
  }
  if (conf.show_help) {
    Libreblackjack::help(argv[0]);
  }

  if (conf.show_version || conf.show_help) {
    return 0;
  }  
  
  // TODO: think of a better way
  if (conf.getDealerName() == "blackjack") {
    dealer = new Blackjack(conf);
  } else {
    std::cerr << "Unknown dealer for '" << conf.getDealerName() <<"' game" << std::endl;
    return -1;
  }

  // TODO: think of a better way
  std::string player_name = conf.getPlayerName();
  if (player_name == "tty") {
    player = new Tty(conf);
  } else if (player_name == "stdinout" || player_name == "stdio") {
    player = new StdInOut(conf);
  } else if (player_name == "basic" || player_name == "internal") {
    player = new Basic(conf);
  } else if (player_name == "informed") {
    player = new Informed(conf);
  } else {
    std::cerr << "Unknown player '" << player_name <<"'" << std::endl;
    return 1;
  }
  
  // assign player to dealer
  dealer->setPlayer(player);

  // let the action begin!
  unsigned int unknownCommands = 0;
  dealer->nextAction = Libreblackjack::DealerAction::StartNewHand;
  while (!dealer->finished()) {
    dealer->deal();
    if (player->actionRequired != Libreblackjack::PlayerActionRequired::None) {
      unknownCommands = 0;
      do {
        if (unknownCommands++ > conf.max_incorrect_commands) {
          std::cerr << "Too many unknown commands." << std::endl;
          return 2;
        }
        player->play();
      } while (dealer->process() <= 0);
    }
  }
  
  player->info(Libreblackjack::Info::Bye);
  
  dealer->prepareReport();
  dealer->writeReportYAML();
  
  delete player;
  delete dealer;
  
  return 0;
}
