/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - internal basic strategy automatic player
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
#include <fstream>
#include <sstream>

#include "../conf.h"
#include "../blackjack.h"
#include "basic.h"


Basic::Basic(Configuration &conf) {

  for (int value = 0; value < 21; value++) {
    for (int upcard = 0; upcard < 12; upcard++) {
      hard[value][upcard] = Libreblackjack::PlayerActionTaken::None;
      soft[value][upcard] = Libreblackjack::PlayerActionTaken::None;
      pair[value][upcard] = Libreblackjack::PlayerActionTaken::None;
    }
  }
  
  // read in a skeleton of the basic strategy
  std::vector<std::string> default_hard(21);
  std::vector<std::string> default_soft(21);
  std::vector<std::string> default_pair(21);
  
  // TODO: read from file
  default_hard[20] = "  ssssssssss";  
  default_hard[19] = "  ssssssssss";  
  default_hard[18] = "  ssssssssss";  
  default_hard[17] = "  ssssssssss";  
  default_hard[16] = "  ssssshhhsh";  
  default_hard[15] = "  ssssshhhhh";  
  default_hard[14] = "  ssssshhhhh";  
  default_hard[13] = "  ssssshhhhh";  
  default_hard[12] = "  hhssshhhhh";  
  default_hard[11] = "  dddddddddh";  
  default_hard[10] = "  ddddddddhd";  
  default_hard[9]  = "  hddddhhhhh";  
  default_hard[8]  = "  hhhhhhhhhh";  
  default_hard[7]  = "  hhhhhhhhhh";  
  default_hard[6]  = "  hhhhhhhhhh";  
  default_hard[5]  = "  hhhhhhhhhh";  
  default_hard[4]  = "  hhhhhhhhhh";
  
  default_soft[20] = "  ssssssssss";  
  default_soft[19] = "  ssssdsssss";  
  default_soft[18] = "  dddddsshhh";  
  default_soft[17] = "  hddddhhhhh";  
  default_soft[16] = "  hhdddhhhhh";  
  default_soft[15] = "  hhdddhhhhh";  
  default_soft[14] = "  hhhddhhhhh";  
  default_soft[13] = "  hhhhdhhhhh";  
  default_soft[12] = "  hhhhdhhhhh";  
  
  default_pair[20] = "  nnnnnnnnnn";  
  default_pair[18] = "  yyyyynyynn";  
  default_pair[16] = "  yyyyyyyyyy";  
  default_pair[14] = "  yyyyyynnnn";  
  default_pair[12] = "  yyyyynnnnn";  
  default_pair[11] = "  yyyyyyyyyy";  
  default_pair[10] = "  nnnnnnnnnn";  
  default_pair[8]  = "  nnnyynnnnn";  
  default_pair[6]  = "  yyyyyynnnn";  
  default_pair[4]  = "  yyyyyynnnn";  
  
  for (int value = 4; value < 21; value++) {
    for (int upcard = 2; upcard < 12; upcard++) {
        
      if (default_hard[value] != "") {
        switch (default_hard[value][upcard]) {
          case 'h':
            hard[value][upcard] = Libreblackjack::PlayerActionTaken::Hit;  
          break;
          case 's':
            hard[value][upcard] = Libreblackjack::PlayerActionTaken::Stand;  
          break;
          case 'd':
            hard[value][upcard] = Libreblackjack::PlayerActionTaken::Double;  
          break;  
        }
      }

      if (default_soft[value] != "") {
        switch (default_soft[value][upcard]) {
          case 'h':
            soft[value][upcard] = Libreblackjack::PlayerActionTaken::Hit;  
          break;
          case 's':
            soft[value][upcard] = Libreblackjack::PlayerActionTaken::Stand;  
          break;
          case 'd':
            soft[value][upcard] = Libreblackjack::PlayerActionTaken::Double;  
          break;  
        }
      }
      
      if (default_pair[value] != "" && default_pair[value][upcard] == 'y') {
        pair[value][upcard] = Libreblackjack::PlayerActionTaken::Split;  
      }
    }
  }  
  
  
  // read the actual file
  conf.set(strategy_file_path, {"strategy_file_path", "strategy_file", "strategy"});  
  
  // std::ifstream is RAII, i.e. no need to call close
  std::ifstream fileStream(strategy_file_path);
  std::string line;  
  std::string token;
  
  if (fileStream.is_open()) {
    while (getline(fileStream, line)) {
      if (line[0] == '#' || line[0] == ';' || line.empty() || line.find_first_not_of(" \t\r\n") == line.npos) {
        continue;
      }

      auto stream = std::istringstream(line);
      // TODO: check error
      stream >> token;
      
      int value = 0;
      Libreblackjack::PlayerActionTaken (*strategy)[21][12] = nullptr;
      switch (token[0]) {
        case 'h':
          strategy = &hard;
        break;
        case 's':
          strategy = &soft;
        break;
        case 'p':
          strategy = &pair;
          // see below how we handle these two cases
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
          // the pair data is different as it is not written as a function of the value
          // but of the value of the individual cards,
          // i.e. p8 means split a pair of eights and not a hand with two fours
          // to avoid clashing a pair of aces with a pair of sixes, we treat the former differently
          if (value != 11) {
            (*strategy)[2*value][upcard] = Libreblackjack::PlayerActionTaken::Split;  
          } else {
            (*strategy)[11][upcard] = Libreblackjack::PlayerActionTaken::Split;  
          }
        } else if (token == "n") {
          if (value != 11) {
            (*strategy)[2*value][upcard] = Libreblackjack::PlayerActionTaken::None;  
          } else {
            (*strategy)[11][upcard] = Libreblackjack::PlayerActionTaken::None;  
          }
        }
      }
    }
  }
  
  return;
}

