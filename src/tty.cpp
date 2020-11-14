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

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "conf.h"
#include "blackjack.h"
#include "tty.h"

// TODO: make class static
std::vector<std::string> commands;


// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}


Tty::Tty(Configuration &conf) {
    
  Libreblackjack::shortversion();
//   Libreblackjack::copyright();

  conf.set(&flat_bet, {"flat_bet", "flatbet"});  
  conf.set(&no_insurance, {"no_insurance", "dont_insure"});  

  if (commands.size() == 0) {
//    commands.push_back("help");
    commands.push_back("hit");
    commands.push_back("stand");
    commands.push_back("double");
    commands.push_back("split");
    commands.push_back("pair");
    commands.push_back("yes");
    commands.push_back("no");
    commands.push_back("quit");
  }
  
  if (conf.exists("color")) {
    conf.set(&color, {"color"});
  }
  
  if (color) {
    black   = "\x1B[0m";
    red     = "\x1B[31m";
    green   = "\x1B[32m";
    yellow  = "\x1B[33m";
    blue    = "\x1B[34m";
    magenta = "\x1B[35m";
    cyan    = "\x1B[36m";
    white   = "\x1B[37m";
    reset   = "\033[0m";
  }
  
  prompt = cyan + " > " + reset;
  
  return;
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
      std::cout << std::endl;
      s = "Starting new hand, bankroll " + std::to_string(intData);
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
    
    case Info::CardDealerRevealsHole:
//      s = "card_dealer_hole";
      s = "Dealer's hole card was " + card[intData].utf8();
      *(++(dealerHand.cards.begin())) = intData;
//      renderTable();  
    break;
    
    case Info::DealerBlackjack:
//      s = "dealer_blackjack";
      s = "Dealer has Blackjack";
    break;
    
    case Info::PlayerWinsInsurance:
//      s = "player_wins_insurance";
      s = "Player wins insurance";
    break;
    
    case Info::PlayerBlackjackAlso:
//      s = "player_blackjack_also";
      s = "Player also has Blackjack";
      renderTable();  
    break;
    
    case Info::PlayerPushes:
//      s = "player_pushes";
      s = "Player pushes";
      renderTable();  
    break;
    
    case Info::PlayerLosses:
//      s = "player_losses";
      s = "Player losses";
      renderTable();  
    break;
    case Info::PlayerBlackjack:
//      s = "blackjack_player";
      s = "Player has Blackjack";
      renderTable();  
    break;
    case Info::PlayerWins:
//      s = "player_wins";
      s = "Player wins " + std::to_string(intData);
      renderTable();  
    break;
    
    case Info::NoBlackjacks:
//      s = "no_blackjacks";
      s = "No blackjacks";
    break;
    
    case Info::PlayerBustsAllHands:
//      s = "player_busted_all_hands";
      if (hands.size() == 1) {
        s = "Player busted";
      } else {  
        s = "Player busted all hands";
      }
      renderTable();  
    break;
    
    case Info::DealerBusts:
//      s = "no_blackjacks";
      s = "Dealer busts!";
      renderTable();  
    break;  
    
    case Info::Help:
      std::cout << "help yourself" << std::endl;        
    break;
    
    
    case Info::Bye:
//      s = "bye";  
      s = "Bye bye! We'll play Blackjack again next time.";
    break;
    
    case Info::None:
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
      renderTable();  
      s = "Insurance?";  
    break;
    
    case PlayerActionRequired::Play:
      renderTable();  
      s = "Play?";
    break;  
    
    case PlayerActionRequired::None:
    break;
    
  }
  
  if (s != "") {
    if (delay > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }  
    std::cout << yellow << " <-- " << s << reset << std::endl;
  }
    
