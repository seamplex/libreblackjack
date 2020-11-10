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
#include <cstring>
#include <thread>
#include <chrono>

#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "conf.h"
#include "blackjack.h"
#include "tty.h"

Tty::Tty(Configuration &conf) {
    
  conf.set(&flat_bet, {"flat_bet", "flatbet"});  
  conf.set(&no_insurance, {"no_insurance", "dont_insure"});  

  // TODO: check conf for colors
  prompt = cyan + " > " + reset;
}

void Tty::info(Info msg, int intData) {
  std::string s;
  
  // TODO: choose utf8 or other representation
  
  switch (msg) {
      
    case Info::InvalidBet:
      if (intData < 0) {
//      s = "bet_negative";  
        s = "Your bet is negative (" + std::to_string(intData) + ")";
      } else if (intData > 0) {
//      s = "bet_maximum";  
        s = "Your bet is larger than the maximum allowed (" + std::to_string(intData) + ")";
      } else {
//      s = "bet_zero";
        s = "Your bet is zero";
      }
    break;

    case Info::NewHand:
//      s = "new_hand";  
      s = "Starting new hand #" + std::to_string(intData);
      dealerHand.cards.clear();
    break;
    
    case Info::Shuffle:
//      s = "shuffle";  
      s = "Deck needs to be shuffled.";
    break;
    case Info::CardPlayer:
      switch (currentHand->cards.size()) {
        case 1:
//          s = "card_player_first";
          s = "Player's first card is " + card[intData].utf8();
        break;
        case 2:
//          s = "card_player_second";
          s = "Player's second card is " + card[intData].utf8();
        break;
        default:
//          s = "card_player";
          s = "Player's card is " + card[intData].utf8();
        break;
      } 
    break;
    
    case Info::CardDealer:
      if (intData != -1) {  
        switch (dealerHand.cards.size()) {
          case 0:
//            s = "card_dealer_up";
            s = "Dealer's up card is " + card[intData].utf8();
          break;
          default:
//            s = "card_dealer";
            s = "Dealer's card is " + card[intData].utf8();
          break;
        }
      } else {
        s = "Dealer's hole card is dealt";
      }
      dealerHand.cards.push_back(intData);
    break;
    
    case Info::CardDealerHoleRevealed:
//      s = "card_dealer_hole";
      s = "Dealer's hole card was " + card[intData].utf8();
      *(++(dealerHand.cards.begin())) = intData;
    break;
    
    case Info::DealerBlackjack:
//      s = "dealer_blackjack";
      s = "Dealer has Blackjack";
      // TODO: draw dealer's hand
    break;
    
    case Info::PlayerWinsInsurance:
//      s = "player_wins_insurance";
      s = "Player wins insurance";
    break;
    
    case Info::PlayerBlackjackAlso:
//      s = "player_blackjack_also";
      s = "Player also has Blackjack";
    break;
    case Info::PlayerPushes:
//      s = "player_pushes";
      s = "Player pushes";
      // TODO:
      //  print_hand_art (player->current_hand);
    break;
    case Info::PlayerLosses:
//      s = "player_losses";
      s = "Player losses";
      // TODO:
      //  print_hand_art (player->current_hand);
    break;
    case Info::PlayerBlackjack:
//      s = "blackjack_player";
      s = "Player has Blackjack";
      // TODO:
      //  print_hand_art (player->current_hand);
    break;
    case Info::PlayerWins:
//      s = "player_wins";
      s = "Player wins " + std::to_string(intData);
    break;
    
    case Info::NoBlackjacks:
//      s = "no_blackjacks";
      s = "No blackjacks";
    break;
    
    case Info::PlayerBustedAllHands:
//      s = "player_busted_all_hands";
      if (hands.size() == 1) {
        s = "Player busted";
      } else {  
        s = "Player busted all hands";
      }
    break;
    
    case Info::Bye:
//      s = "bye";  
      s = "Bye bye! We'll play Blackjack again next time.";
    break;
  }
  
  if (delay > 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
  }  
  std::cout << green << s << reset << std::endl;
  
  return;
}