int Basic::play() {

  std::size_t value;
  std::size_t upcard;
  
  switch (actionRequired) {
    case Libreblackjack::PlayerActionRequired::Bet:
      currentBet = 1;
      actionTaken = Libreblackjack::PlayerActionTaken::Bet;
    break;

    case Libreblackjack::PlayerActionRequired::Insurance:
      actionTaken = Libreblackjack::PlayerActionTaken::DontInsure;
    break;
    
    case Libreblackjack::PlayerActionRequired::Play:

#ifdef BJDEBUG
      std::cout << "player " << playerValue << " dealer " << dealerValue << std::endl;
#endif      
      value = std::abs(playerValue);
      upcard = std::abs(dealerValue);
      
      // first, we see if we can and shold split
      if (canSplit &&
           ((playerValue == -12 &&    pair[11][upcard] == Libreblackjack::PlayerActionTaken::Split) ||
                                   pair[value][upcard] == Libreblackjack::PlayerActionTaken::Split)) {
          actionTaken = Libreblackjack::PlayerActionTaken::Split;

      } else {
      
        actionTaken = (playerValue < 0) ? soft[value][upcard] : hard[value][upcard];
        
        if (canDouble == false) {
          if (actionTaken == Libreblackjack::PlayerActionTaken::Double) {
            actionTaken = Libreblackjack::PlayerActionTaken::Hit;
          }
        }
      }
      
#ifdef BJDEBUG
      if (actionTaken == Libreblackjack::PlayerActionTaken::Hit) {
        std::cout << "hit" << std::endl;
      } else if (actionTaken == Libreblackjack::PlayerActionTaken::Stand) {
        std::cout << "stand" << std::endl;
      } else if (actionTaken == Libreblackjack::PlayerActionTaken::Split) {
        std::cout << "split" << std::endl;
      } else {
        std::cout << "none" << std::endl;
      }
#endif
      
      
    break;  
    
    case Libreblackjack::PlayerActionRequired::None:
    break;  
    
  }
  
  return 0;
}
