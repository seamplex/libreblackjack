/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - tty interactive player
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
#include <fstream>
#include <sstream>

#include "conf.h"
#include "blackjack.h"
#include "internal.h"


Internal::Internal(Configuration &conf) {
    
  hard.resize(21);  // 4--20
  soft.resize(21);  // 12--20
  pair.resize(12);  // 2--11
  
  for (int value = 0; value < 21; value++) {
    hard[value].resize(12);
    soft[value].resize(12);
    if (value < 12) {
      pair[value].resize(12);
    }
    for (int upcard = 0; upcard < 12; upcard++) {
      hard[value][upcard] = Libreblackjack::PlayerActionTaken::None;
      soft[value][upcard] = Libreblackjack::PlayerActionTaken::None;
      if (value < 12) {
        pair[value][upcard] = Libreblackjack::PlayerActionTaken::None;
      }
    }
  }
  // TODO: read a default basic strategy
  
  // std::ifstream is RAII, i.e. no need to call close
  std::ifstream fileStream("bs.txt");
  std::string line;  
  std::string token;
  
  if (fileStream.is_open()) {
    while (getline(fileStream, line)) {
      if (line[0] == '#' || line[0] == ';' || line.empty()) {
        continue;
      }

      auto stream = std::istringstream(line);
      // TODO: check error
      stream >> token;
      
      int value = 0;
      std::vector<std::vector<Libreblackjack::PlayerActionTaken>> *strategy = nullptr;
      switch (token[0]) {
        case 'h':
          strategy = &hard;
        break;
        case 's':
          strategy = &soft;
        break;
        case 'p':
          strategy = &pair;
          if (token[1] == 'A') {
            value = 11;  
          } else if (token[1] == 'T') {
            value = 10;
          }
        break;
      }

      if (value == 0) {
        value = std::stoi(token.substr(1));
      }
      
      for (int upcard = 2; upcard < 12; upcard++) {
      // TODO: check error
        stream >> token;
        if (token == "h") {
          (*strategy)[value][upcard] = Libreblackjack::PlayerActionTaken::Hit;  
        } else if (token == "s") {
          (*strategy)[value][upcard] = Libreblackjack::PlayerActionTaken::Stand;  
        } else if (token == "d") {
          (*strategy)[value][upcard] = Libreblackjack::PlayerActionTaken::Double;  
        } else if (token == "y") {
          (*strategy)[value][upcard] = Libreblackjack::PlayerActionTaken::Split;  
        } else if (token == "n") {
          (*strategy)[value][upcard] = Libreblackjack::PlayerActionTaken::None;  
        }
      }
    }
  }
  
  return;
}

void Internal::info(Libreblackjack::Info msg, int p1, int p2) {
  return;
}

int Internal::play() {

  switch (actionRequired) {
    case Libreblackjack::PlayerActionRequired::Bet:
      currentBet = 1;
      actionTaken = Libreblackjack::PlayerActionTaken::Bet;
    break;

    case Libreblackjack::PlayerActionRequired::Insurance:
      actionTaken = Libreblackjack::PlayerActionTaken::DontInsure;
    break;
    
    case Libreblackjack::PlayerActionRequired::Play:

//      std::cout << "player " << playerValue << " dealer " << dealerValue << std::endl;
        
      // TODO: split
        
      // soft
      {
        std::size_t value = std::abs(playerValue);
        std::size_t upcard = std::abs(dealerValue);
        actionTaken = (playerValue < 0) ? soft[value][upcard] : hard[value][upcard];
        
        // TODO: double  
        if (actionTaken == Libreblackjack::PlayerActionTaken::Double) {
          actionTaken = Libreblackjack::PlayerActionTaken::Hit;
        }
      }  
/*      
      if (actionTaken == Libreblackjack::PlayerActionTaken::Hit) {
        std::cout << "hit" << std::endl;
      } else if (actionTaken == Libreblackjack::PlayerActionTaken::Stand) {
        std::cout << "stand" << std::endl;
      } else {
        std::cout << "none" << std::endl;
      }
*/    
    break;  
    
    case Libreblackjack::PlayerActionRequired::None:
    break;  
    
  }
  
  return 0;
}
