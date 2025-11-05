/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - internal basic strategy automatic player
 *
 *  Copyright (C) 2020, 2023, 2025 jeremy theler
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

namespace lbj {

Basic::Basic(Configuration &conf) : Player(conf) {

  for (int value = 0; value < 21; value++) {
    for (int upcard = 0; upcard < 12; upcard++) {
      hard[value][upcard] = PlayerActionTaken::None;
      soft[value][upcard] = PlayerActionTaken::None;
      pair[value][upcard] = PlayerActionTaken::None;
    }
  }
  
  // read in a skeleton of the basic strategy
  std::vector<std::string> default_hard(21);
  std::vector<std::string> default_soft(21);
  std::vector<std::string> default_pair(21);
  
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
            hard[value][upcard] = PlayerActionTaken::Hit;  
          break;
          case 's':
            hard[value][upcard] = PlayerActionTaken::Stand;  
          break;
          case 'd':
            hard[value][upcard] = PlayerActionTaken::Double;  
          break;  
        }
      }

      if (default_soft[value] != "") {
        switch (default_soft[value][upcard]) {
          case 'h':
            soft[value][upcard] = PlayerActionTaken::Hit;  
          break;
          case 's':
            soft[value][upcard] = PlayerActionTaken::Stand;  
          break;
          case 'd':
            soft[value][upcard] = PlayerActionTaken::Double;  
          break;  
        }
      }
      
      if (default_pair[value] != "" && default_pair[value][upcard] == 'y') {
        pair[value][upcard] = PlayerActionTaken::Split;  
      }
    }
  }  
  
  
  // read the actual file
  conf.set(strategy_file_path, {"strategy_file_path", "strategy_file", "strategy"});  
  
  // std::ifstream is RAII, i.e. no need to call close
  std::ifstream file_stream(strategy_file_path);
  
  if (file_stream.is_open()) {
    std::string line;
    int line_num = 0;
    while (getline(file_stream, line)) {
      line_num++;
      // TODO: trim?
      if (line[0] == '#' || line[0] == ';' || line.empty() || line.find_first_not_of(" \t\r\n") == line.npos) {
        continue;
      }

      std::string token;
      auto stream = std::istringstream(line);
      // TODO: check error
      stream >> token;
      
      int value = 0;
      PlayerActionTaken (*strategy)[21][12] = nullptr;
      switch (token[0]) {
        case 'h':
        case 'H':
          strategy = &hard;
        break;
        case 's':
        case 'S':
          strategy = &soft;
        break;
        case 'p':
        case 'P':
          strategy = &pair;
          // see below how we handle these two cases
          if (token[1] == 'A' || token[1] == 'a') {
            value = 11;  
          } else if (token[1] == 'T' || token[1] == 't') {
            value = 10;
          }
        break;
        default:
          std::cerr << "error: either h (hard), s (soft) or p (pair) expected as the first character in " << strategy_file_path << ":" << line_num << std::endl;
          exit(1);
        break;
      }

      if (value == 0) {
        value = std::stoi(token.substr(1));
      }
      if (value == 0 || value > 20) {
        std::cerr << "error: unknown value in " << strategy_file_path << ":" << line_num << std::endl;
        exit(1);
      }
      
      for (int upcard = 2; upcard < 12; upcard++) {
        // TODO: check error
        stream >> token;
        if (token == "h" || token == "H") {
          (*strategy)[value][upcard] = PlayerActionTaken::Hit;  
        } else if (token == "s" || token == "S") {
          (*strategy)[value][upcard] = PlayerActionTaken::Stand;  
        } else if (token == "d" || token == "D") {
          (*strategy)[value][upcard] = PlayerActionTaken::Double;  
        } else if (token == "y" || token == "Y") {
          // the pair data is different as it is not written as a function of the value
          // but of the value of the individual cards,
          // i.e. p8 means split a pair of eights and not a hand with two fours
          // to avoid clashing a pair of aces with a pair of sixes, we treat the former differently
          if (value != 11) {
            (*strategy)[2*value][upcard] = PlayerActionTaken::Split;  
          } else {
            (*strategy)[11][upcard] = PlayerActionTaken::Split;  
          }
        } else if (token == "n" || token == "N") {
          if (value != 11) {
            (*strategy)[2*value][upcard] = PlayerActionTaken::None;  
          } else {
            (*strategy)[11][upcard] = PlayerActionTaken::None;  
          }
        } else {
          std::cerr << "error: unknown command '" << token << "' in " << strategy_file_path << ":" << line_num << std::endl;
          exit(1);
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
    case PlayerActionRequired::Bet:
      current_bet = 1;
      actionTaken = PlayerActionTaken::Bet;
    break;

    case PlayerActionRequired::Insurance:
      actionTaken = PlayerActionTaken::DontInsure;
    break;
    
    case PlayerActionRequired::Play:

#ifdef BJDEBUG
      std::cout << "player " << value_player << " dealer " << value_dealer << std::endl;
#endif      
      value = std::abs(value_player);
      upcard = std::abs(value_dealer);
      
      // first, we see if we can and shold split
      if (can_split &&
           ((value_player == -12 &&    pair[11][upcard] == PlayerActionTaken::Split) ||
                                   pair[value][upcard] == PlayerActionTaken::Split)) {
          actionTaken = PlayerActionTaken::Split;

      } else {
      
        actionTaken = (value_player < 0) ? soft[value][upcard] : hard[value][upcard];
        
        if (can_double == false) {
          if (actionTaken == PlayerActionTaken::Double) {
            actionTaken = PlayerActionTaken::Hit;
          }
        }
      }
      
#ifdef BJDEBUG
      if (actionTaken == PlayerActionTaken::Hit) {
        std::cout << "hit" << std::endl;
      } else if (actionTaken == PlayerActionTaken::Stand) {
        std::cout << "stand" << std::endl;
      } else if (actionTaken == PlayerActionTaken::Split) {
        std::cout << "split" << std::endl;
      } else {
        std::cout << "none" << std::endl;
      }
#endif
      
      
    break;  
    
    case PlayerActionRequired::None:
    break;  
    
  }
  
  return 0;
}
}
