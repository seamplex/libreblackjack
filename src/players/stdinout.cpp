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
#include <sstream>

#include "../conf.h"
#include "../blackjack.h"
#include "stdinout.h"

namespace lbj {

std::string double_to_string_g_format(double value) {
#ifdef HAS_CPP_20
  // C++20 implementation using std::format
  return std::format("{}", value);
#else
  // Pre-C++20 implementation using stringstream
  std::ostringstream oss;
  oss << value;
  return oss.str();
#endif
}

StdInOut::StdInOut(Configuration &conf) : Player(conf) {

  verbose = false;
  if (conf.exists("verbose")) {
    conf.set(&verbose, {"verbose"});
  }

  return;
}

void StdInOut::info(lbj::Info msg, int p1, int p2) {

  std::string s;

  switch (msg) {

    case lbj::Info::Shuffle:
///inf+shuffling+usage `shuffling`
///inf+shuffling+details The dealer informs that he is shuffling the decks.
///inf+shuffling+details This only happens when a non-zero value for the option `decks` is set.
///inf+shuffling+details If `decks = 0` (or the command-line option `-d0` is passed), then cards
///inf+shuffling+details are drawn randomnly from an infinite set of cards and there is no need to shuffle.
///inf+shuffling+example shuffling
      // TODO: ask the user to cut  
      s = "shuffling";  
    break;

    case lbj::Info::NewHand:
///inf+new_hand+usage `new_hand` $n$ $b$
///inf+new_hand+details The dealer states that a new hand is starting. The integer $n$ gives
///inf+new_hand+details the number of the hand that is about to start (first hand is $n=1$).
///inf+new_hand+details The decimal number $b$ states the player's bankroll before placing
///inf+new_hand+details the bet in the hand that is about to start.
///inf+new_hand+details Even though bets have to be integers, pay offs might be non-integer
///inf+new_hand+details such as when winning a natural (e.g. $3/2 = 1.5$ or $6/5$ = 1.2).    
///inf+new_hand+example new_hand 1 0
///inf+new_hand+example new_hand 22 -8
///inf+new_hand+example new_hand 24998 -7609.5
      s = "new_hand " + std::to_string(p1) + " " + double_to_string_g_format(1e-3*p2);
    break;

    case lbj::Info::BetInvalid:
      if (p1 < 0) {
///inf+bet_negative+usage `bet_negative`
///inf+bet_negative+details The dealer complains that the bet the placer placed is invalid.
///inf+bet_negative+details Only positive integer numbers are allowed.
///inf+bet_negative+details The player will receive a new `bet?` message.
///inf+bet_negative+example bet_negative
        s = "bet_negative" + std::to_string(p1);  
      } else if (p1 > 0) {
///inf+bet_maximum+usage `bet_maximum`
///inf+bet_maximum+details The dealer complains that the bet the placer placed is invalid.
///inf+bet_maximum+details The bet is larger than the maximum wager allowed by `maximum_bet`.
///inf+bet_maximum+details The player will receive a new `bet?` message.
///inf+bet_maximum+example bet_maximum
        s = "bet_maximum" + std::to_string(p1);  
      } else {
///inf+bet_zero+usage `bet_zero`
///inf+bet_zero+details The dealer complains that the bet the placer placed is invalid.
///inf+bet_zero+details Only positive integer numbers are allowed.
///inf+bet_zero+details The player will receive a new `bet?` message.
///inf+bet_zero+example bet_zero
        s = "bet_zero";
      }
    break;
    
    case lbj::Info::CardPlayer:
///inf+card_player+usage `card_player` $rs$ `[` $h$ `]`
///inf+card_player+details The dealer informs that the player has been dealt a card.
///inf+card_player+details The card is given as a two-character ASCII representation where
///inf+card_player+details the first character $r$ indicates the rank and the second 
///inf+card_player+details character $s$ gives the suit.
///inf+card_player+details @
///inf+card_player+details | Character |  Rank            |
///inf+card_player+details |:---------:|------------------|
///inf+card_player+details |    `A`    | Ace              |
///inf+card_player+details |    `2`    | Deuce            |
///inf+card_player+details |    `3`    | Three            |
///inf+card_player+details |    `4`    | Four             |
///inf+card_player+details |    `5`    | Five             |
///inf+card_player+details |    `6`    | Six              |
///inf+card_player+details |    `7`    | Seven            |
///inf+card_player+details |    `8`    | Eight            |
///inf+card_player+details |    `9`    | Nine             |
///inf+card_player+details |    `T`    | Ten              |
///inf+card_player+details |    `J`    | Jack             |
///inf+card_player+details |    `Q`    | Queen            |
///inf+card_player+details |    `J`    | King             |
///inf+card_player+details @
///inf+card_player+details : #tbl:rank Valid rank (first) character
      
///inf+card_player+details | Character |  Suit            |
///inf+card_player+details |:---------:|------------------|
///inf+card_player+details |    `C`    | ♣ Clubs          |
///inf+card_player+details |    `D`    | ♦ Diamonds       |
///inf+card_player+details |    `H`    | ♥ Hearts         |
///inf+card_player+details |    `S`    | ♠ Spades         |
///inf+card_player+details @
///inf+card_player+details : #tbl:suit Valid suit (second) character
///inf+card_player+details @
///inf+card_player+details The optional argument $h$ indicates the id of the player's hand
///inf+card_player+details being dealt. If it not present, that means the base hand.
///inf+card_player+details When performing a splitting on the base hand, the original hand
///inf+card_player+details has id equal to zero and the new hand has id equal to one.
///inf+card_player+details Subsequent splits trigger new hands with sequential ids.
///inf+card_player+example card_player 9C 
///inf+card_player+example card_player JD 
///inf+card_player+example card_player QC
///inf+card_player+example card_player KS
///inf+card_player+example card_player TD 1
///inf+card_player+example card_player 6H 2 
      s = "card_player " + card[p1].ascii() + " " + ((p2 != 0)?(std::to_string(p2)+ " "):"") ;
      break;
    break;

    case lbj::Info::CardDealerUp:
///inf+card_dealer_up+usage `card_dealer_up` $rs$
///inf+card_dealer_up+details The dealer informs that the dealer has been dealt the up card (i.e.
///inf+card_dealer_up+details the first card facing up). This message is issued only once per hand.
///inf+card_dealer_up+details The card is given as the two-character ASCII representation discussed
///inf+card_dealer_up+details in @sec:card_player.
///inf+card_dealer_up+example card_dealer_up KD
///inf+card_dealer_up+example card_dealer_up 7H
///inf+card_dealer_up+example card_dealer_up KH
///inf+card_dealer_up+example card_dealer_up QD
///inf+card_dealer_up+example card_dealer_up 6C
      s = "card_dealer_up " + card[p1].ascii();
    break;

    case lbj::Info::CardDealer:
///inf+card_dealer+usage `card_dealer` $rs$
///inf+card_dealer+details The dealer informs that the dealer has been dealt a card.
///inf+card_dealer+details The card is given as the two-character ASCII representation discussed
///inf+card_dealer+details in @sec:card_player.
///inf+card_dealer+example card_dealer TH
///inf+card_dealer+example card_dealer JC
///inf+card_dealer+example card_dealer 5D
///inf+card_dealer+example card_dealer 5H
///inf+card_dealer+example card_dealer QH
      s = "card_dealer " + card[p1].ascii();
    break;

    case lbj::Info::CardDealerRevealsHole:
///inf+card_dealer_hole+usage `card_dealer_hole` $rs$
///inf+card_dealer_hole+details The dealer informs what his hole card is.
///inf+card_dealer_hole+details This message is issued only if playing the american rules, i.e.
///inf+card_dealer_hole+details with `enhc = false` or `ahc = true`.
///inf+card_dealer_hole+details The card is given as the two-character ASCII representation discussed
///inf+card_dealer_hole+details in @sec:card_player.
///inf+card_dealer_hole+example card_dealer_hole KH
///inf+card_dealer_hole+example card_dealer_hole AC
///inf+card_dealer_hole+example card_dealer_hole 4H
///inf+card_dealer_hole+example card_dealer_hole 5D
///inf+card_dealer_hole+example card_dealer_hole 7H
      s = "card_dealer_hole " + card[p1].ascii();
//      *(++(dealerHand.cards.begin())) = p1;
//      currentHandId = 0;
    break;

    case lbj::Info::DealerBlackjack:
///inf+dealer_blackjack+usage `dealer_blackjack`
///inf+dealer_blackjack+details The dealer informs that he has blackjack.
///inf+dealer_blackjack+example dealer_blackjack
      s = "dealer_blackjack";
    break;

    case lbj::Info::PlayerWinsInsurance:
///inf+player_wins_insurance+usage `player_wins_insurance`
///inf+player_wins_insurance+details The dealer informs that the user won the insurance.
///inf+player_wins_insurance+example player_wins_insurance
      s = "player_wins_insurance";
    break;

    case lbj::Info::PlayerBlackjackAlso:
///inf+player_blackjack_also+usage `player_blackjack_also`
///inf+player_blackjack_also+details The dealer informs that both he and the user have blackjack.
///inf+player_blackjack_also+example player_blackjack_also
      s = "player_blackjack_also";
    break;

    case lbj::Info::PlayerSplitInvalid:
///inf+player_split_invalid+usage `player_split_invalid`
///inf+player_split_invalid+details The dealer complains that the split request cannot be
///inf+player_split_invalid+details fulfilled. Splitting is only possible when exactly two
///inf+player_split_invalid+details cards with the same rank have been dealt in a hand.
///inf+player_split_invalid+details The player will receive a new `play?` message.
///inf+player_split_invalid+example player_split_invalid
      s = "player_split_invalid";
    break;

    case lbj::Info::PlayerSplitOk:
///inf+player_split_ok+usage `player_split_ok` $k$
///inf+player_split_ok+details The dealer informs that the split request was successfully be
///inf+player_split_ok+details fulfilled. The integer $k$ indicates the id of the split hand (the
///inf+player_split_ok+details first hand has an id equal to one so $k>1$ when splitting).
///inf+player_split_ok+example player_split_ok 2
///inf+player_split_ok+example player_split_ok 3
      s = "player_split_ok" + ((p1 != 0)?std::to_string(p1):"");
      hand_to_split = p1;
    break;

    case lbj::Info::PlayerSplitIds:
      s = "new_split_hand " + std::to_string(p2) + " " + card[card_to_split].ascii();
    break;

    case lbj::Info::PlayerDoubleInvalid:
///inf+player_double_invalid+usage `player_double_invalid`
///inf+player_double_invalid+details The dealer complains that the doubling-down request cannot be
///inf+player_double_invalid+details fulfilled. Doubling down is only possible when exactly two
///inf+player_double_invalid+details cards have been dealt in a hand and the `doa` or `do9` option is met.
///inf+player_double_invalid+details The player will receive a new `play?` message.
///inf+player_double_invalid+example player_double_invalid
      s = "player_double_invalid";
    break;

    case lbj::Info::PlayerNextHand:
      s = "player_next_hand";
    break;
    
    case lbj::Info::PlayerPushes:
      s = "player_pushes " + std::to_string(value_player) + " " + std::to_string(value_dealer);
    break;
    
    case lbj::Info::PlayerLosses:
      s = "player_losses " + std::to_string(value_player) + " " + std::to_string(value_dealer);
    break;
    case lbj::Info::PlayerBlackjack:
      s = "blackjack_player";
    break;
    case lbj::Info::PlayerWins:
      s = "player_wins " + std::to_string(value_player) + " " + std::to_string(value_dealer);
    break;
    
    case lbj::Info::NoBlackjacks:
      s = "no_blackjacks";
    break;

    case lbj::Info::DealerBusts:
      s = "dealer_busts " + std::to_string(value_player) + " " + std::to_string(value_dealer);
    break;  
    
    case lbj::Info::Rules:
      s = rules;
    break;

    case lbj::Info::Bankroll:
      s = "bankroll " + std::to_string(1e-3*p1);
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
///int+bet?+usage `bet?`
///int+bet?+details The dealer asks the user the amount to wage in the hand that is about to start.
///int+bet?+details The player should send a positive integer in response.
///int+bet?+details first hand has an id equal to one so $k>1$ when splitting).
///int+bet?+example bet?
      s = "bet?";
    break;

    case lbj::PlayerActionRequired::Insurance:
///int+insurance?+usage `insurance?`
///int+insurance?+details The dealer asks the user if she wants to insure her hand when the dealer's
///int+insurance?+details upcards is an ace.
///int+insurance?+details This message is only sent if `no_insurance` and `always_insure` are both false.
///int+insurance?+details The player should answer either `yes` (or `y`) or `no` (or `n`).
///int+insurance?+example insurance?
      s = "insurance?";
    break;

    case lbj::PlayerActionRequired::Play:
///int+play?+usage `play?` $p$ $d$
///int+play?+details The dealer asks the user to play, i.e. to choose wether to
///int+play?+details @
///int+play?+details  * `pair` (or `p`) or `split`
///int+play?+details  * `double` (or `d`)
///int+play?+details  * `hit` (or `h`)
///int+play?+details  * `stand` (or `s`)
///int+play?+details  * `quit` (or `q`)
///int+play?+details @
///int+play?+details given that the value of the player's hand id $p$ and that the value of the dealer's hand is $d$,
///int+play?+details where $p$ and $d$ are integers. If $p$ is negative, the hand is soft with a value equal to $|p|$.
///int+play?+example play? 17 10
///int+play?+example play? 20 10
///int+play?+example play? -17 3
///int+play?+example play? -19 7
///int+play?+example play? 16 10
///int+play?+example play? -16 10
///int+play?+example play? 16 5
///int+play?+example play? 7 7
      s = "play? " + std::to_string(value_player) + " " + std::to_string(value_dealer);
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
  } else if (command == "rules") {
    actionTaken = lbj::PlayerActionTaken::Rules;
  } else if (command == "help") {
    actionTaken = lbj::PlayerActionTaken::Help;
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
          current_bet = std::stoi(command);
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