int Tty::play() {

  std::string s;  
  switch (actionRequired) {
    case PlayerActionRequired::Bet:
      s = "Bet?";  
    break;

    case PlayerActionRequired::Insurance:
      s = "Insurance?";  
    break;
    
    case PlayerActionRequired::Play:
      std::cout << " -- Dealer's hand:  --------" << std::endl;
      render(&dealerHand);
      std::cout << "    Total: " << dealerHand.total() << std::endl;

      std::cout << " -- Player's hand --------" << std::endl;
      for (auto hand : hands) {
        render(&hand);
        std::cout << "    Total: " << hand.total() << std::endl;
      }  
      s = "Play?";
    break;  
  }
  
  if (s != "") {
    if (delay > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }  
    std::cout << yellow << " <-- " << s << reset << std::endl;
  }
    
#ifdef HAVE_LIBREADLINE
    
  if ((input_buffer = readline(prompt.c_str())) == nullptr) {
      
    // EOF means "quit"
    actionTaken = PlayerActionTaken::Quit;
    std::cout << std::endl;
    
  } else {

    add_history(input_buffer);
    actionTaken = PlayerActionTaken::None;

  // TODO: convertir a string y usar algo comun para non-readline
    // check common commands first
    if (strcmp(input_buffer, "quit") == 0 || strcmp(input_buffer, "q")== 0) {
      actionTaken = PlayerActionTaken::Quit;
    } else if (strcmp(input_buffer, "help") == 0) {
      actionTaken = PlayerActionTaken::Help;
    } else if (strcmp(input_buffer, "count") == 0 || strcmp(input_buffer, "c")== 0) {
      actionTaken = PlayerActionTaken::Count;
    } else if (strcmp(input_buffer, "upcard") == 0 || strcmp(input_buffer, "u")== 0) {
      actionTaken = PlayerActionTaken::UpcardValue;
    } else if (strcmp(input_buffer, "bankroll") == 0 || strcmp(input_buffer, "b")== 0) {
      actionTaken = PlayerActionTaken::Bankroll;
    } else if (strcmp(input_buffer, "hands") == 0) {
      actionTaken = PlayerActionTaken::Hands;
    } else if (strcmp(input_buffer, "table") == 0) {
      actionTaken = PlayerActionTaken::Table;
    }
    
    if (actionTaken == PlayerActionTaken::None) {
      switch (actionRequired) {

        case PlayerActionRequired::Bet:
          currentBet = std::stoi(input_buffer);
          actionTaken = PlayerActionTaken::Bet;
        break;

        case PlayerActionRequired::Insurance:
          if (strcmp(input_buffer, "y") == 0 || strcmp(input_buffer, "yes") == 0) {
            actionTaken = PlayerActionTaken::Insure;
          } else if (strcmp(input_buffer, "n") == 0 || strcmp(input_buffer, "no") == 0) {
            actionTaken = PlayerActionTaken::DontInsure;
          } else {
            // TODO: chosse if we allow not(yes) == no
            actionTaken = PlayerActionTaken::None;  
          }
        break;

        case PlayerActionRequired::Play:

          // TODO: sort by higher-expected response first
          if (strcmp(input_buffer, "h") == 0 || strcmp(input_buffer, "hit") == 0) {
            actionTaken = PlayerActionTaken::Hit;
          } else if (strcmp(input_buffer, "s") == 0 || strcmp(input_buffer, "stand") == 0) {
            actionTaken = PlayerActionTaken::Stand;
          } else if (strcmp(input_buffer, "d") == 0 || strcmp(input_buffer, "double") == 0) {
            actionTaken = PlayerActionTaken::Stand;
          } else if (strcmp(input_buffer, "p") == 0 || strcmp(input_buffer, "pair") == 0 || strcmp(input_buffer, "split") == 0) {
            actionTaken = PlayerActionTaken::Split;
          } else {
            actionTaken = PlayerActionTaken::None;
          }
        break;
      }
    }
      
    free(input_buffer);
  }
  
#else

  std::cout << prompt;
  std::cin >> input_buffer;

  // TODO: check EOF
  // TODO: esto puede ir en algo comun para tty y stdout
  if (input_buffer == "hit" || input_buffer == "h") {
    *command = Command::Hit;
  } else {
    *command = Command::None;
  }
  
#endif
  
  return 0;
}



void Tty::render(Hand *hand) {

  for (auto it : hand->cards) {
    std::cout << " _____   ";
  }
  std::cout << std::endl;
  
  unsigned int i = 0;
  for (auto it : hand->cards) {
    if (it > 0) {
      std::cout << "|" << card[it].getNumberASCII() << ((card[it].number != 10)?" ":"") << "   |  ";
    } else {
      std::cout << "|#####|  ";
    }
    i++;
  }
  std::cout << std::endl;

  i = 0;
  for (auto it : hand->cards) {
    if (it > 0) {
      std::cout << "|     |  ";
    } else {
      std::cout << "|#####|  ";
    }
    i++;
  }
  std::cout << std::endl;
  
  i = 0;
  for (auto it : hand->cards) {
    if (it > 0) {
      std::cout << "|  " << card[it].getSuitUTF8() << "  |  ";
    } else {
      std::cout << "|#####|  ";
    }
    i++;
  }
  std::cout << std::endl;
  
  i = 0;
  for (auto it : hand->cards) {
    if (it > 0) {
      std::cout << "|     |  ";
    } else {
      std::cout << "|#####|  ";
    }
    i++;
  }
  std::cout << std::endl;

  i = 0;
  for (auto it : hand->cards) {
    if (it > 0) {
      std::cout << "|___" << ((card[it].number != 10)?"_":"") << card[it].getNumberASCII() << "|  ";
    } else {
      std::cout << "|#####|  ";
    }
    i++;
  }
  std::cout << std::endl;
  
  return;
    
}
