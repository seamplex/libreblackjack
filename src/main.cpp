/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - main function
 *
 *  Copyright (C) 2020,2023,2025 jeremy theler
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
#include "dealer.h"
#include "blackjack.h"
#include "players/tty.h"
#include "players/stdinout.h"
#include "players/basic.h"
#include "players/informed.h"

void progress_bar(size_t n, size_t N, int bar_width) {
  float progress = float(n) / N;
  int pos = bar_width * progress;

  std::cout << "\r[";
  for (int i = 0; i < bar_width; ++i) {
    std::cout << ((i < pos) ? "=" : ((i == pos) ? ">" : " "));
  }
  std::cout << "] " << int(progress * 100.0) << " %";
  std::cout.flush();  
}

int main(int argc, char **argv) {
  
  lbj::Configuration conf(argc, argv);

  if (conf.show_version) {
    lbj::shortversion();
    lbj::copyright();
  }
  if (conf.show_help) {
    lbj::help(argv[0]);
  }

  if (conf.show_version || conf.show_help) {
    return 0;
  }  
  
  // simple factory pattern
  // for more dealers we might have a registration mechanism
  lbj::Dealer *dealer = nullptr;
  if (conf.getDealerName() == "blackjack") {
    dealer = new lbj::Blackjack(conf);
  } else {
    std::cerr << "error: unknown dealer for '" << conf.getDealerName() <<"' game" << std::endl;
    return -1;
  }

  // simple factory pattern
  // for more players we might have a registration mechanism
  int progress_bar_width = 0;
  lbj::Player *player = nullptr;
  std::string player_name = conf.getPlayerName();
  if (player_name == "tty") {
    player = new lbj::Tty(conf);
  } else if (player_name == "stdinout" || player_name == "stdio") {
    player = new lbj::StdInOut(conf);
  } else if (player_name == "basic" || player_name == "internal") {
    player = new lbj::Basic(conf);
    progress_bar_width = 50;
  } else if (player_name == "informed") {
    player = new lbj::Informed(conf);
    progress_bar_width = 50;
  } else {
    std::cerr << "error: unknown player '" << player_name <<"'" << std::endl;
    return 1;
  }
  
  // complain if there are configuration options which are not used
  if (conf.checkUsed() != 0) {
    return 1;
  }
  
  // assign player to dealer
  dealer->setPlayer(player);

  // set up progress bar
  const size_t progress_step =  (progress_bar_width) ? dealer->n_hands / progress_bar_width : 0;
  size_t progress_last = 0;
  if (progress_bar_width > 0) {
    progress_bar(0, dealer->n_hands, progress_bar_width);  
  }  
  
  // --- let the action begin! -------------------------------------------------
  size_t n_incorrect_commands = 0;
  dealer->nextAction = lbj::DealerAction::StartNewHand;
  while (!dealer->finished()) {
    dealer->deal();
    if (player->actionRequired != lbj::PlayerActionRequired::None) {
      n_incorrect_commands = 0;
      do {
        if (n_incorrect_commands++ > conf.max_incorrect_commands) {
          std::cerr << "Too many unknown commands." << std::endl;
          return 2;
        }
        player->play();
      } while (dealer->process() <= 0);
    }
    if (progress_bar_width > 0) {
      if ((dealer->n_hand - progress_last) > progress_step) {
        progress_bar(dealer->n_hand, dealer->n_hands, progress_bar_width);
        progress_last = dealer->n_hand;
      }
    }
  }
  // ---------------------------------------------------------------------------
  
  if (progress_bar_width > 0) {
    progress_bar(dealer->n_hands, dealer->n_hands, progress_bar_width);  
    std::cout << std::endl;
  }
  
  player->info(lbj::Info::Bye);
  
  dealer->prepareReport();
  dealer->writeReportYAML();
  
  delete player;
  delete dealer;
  
  return 0;
}
