/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - stdio player
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

#include "../conf.h"
#include "../blackjack.h"
#include "stdinout.h"

namespace lbj {

StdInOut::StdInOut(Configuration &conf) : Player(conf) {
    
  conf.set(&flat_bet, {"flat_bet", "flatbet"});  
  conf.set(&no_insurance, {"never_insurance", "never_insure", "no_insurance", "dont_insure"});  
  conf.set(&always_insure, {"always_insure"});  
  
  verbose = false;
  if (conf.exists("verbose")) {
    conf.set(&verbose, {"verbose"});
  }
  
  return;
}

void StdInOut::info(lbj::Info msg, int p1, int p2) {
    
  std::string s;
  
  switch (msg) {

    case lbj::Info::BetInvalid:
      if (p1 < 0) {
        s = "bet_negative" + std::to_string(p1);  
      } else if (p1 > 0) {
        s = "bet_maximum" + std::to_string(p1);  
      } else {
        s = "bet_zero";
      }
    break;

    case lbj::Info::NewHand:
      s = "new_hand " + std::to_string(p1) + " " + std::to_string(1e-3*p2);  
      
      // clear dealer's hand
      dealerHand.cards.clear();

      // erase all of our hands
      {
        for (auto hand = hands.begin(); hand != hands.end(); ++hand) {
          hand->cards.clear();
        }
      }
      // create one, add and make it the current one
      hands.clear();
      hands.push_back(std::move(PlayerHand()));
      currentHand = hands.begin();
      currentHandId = 0;
    break;
    
    case lbj::Info::Shuffle:
      // TODO: ask the user to cut  
      s = "shuffling";  
    break;
    
    case lbj::Info::CardPlayer:
      s = "card_player " + card[p1].ascii() + " " + ((p2 != 0)?(std::to_string(p2)+ " "):"") ;
      if (p2 != static_cast<int>(currentHandId)) {
        for (currentHand = hands.begin(); currentHand != hands.end(); ++currentHand) {
          if (static_cast<int>(currentHand->id) == p2) {
            break;
          }
        }
        currentHandId = p2;
      }
      currentHand->cards.push_back(p1);
      break;
    break;
    
    case lbj::Info::CardDealer:
      if (p1 > 0) {
        switch (dealerHand.cards.size()) {
          case 0:
            s = "card_dealer_up " + card[p1].ascii();
          break;
          default:
            s = "card_dealer " + card[p1].ascii();;
          break;
        }
      } else {
        s = "card_dealer " + card[p1].ascii();;
      }
      dealerHand.cards.push_back(p1);
      currentHandId = 0;
    break;
    
    case lbj::Info::CardDealerRevealsHole:
      s = "card_dealer_hole " + card[p1].ascii();;
      *(++(dealerHand.cards.begin())) = p1;
      currentHandId = 0;
    break;
    
    case lbj::Info::DealerBlackjack:
      s = "dealer_blackjack";
    break;
    
    case lbj::Info::PlayerWinsInsurance:
      s = "player_wins_insurance";
    break;
    
    case lbj::Info::PlayerBlackjackAlso:
      s = "player_blackjack_also";
    break;

    case lbj::Info::PlayerSplitInvalid:
      s = "player_split_invalid";
    break;

    case lbj::Info::PlayerSplitOk:
      s = "player_split_ok" + ((p1 != 0)?std::to_string(p1):"");
      handToSplit = p1;
    break;

    case lbj::Info::PlayerSplitIds:

      {
        bool found = false;
        for (auto hand = hands.begin(); hand != hands.end(); ++hand) {
          if (hand->id == handToSplit) {
            found = true;
            hand->id = p1;
            cardToSplit = *(++(hand->cards.begin()));
            hand->cards.pop_back();
            break;
          }
        }
        if (found == false) {
          exit(0); 
        }
      
        // create a new hand
        PlayerHand newHand;
        newHand.id = p2;
        newHand.cards.push_back(cardToSplit);
        hands.push_back(std::move(newHand));
      }  
    
      currentHandId = p1;  
      s = "new_split_hand " + std::to_string(p2) + " " + card[cardToSplit].ascii();
    break;

    case lbj::Info::PlayerDoubleInvalid:
      s = "player_double_invalid";
    break;
    
    case lbj::Info::PlayerNextHand:
      s = "player_next_hand";
    break;
    
    case lbj::Info::PlayerPushes:
      s = "player_pushes " + std::to_string(playerValue) + " " + std::to_string(dealerValue);;
    break;
    
    case lbj::Info::PlayerLosses:
      s = "player_losses " + std::to_string(playerValue) + " " + std::to_string(dealerValue);
    break;
    case lbj::Info::PlayerBlackjack:
      s = "blackjack_player";
    break;
    case lbj::Info::PlayerWins:
      s = "player_wins " + std::to_string(playerValue) + " " + std::to_string(dealerValue);;
    break;
    
    case lbj::Info::NoBlackjacks:
      s = "no_blackjacks";
    break;

    case lbj::Info::DealerBusts:
      s = "dealer_busts " + std::to_string(playerValue) + " " + std::to_string(dealerValue);;
    break;  
    
    case lbj::Info::Help:
      // TODO: help
      std::cout << "help yourself" << std::endl;        
    break;

    case lbj::Info::Bankroll:
      std::cout << "bankroll " << std::to_string(1e-3*p1) << std::endl;        
    break;
    
    
    case lbj::Info::CommandInvalid:
      s = "command_invalid";
    break;
    
    case lbj::Info::Bye:
      s = "bye";  
    break;
    
    case lbj::Info::None:
    break;
    
  }
  
  std::cout << s << std::endl;
  
  return;
}

int StdInOut::play(void) {
  
  std::string s;
  
  switch (actionRequired) {
    case lbj::PlayerActionRequired::Bet:
      s = "bet?";  
    break;

    case lbj::PlayerActionRequired::Insurance:
      s = "insurance?";  
    break;
    
    case lbj::PlayerActionRequired::Play:
      s = "play? " + std::to_string(playerValue) + " " + std::to_string(dealerValue);
    break;  
    
    case lbj::PlayerActionRequired::None:
    break;
    
  }

  std::cout << s << std::endl;
  
  std::string command;
  std::cin >> command;

  if (std::cin.eof()) {
    actionTaken = lbj::PlayerActionTaken::Quit;
    return 0;
  }

  trim(command);
  actionTaken = lbj::PlayerActionTaken::None;
    
    // check common commands first
           if (command == "quit" || command == "q") {
      actionTaken = lbj::PlayerActionTaken::Quit;
    } else if (command == "help") {
      actionTaken = lbj::PlayerActionTaken::Help;
//    } else if (command == "count" || command == "c") {
//      actionTaken = lbj::PlayerActionTaken::Count;
    } else if (command == "upcard" || command == "u") {
      actionTaken = lbj::PlayerActionTaken::UpcardValue;
    } else if (command == "bankroll" || command == "b") {
      actionTaken = lbj::PlayerActionTaken::Bankroll;
    } else if (command == "hands") {
      actionTaken = lbj::PlayerActionTaken::Hands;
    }
    
    if (actionTaken == lbj::PlayerActionTaken::None) {
      switch (actionRequired) {

        case lbj::PlayerActionRequired::Bet:
          if (isdigit(command[0])) {
            currentBet = std::stoi(command);
            actionTaken = lbj::PlayerActionTaken::Bet;
          }
        break;

        case lbj::PlayerActionRequired::Insurance:
          if (command == "y" || command == "yes") {
            actionTaken = lbj::PlayerActionTaken::Insure;
          } else if (command == "n" || command == "no") {
            actionTaken = lbj::PlayerActionTaken::DontInsure;
          } else {
            // TODO: chosse if we allow not(yes) == no
            actionTaken = lbj::PlayerActionTaken::None;  
          }
        break;

        case lbj::PlayerActionRequired::Play:
          // TODO: sort by higher-expected response first
                 if (command == "h" || command =="hit") {
            actionTaken = lbj::PlayerActionTaken::Hit;
          } else if (command == "s" || command == "stand") {
            actionTaken = lbj::PlayerActionTaken::Stand;
          } else if (command == "d" || command == "double") {
            actionTaken = lbj::PlayerActionTaken::Double;
          } else if (command == "p" || command == "split" || command == "pair") {
            actionTaken = lbj::PlayerActionTaken::Split;
          } else {
            actionTaken = lbj::PlayerActionTaken::None;
          }
        break;
        
        case lbj::PlayerActionRequired::None:
        break;
        
      }
    }

  
  return 0;

}
}