#ifdef HAVE_LIBREADLINE
  rl_attempted_completion_function = rl_completion;  
  if ((input_buffer = readline(prompt.c_str())) == nullptr) {
      
    // EOF means "quit"
    actionTaken = PlayerActionTaken::Quit;
    std::cout << std::endl;
    
  } else {

    add_history(input_buffer);
    actionTaken = PlayerActionTaken::None;

    // TODO: better solution
    std::string command = input_buffer;
    trim(command);
    
    
    // check common commands first
           if (command == "quit" || command == "q") {
      actionTaken = PlayerActionTaken::Quit;
    } else if (command == "help") {
      actionTaken = PlayerActionTaken::Help;
    } else if (command == "count" || command == "c") {
      actionTaken = PlayerActionTaken::Count;
    } else if (command == "upcard" || command == "u") {
      actionTaken = PlayerActionTaken::UpcardValue;
    } else if (command == "bankroll" || command == "b") {
      actionTaken = PlayerActionTaken::Bankroll;
    } else if (command == "hands") {
      actionTaken = PlayerActionTaken::Hands;
    }
    
    if (actionTaken == PlayerActionTaken::None) {
      switch (actionRequired) {

        case PlayerActionRequired::Bet:
          currentBet = std::stoi(input_buffer);
          actionTaken = PlayerActionTaken::Bet;
        break;

        case PlayerActionRequired::Insurance:
          if (command == "y" || command == "yes") {
            actionTaken = PlayerActionTaken::Insure;
          } else if (command == "n" || command == "no") {
            actionTaken = PlayerActionTaken::DontInsure;
          } else {
            // TODO: chosse if we allow not(yes) == no
            actionTaken = PlayerActionTaken::None;  
          }
        break;

        case PlayerActionRequired::Play:
          // TODO: sort by higher-expected response first
                 if (command == "h" || command =="hit") {
            actionTaken = PlayerActionTaken::Hit;
          } else if (command == "s" || command == "stand") {
            actionTaken = PlayerActionTaken::Stand;
          } else if (command == "d" || command == "double") {
            actionTaken = PlayerActionTaken::Stand;
          } else if (command == "p" || command == "split" || command == "pair") {
            actionTaken = PlayerActionTaken::Split;
          } else {
            actionTaken = PlayerActionTaken::None;
          }
        break;
        
        case PlayerActionRequired::None:
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

void Tty::renderTable(void) {

  std::cout << " -- Dealer's hand:  --------" << std::endl;
  renderHand(&dealerHand);
  std::cout << "    Total: " << dealerHand.total() << std::endl;

  std::cout << " -- Player's hand --------" << std::endl;
  for (auto hand : hands) {
    renderHand(&hand);
    std::cout << "    Total: " << hand.total() << std::endl;
  }  

  return;
}

void Tty::renderHand(Hand *hand) {

  for (unsigned int i = 0; i < hand->cards.size(); i++) {
    std::cout << " _____   ";
  }
  std::cout << std::endl;
  
  unsigned int i = 0;
  for (auto it : hand->cards) {
    if (it >= 0) {
      std::cout << "|" << card[it].getNumberASCII() << ((card[it].number != 10)?" ":"") << "   |  ";
    } else {
      std::cout << "|#####|  ";
    }
    i++;
  }
  std::cout << std::endl;

  i = 0;
  for (auto it : hand->cards) {
    if (it >= 0) {
      std::cout << "|     |  ";
    } else {
      std::cout << "|#####|  ";
    }
    i++;
  }
  std::cout << std::endl;
  
  i = 0;
  for (auto it : hand->cards) {
    if (it >= 0) {
      std::cout << "|  " << card[it].getSuitUTF8() << "  |  ";
    } else {
      std::cout << "|#####|  ";
    }
    i++;
  }
  std::cout << std::endl;
  
  i = 0;
  for (auto it : hand->cards) {
    if (it >= 0) {
      std::cout << "|     |  ";
    } else {
      std::cout << "|#####|  ";
    }
    i++;
  }
  std::cout << std::endl;

  i = 0;
  for (auto it : hand->cards) {
    if (it >= 0) {
      std::cout << "|___" << ((card[it].number != 10)?"_":"") << card[it].getNumberASCII() << "|  ";
    } else {
      std::cout << "|#####|  ";
    }
    i++;
  }
  std::cout << std::endl;
  
  return;
    
}


int Tty::list_index = 0;
int Tty::len = 0;

char *Tty::rl_command_generator(const char *text, int state) {
    
  if (!state) {
    list_index = 0;
    len = strlen(text);
  }

  for (unsigned int i = list_index; i < commands.size(); i++) {
    if (commands[i].compare(0, len, text) == 0) {
      list_index = i+1;
      return strdup(commands[i].c_str());
    }
  }

  return NULL;
}

char **Tty::rl_completion(const char *text, int start, int end) {
  char **matches = NULL;

#ifdef HAVE_LIBREADLINE
  matches = rl_completion_matches(text, rl_command_generator);
#endif

  return matches;
}
