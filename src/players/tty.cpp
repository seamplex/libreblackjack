/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - tty interactive player
 *
 *  Copyright (C) 2020, 2023 jeremy theler
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
#include <thread>
#include <chrono>

#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "../conf.h"
#include "../blackjack.h"
#include "tty.h"

namespace lbj {

    // TODO: make class static
std::vector<std::string> commands;

// https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
template<typename ... Args>
std::string string_format2( const std::string& format, Args ... args)
{
  size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
  if (size <= 0) {
    return std::string("");
  }
  std::unique_ptr<char[]> buf(new char[size]); 
  snprintf(buf.get(), size, format.c_str(), args ...);
  return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

Tty::Tty(Configuration &conf) {
    
  shortversion();
//   lbj::copyright();

  conf.set(&flat_bet, {"flat_bet", "flatbet"});  
  conf.set(&no_insurance, {"never_insurance", "never_insure", "no_insurance", "dont_insure"});  
  conf.set(&always_insure, {"always_insure"});  
  conf.set(&delay, {"delay"});  

  if (commands.size() == 0) {
//    commands.push_back("help");
    commands.push_back("hit");
    commands.push_back("stand");
    commands.push_back("double");
    commands.push_back("split");
    commands.push_back("pair");
    commands.push_back("yes");
    commands.push_back("no");
    commands.push_back("bankroll");
    commands.push_back("quit");
  }
  
  verbose = true;
  if (conf.exists("verbose")) {
    conf.set(&verbose, {"verbose"});
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

void Tty::info(lbj::Info msg, int p1, int p2) {
  std::string s;
  bool render = false;
  
  // TODO: choose utf8 or other representation
  
  switch (msg) {

    case lbj::Info::BetInvalid:
      if (p1 < 0) {
        s = "Your bet is negative (" + std::to_string(p1) + ")";
      } else if (p1 > 0) {
        s = "Your bet is larger than the maximum allowed (" + std::to_string(p1) + ")";
      } else {
        s = "Your bet is zero";
      }
    break;

    case lbj::Info::NewHand:
      std::cout << std::endl;
      s = "Starting new hand #" + std::to_string(p1) + " with bankroll " + string_format2("%g", 1e-3*p2);
      
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
      s = "Deck needs to be shuffled.";
    break;
    
    case lbj::Info::CardPlayer:
      if (p2 != static_cast<int>(currentHandId)) {
        for (currentHand = hands.begin(); currentHand != hands.end(); ++currentHand) {
          if (static_cast<int>(currentHand->id) == p2) {
            break;
          }
        }
        currentHandId = p2;
      }
      currentHand->cards.push_back(p1);
      s = "Player's card" + ((p2 != 0)?(" in hand #"+std::to_string(p2)):"") + " is " + card[p1].utf8();
      break;
    break;
    
    case lbj::Info::CardDealer:
      if (p1 > 0) {
        switch (dealerHand.cards.size()) {
          case 0:
            s = "Dealer's up card is " + card[p1].utf8();
          break;
          default:
            s = "Dealer's card is " + card[p1].utf8();
          break;
        }
      } else {
        s = "Dealer's hole card is dealt";
      }
      dealerHand.cards.push_back(p1);
      currentHandId = 0;
    break;
    
    case lbj::Info::CardDealerRevealsHole:
      s = "Dealer's hole card was " + card[p1].utf8();
      *(++(dealerHand.cards.begin())) = p1;
      currentHandId = 0;
    break;
    
    case lbj::Info::DealerBlackjack:
      s = "Dealer has Blackjack";
    break;
    
    case lbj::Info::PlayerWinsInsurance:
      s = "Player wins insurance";
    break;
    
    case lbj::Info::PlayerBlackjackAlso:
      s = "Player also has Blackjack";
      render = true;
    break;

    case lbj::Info::PlayerSplitInvalid:
      s = "Cannot split";
    break;

    case lbj::Info::PlayerSplitOk:
      s = "Splitting hand" + ((p1 != 0)?(" #" + std::to_string(p1)):"");
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
    
      s = "Creating new hand #" + std::to_string(p2) + " with card " + card[cardToSplit].utf8();
      currentHandId = p1;  
    break;

    case lbj::Info::PlayerDoubleInvalid:
      s = "Cannot double down";
    break;
    
    case lbj::Info::PlayerNextHand:
      s = "Playing next hand #" + std::to_string(p1);
      render = true;
    break;
    
    case lbj::Info::PlayerPushes:
      s = "Player pushes " + string_format2("%g", 1e-3*p1) + ((p2 > 0) ? (" with " + std::to_string(p2)) : "");
      render = true;
    break;
    
    case lbj::Info::PlayerLosses:
      s = "Player losses " + string_format2("%g", 1e-3*p1) + ((p2 > 0) ? (" with " + std::to_string(p2)) : "");
      render = true;
    break;
    case lbj::Info::PlayerBlackjack:
      s = "Player has Blackjack";
      render = true;
    break;
    case lbj::Info::PlayerWins:
      s = "Player wins " + string_format2("%g", 1e-3*p1) + ((p2 > 0) ? (" with " + std::to_string(p2)) : "");
      render = true;
    break;
    
    case lbj::Info::NoBlackjacks:
      s = "No blackjacks";
    break;

    case lbj::Info::DealerBusts:
      s = "Dealer busts with " + std::to_string(p1);
    break;  
    
    case lbj::Info::Help:
      std::cout << "help yourself" << std::endl;        
    break;

    case lbj::Info::Bankroll:
      std::cout << "Your bankroll is " << string_format2("%g", 1e-3*p1) << std::endl;        
    break;
    
    
    case lbj::Info::CommandInvalid:
      s = "Invalid command";
    break;
    
    case lbj::Info::Bye:
      s = "Bye bye! We'll play Blackjack again next time";
    break;
    
    case lbj::Info::None:
    break;
    
  }
  
  if (delay > 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
  }  
  std::cout << green << s << reset << std::endl;

  if (render) {
    renderTable();
  }
  
  return;
}

int Tty::play() {

  std::string s;  
  switch (actionRequired) {
    case lbj::PlayerActionRequired::Bet:
      s = "Bet?";  
    break;

    case lbj::PlayerActionRequired::Insurance:
      renderTable();  
      s = "Insurance?";  
    break;
    
    case lbj::PlayerActionRequired::Play:
      renderTable();  
      s = "Play? " + std::to_string(playerValue) + " " + std::to_string(dealerValue);
    break;  
    
    case lbj::PlayerActionRequired::None:
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
    actionTaken = lbj::PlayerActionTaken::Quit;
    std::cout << std::endl;
    return 0;
  }
  
  add_history(input_buffer);
  std::string command = input_buffer;
  free(input_buffer);
  trim(command);
#else
  
  std::cout << prompt;
  std::string command;
  std::cin >> command;
    
#endif  
  
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

void Tty::renderTable(void) {

  std::cout << " -- Dealer's hand:  --------" << std::endl;
  renderHand(&dealerHand);
  std::cout << "    Value: " << ((dealerHand.value() < 0)?"soft ":"") << std::abs(dealerHand.value()) << std::endl;

  std::cout << " -- Player's hand --------" << std::endl;
  for (auto hand : hands) {
    renderHand(&hand, (hand.id != 0) && (hand.id == currentHandId));
    std::cout << "    Value: " << ((hand.value() < 0)?"soft ":"") << std::abs(hand.value()) << std::endl;
  }  

  return;
}

void Tty::renderHand(Hand *hand, bool current) {

  std::string ansiColor;
  std::string ansiReset;
  
  for (unsigned int i = 0; i < hand->cards.size(); i++) {
    std::cout << " _____   ";
  }
  std::cout << std::endl;
    
  for (auto c : hand->cards) {
    if (color && (card[c].suit == lbj::Suit::Diamonds || card[c].suit == lbj::Suit::Hearts)) {
      ansiColor = red;
      ansiReset = reset;
    } else {
      ansiColor = "";
      ansiReset = "";
    }
    
    if (c > 0) {
      std::cout << "|" << ansiColor << card[c].getNumberASCII() << ((card[c].number != 10)?" ":"") << ansiReset << "   |  ";
    } else {
      std::cout << "|#####|  ";
    }
  }
  std::cout << std::endl;

  for (auto c : hand->cards) {
    if (c > 0) {
      std::cout << "|     |  ";
    } else {
      std::cout << "|#####|  ";
    }
  }
  std::cout << std::endl;
  
  for (auto c : hand->cards) {
    if (color && (card[c].suit == lbj::Suit::Diamonds || card[c].suit == lbj::Suit::Hearts)) {
      ansiColor = red;
      ansiReset = reset;
    } else {
      ansiColor = "";
      ansiReset = "";
    }
    
    if (c > 0) {
      std::cout << "|  " << ansiColor << card[c].getSuitUTF8() << ansiReset << "  |  ";
    } else {
      std::cout << "|#####|  ";
    }
  }
  if (current) {
    std::cout << cyan << "<-- current hand" << reset;
  }
  std::cout << std::endl;
  
  for (auto c : hand->cards) {
    if (c > 0) {
      std::cout << "|     |  ";
    } else {
      std::cout << "|#####|  ";
    }
  }
  std::cout << std::endl;

  for (auto c : hand->cards) {
    if (color && (card[c].suit == lbj::Suit::Diamonds || card[c].suit == lbj::Suit::Hearts)) {
      ansiColor = red;
      ansiReset = reset;
    } else {
      ansiColor = "";
      ansiReset = "";
    }
      
    if (c > 0) {
      std::cout << "|___" << ansiColor << ((card[c].number != 10)?"_":"") << card[c].getNumberASCII() << ansiReset<< "|  ";
    } else {
      std::cout << "|#####|  ";
    }
  }
  std::cout << std::endl;
  
  return;
    
}

int Tty::list_index = 0;
int Tty::len = 0;

char *Tty::rl_command_generator(const char *text, int state) {
#ifdef HAVE_LIBREADLINE
    
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
#endif
  return NULL;
}

char **Tty::rl_completion(const char *text, int start, int end) {
  char **matches = NULL;
#ifdef HAVE_LIBREADLINE
  matches = rl_completion_matches(text, rl_command_generator);
#endif
  return matches;
}
}