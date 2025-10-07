/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - standard blackjack dealer
 *
 *  Copyright (C) 2020, 2025 jeremy theler
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
#include <utility>
#include <random>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <list>

#include "blackjack.h"

namespace lbj {
Blackjack::Blackjack(Configuration &conf) : Dealer(conf), rng(dev_random()), fiftyTwoCards(1, 52) {

///conf+hands+usage `hands = ` $n$
///conf+hands+details Sets the number of hands to play before quiting.
///conf+hands+details If $n$ is zero, the program keeps playing until it receives the command `quit`.
///conf+hands+details Otherwise it plays $n$ hands and quits.
///conf+hands+details This parameter can be set on the command line with the option `-n`$n$ or `--hands=`$n$.
///conf+hands+default $0$
///conf+hands+example hands = 1
///conf+hands+example hands = 1000000
///conf+hands+example hands = 1e6
  conf.set(&n_hands, {"n_hands", "hands"});

///conf+decks+usage `decks = ` $n$
///conf+decks+details Sets the number of decks used in the game.
///conf+decks+details If $n$ is zero, the program draws cards from an infinte set.
///conf+decks+details For a finite $n$, the cards are drawn from a shoe.
///conf+decks+default $0$
///conf+decks+example decks = 1
///conf+decks+example decks = 2
///conf+decks+example decks = 8
  conf.set(&n_decks, {"decks", "n_decks"});

///conf+maximum_bet+usage `maximum_bet = ` $n$
///conf+maximum_bet+details Sets a limit on the size of the bet.
///conf+maximum_bet+details If a bet larger than the limit is placed, the dealer answers
///conf+maximum_bet+details `bet_maximum` and asks again. A value of 0 means no limit.
///conf+maximum_bet+details This is only useful when modeling variable-betting schemes.
///conf+maximum_bet+default $0$
///conf+maximum_bet+example maximum_bet = 0
///conf+maximum_bet+example maximum_bet = 1
///conf+maximum_bet+example maximum_bet = 20
  conf.set(&max_bet, {"maximum_bet", "max_bet", "maxbet"});

///conf+rules+usage `rules = [ ahc | enhc ] [ h17 | s17 ] [ das | ndas ] [ doa | do9 ]`
///conf+rules+details Defines the rules of the game.
///conf+rules+details @
///conf+rules+details | Rule                                     |   Yes   |   No    |
///conf+rules+details |:-----------------------------------------|:-------:|:-------:|
///conf+rules+details | Dealer peeks for blackjack               |  `ahc`  |  `enhc` |
///conf+rules+details | Dealer has to hit a soft 17              |  `h17`  |  `s17`  |
///conf+rules+details | Player can double after splitting        |  `das`  |  `ndas` |
///conf+rules+details | Player can double on any first two cards |  `doa`  |  `do9`  |
///conf+rules+details @
///conf+rules+details  * When playing `ahc` (default), the dealer has a hole card.
///conf+rules+details  If the upcard is an ace, he checks for possible blackjack before
///conf+rules+details  allowing for the player to split nor double down.
///conf+rules+details  When playing `enhc` the dealer does not draw a hole card and
///conf+rules+details  check for blackjack after the player has played.
///conf+rules+details  * The `do9` rules means that the player can only double if the
///conf+rules+details  first two cards sum up nine, ter or eleven.
///conf+rules+details @
///conf+rules+default Empty, meaning `ahc`, `h17`, `das`, `doa`.
///conf+rules+example rules = ahc h17 das doa
///conf+rules+example rules = enhc s17 ndas
///conf+rules+example rules = s17 do9
  if (conf.exists("rules")) {
    std::istringstream iss(conf.getString("rules"));
    std::string token;
    while(iss >> token) {
      // I cannot believe there is no case-insensitive string comparison in C++
      if (token == "enhc" || token == "ENHC") {
        enhc = true;
      } else if (token == "ahc" || token == "AHC") {
        enhc = false;
      } else if (token == "h17" || token == "H17") {
        h17 = true;
      } else if (token == "s17" || token == "S17") {
        h17 = false;
      } else if (token == "das" || token == "DAS") {
        das = true;
      } else if (token == "ndas" || token == "NDAS") {
        das = false;
      } else if (token == "doa" || token == "DOA") {
        doa = true;
      } else if (token == "do9" || token == "DO9") {
        doa = false;
/*        
      } else if (token == "rsa" || token == "RSA") {
        rsa = true;
      } else if (token == "nrsa" || token == "NRSA") {
        rsa = false;
*/
      } else {
        std::cerr << "error: unknown rule " << token << std::endl;
        exit(1);
      }
    }
    conf.markUsed("rules");
  }

//  conf.set(&rsa, {"rsa", "resplit_aces"});

  conf.set(&blackjack_pays, {"blackjack_pays"});

  conf.set(&playerStats.bankroll, {"bankroll", "initial_bankroll"});

  // parent class_
  conf.set(&number_of_burnt_cards, {"number_of_burnt_cards", "n_burnt_cards", "burnt_cards"});
  conf.set(&penetration, {"penetration"});
  conf.set(&penetration_sigma, {"penetration_sigma", "penetration_dispersion"});
  conf.set(&shuffle_every_hand, {"shuffle", "shuffle_every_hand"});
  conf.set(&quit_when_arranged_cards_run_out, {"quit_when_arranged_cards_run_out"});
  conf.set(&new_hand_reset_cards, {"new_hand_reset_cards"});

  // read arranged cards
  if (conf.exists("cards")) {
    if (conf.exists("cards_file")) {
      std::cerr << "error: cannot have both cards and cards_file" << std::endl;
      exit(1);
    }
    std::istringstream iss(conf.getString("cards"));
    if (read_arranged_cards(std::move(iss)) != 0) {
      exit(1);
    }
    conf.markUsed("cards");
  }

  if (conf.exists("cards_file")) {
    if (conf.exists("cards")) {
      std::cerr << "error: cannot have both cards and cards_file" << std::endl;
      exit(1);
    }

    std::ifstream file(conf.getString("cards_file"));
    if (!file.is_open()) {
      std::cerr << "error: opening file " << conf.getString("cards_file") << std::endl;
      exit(1);
    }
    std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::istringstream iss(file_content);
    if (read_arranged_cards(std::move(iss)) != 0) {
      exit(1);
    }
    conf.markUsed("cards_file");
  }

  n_arranged_cards = arranged_cards.size();

  bool explicit_seed = conf.set(&rng_seed, {"rng_seed", "seed"});
  if (explicit_seed) {
    rng = std::mt19937(rng_seed);
  }

  // initialize shoe and perform initial shuffle
  if (n_decks > 0) {
    shoe.reserve(52*n_decks);
    for (unsigned int deck = 0; deck < n_decks; deck++) {
      for (unsigned int tag = 1; tag <= 52; tag++) {
        shoe.push_back(tag);
      }
    }
    shuffle();
    cut_card_position = static_cast<size_t>(penetration * 52 * n_decks);
  }
}

Blackjack::~Blackjack() {
  return;    
}

Player::Player(Configuration &conf) {
///conf+flat_bet+usage `flat_bet = ` $b$
///conf+flat_bet+details Tells both the dealer and the player that the betting scheme is flat or not.
///conf+flat_bet+details The dealer will not ask for bets and the internal player, if asked, always says `1`.
///conf+flat_bet+details The value can be either `false` or `true` or `0` or `1`.
///conf+flat_bet+default $false$
///conf+flat_bet+example flat_bet = false
///conf+flat_bet+example flat_bet = true
///conf+flat_bet+example flat_bet = 1
  conf.set(&flat_bet, {"flat_bet", "flatbet"});  

///conf+no_insurance+usage `no_insurance = ` $b$
///conf+no_insurance+details If $b$ is `true`, the dealer will not ask for insurance and assume
///conf+no_insurance+details the player will never take it when the dealer shows an ace.
///conf+no_insurance+details The value can be either `false` or `true` or `0` or `1`.
///conf+no_insurance+default $false$
///conf+no_insurance+example no_insurance = false
///conf+no_insurance+example no_insurance = true
///conf+no_insurance+example no_insurance = 1
  conf.set(&no_insurance, {"no_insurance", "never_insurance", "never_insure", "dont_insure"});  
  conf.set(&always_insure, {"always_insure"});  
}

int Blackjack::read_arranged_cards(std::istringstream iss) {
  std::string token;
  while(iss >> token) {

    // TODO: move to cards.cpp  
    bool number = true;
    for (char c : token) {
      number &= std::isdigit(c);
    }

    int n = 0;
    if (number) {
      n = std::stoi(token);
      if (n <= 0 || n > 52) {
        // TODO: negative values are placeholders for random cards
        std::cerr << "error: invalid integer card " << token << std::endl;
        return 1;
      }
    } else {
      char rank = token[0];
      char suit = token[1];
      if (rank == 'A') {
        n = 1;    
      } else if (rank == 'T') {
        n = 10;    
      } else if (rank == 'J') {
        n = 11;    
      } else if (rank == 'Q') {
        n = 12;    
      } else if (rank == 'K') {
        n = 13;    
      } else {
        n = rank - '0';
      }
      if (n < 1 || n > 13) {
        std::cerr << "error: invalid ASCII card rank " << token << std::endl;
        return 1;
      }

      if (suit != '\0') {
        if (suit == 'C') {
          n += static_cast<int>(lbj::Suit::Clubs) * 13;
        } else if (suit == 'D') {
          n += static_cast<int>(lbj::Suit::Diamonds) * 13;
        } else if (suit == 'H') {
          n += static_cast<int>(lbj::Suit::Hearts) * 13;
        } else if (suit == 'S') {
          n += static_cast<int>(lbj::Suit::Spades) * 13;
        } else {
          std::cerr << "error: invalid ASCII card suit " << token << std::endl;
          return 1;
        }
      }
    }

    if (n == 0) {
      std::cerr << "error: invalid card " << token << std::endl;
    }

    arranged_cards.push_back(n);
  }
  return 0;
}

void Blackjack::can_double_split(void) {
  int n_cards = playerStats.currentHand->cards.size();
  player->can_double = (n_cards == 2);
  if (das == false) {
    player->can_double &= (playerStats.splits == 0);
  }
  if (doa == false) {
    int value = playerStats.currentHand->value();
    player->can_double &= (value == 9 || value == 10 || value == 11);
  }

  player->can_split = (n_cards == 2) && (card[*(playerStats.currentHand->cards.begin())].value == card[*(++playerStats.currentHand->cards.begin())].value) && (playerStats.splits < resplits);
  return;
}


void Blackjack::deal(void) {

  bool player_blackjack = false;
  // let's start by assuming the player does not need to do anything
  player->actionRequired = lbj::PlayerActionRequired::None;

  switch(nextAction) {
    // -------------------------------------------------------------------------  
    case lbj::DealerAction::StartNewHand:

      // check if we are done
      if (n_hands > 0 && n_hand >= n_hands) {
        finished(true);
        return;
      }

      if (n_hand != 0) {
        updateMeanAndVariance();
      }

      if (new_hand_reset_cards) {
        i_arranged_cards = 0;
      }
      playerStats.currentOutcome = 0;
      n_hand++;

      // clear dealer's hand
      hand.cards.clear();

      // erase all the player's hands, create one, add and make it the current one
      for (auto &player_hand : playerStats.hands) {
        player_hand.cards.clear();
      }
      playerStats.hands.clear();
      playerStats.hands.push_back(std::move(PlayerHand()));
      playerStats.currentHand = playerStats.hands.begin();

      // state that the player did not win anything nor split nor doubled down
      playerStats.splits = 0;

      if (last_pass || shuffle_every_hand) {
        info(lbj::Info::Shuffle);

        // shuffle the shoe
        shuffle();

        // burn as many cards as asked
        pos += number_of_burnt_cards;
        last_pass = false;
      }

      info(lbj::Info::NewHand, n_hand, 1e3*playerStats.bankroll);
#ifdef BJDEBUG
      std::cout << "new hand #" << n_hand << std::endl;
#endif

      if (player->flat_bet) {

        // TODO: check bankroll
        playerStats.currentHand->bet = player->flat_bet;
        // take player's money
        playerStats.bankroll -= playerStats.currentHand->bet;
        if (playerStats.bankroll < playerStats.worstBankroll) {
          playerStats.worstBankroll = playerStats.bankroll;
        }
        playerStats.totalMoneyWaged += playerStats.currentHand->bet;

        player->actionRequired = lbj::PlayerActionRequired::None;
        nextAction = lbj::DealerAction::DealPlayerFirstCard;

      } else {

        player->actionRequired = lbj::PlayerActionRequired::Bet;
        nextAction = lbj::DealerAction::None;
        
      }

      return;
      
    break;
    
    // -------------------------------------------------------------------------  
    case lbj::DealerAction::DealPlayerFirstCard:
      // where's step 2? <- probably that's the player's bet
      // step 3. deal the first card to each player
      player_first_card = draw(&(*playerStats.currentHand));
      info(lbj::Info::CardPlayer, player_first_card);
#ifdef BJDEBUG
      std::cout << "first card " << card[player_first_card].utf8() << std::endl;
#endif
      // step 4. show dealer's upcard
      dealer_up_card = draw(&hand);
      info(lbj::Info::CardDealer, dealer_up_card);
#ifdef BJDEBUG
      std::cout << "up card " << card[dealer_up_card].utf8() << std::endl;
#endif
      player->value_dealer = hand.value();

      // step 5. deal the second card to each player
      player_second_card = draw(&(*playerStats.currentHand));
      info(lbj::Info::CardPlayer, player_second_card);
      player->value_player = playerStats.currentHand->value();
#ifdef BJDEBUG
      std::cout << "second card " << card[player_second_card].utf8() << std::endl;
#endif
      
      if (enhc == false) {
        // step 6. deal the dealer's hole card 
        dealer_hole_card = draw(&hand);
        info(lbj::Info::CardDealer);

        // step 7.a. if the upcard is an ace ask for insurance
        if (card[dealer_up_card].value == 11) {
          if (player->no_insurance == false && player->always_insure == false) {
            player->actionRequired = lbj::PlayerActionRequired::Insurance;
            nextAction = lbj::DealerAction::None;
            return;
        
          } else if (player->always_insure) {
            playerStats.currentHand->insured = true;
            // TODO: allow insurance for less than one half of the original bet
            // if the guy (girl) wants to insure, we take his (her) money
            playerStats.bankroll -= 0.5 * playerStats.currentHand->bet;
            if (playerStats.bankroll < playerStats.worstBankroll) {
              playerStats.worstBankroll = playerStats.bankroll;
            }
            playerStats.handsInsured++;
          
            player->actionRequired = lbj::PlayerActionRequired::None;
            nextAction = lbj::DealerAction::CheckforBlackjacks;
            return;
          }
        }
      
        // step 7.b. if either the dealer or the player has a chance to have a blackjack, check
        if ((card[dealer_up_card].value == 10 || card[dealer_up_card].value == 11) || std::abs(player->value_player) == 21) {
          player->actionRequired = lbj::PlayerActionRequired::None;
          nextAction = lbj::DealerAction::CheckforBlackjacks;
          return;
        }
      } else {
        // in ENHC, if the player has 21.. 
        if (player->value_player == -21) {
          if (card[dealer_up_card].value == 10 || card[dealer_up_card].value == 11) {
            // and the dealer shows an ace or a face she has to draw
            // (actually she should ask for insurance)
            // but if the dealer does show an ace or a face the she has to hit
            dealer_hole_card = draw(&hand);
            info(lbj::Info::CardDealerRevealsHole, dealer_hole_card);
          }
          player->actionRequired = lbj::PlayerActionRequired::None;
          nextAction = lbj::DealerAction::CheckforBlackjacks;
          return;
        }
          
      }

      // step 7.c. ask the player to play
      can_double_split();
      player->actionRequired = lbj::PlayerActionRequired::Play;
      nextAction = lbj::DealerAction::AskForPlay;
      return;
    break;
 
    case lbj::DealerAction::CheckforBlackjacks:
      // step 8. check if there are any blackjack
      player_blackjack = playerStats.currentHand->blackjack();
      if (hand.blackjack()) {
        if (enhc == false) {
          info(lbj::Info::CardDealerRevealsHole, dealer_hole_card);
        }
        info(lbj::Info::DealerBlackjack);
#ifdef BJDEBUG
        std::cout << "dealer blackjack " << card[dealer_hole_card].utf8() << std::endl;
#endif
        playerStats.blackjacksDealer++;

        if (playerStats.currentHand->insured) {
          
          // pay him (her)
          playerStats.bankroll += (1.0 + 0.5) * playerStats.currentHand->bet;
          playerStats.currentOutcome += playerStats.currentHand->bet;
          info(lbj::Info::PlayerWinsInsurance, 1e3*playerStats.currentHand->bet);

          playerStats.winsInsured++;
        }

        if (player_blackjack) {
          info(lbj::Info::PlayerBlackjackAlso);
#ifdef BJDEBUG
          std::cout << "dealer_hole_card " << card[dealer_hole_card].utf8() << std::endl;
#endif

          // give him his (her her) money back
          playerStats.bankroll += playerStats.currentHand->bet;
          info(lbj::Info::PlayerPushes, 1e3*playerStats.currentHand->bet);
          
          playerStats.blackjacksPlayer++;
          playerStats.pushes++;
          
        } else {
          
          playerStats.currentOutcome -= playerStats.currentHand->bet;
          info(lbj::Info::PlayerLosses, 1e3*playerStats.currentHand->bet);
          
          playerStats.losses++;
        }

        nextAction = lbj::DealerAction::StartNewHand;
        player->actionRequired = lbj::PlayerActionRequired::None;
        return;
        
      } else if (player_blackjack) {

        // pay him (her)
        playerStats.bankroll += (1.0 + blackjack_pays) * playerStats.currentHand->bet;
        playerStats.currentOutcome += blackjack_pays * playerStats.currentHand->bet;
        info(lbj::Info::PlayerWins, 1e3 * blackjack_pays*playerStats.currentHand->bet);
        
        playerStats.blackjacksPlayer++;
        playerStats.wins++;
        playerStats.winsBlackjack++;

        nextAction = lbj::DealerAction::StartNewHand;
        player->actionRequired = lbj::PlayerActionRequired::None;
        return;
        
      } else {
        // only if the dealer had the chance to have a blackjack we say "No blackjacks"
        if (enhc == false && (card[dealer_up_card].value == 10 || card[dealer_up_card].value == 11)) {
          info(lbj::Info::NoBlackjacks);
        }
        
        can_double_split();
        nextAction = lbj::DealerAction::AskForPlay;
        player->actionRequired = lbj::PlayerActionRequired::Play;
        return;
      }        
    break;
    
    case lbj::DealerAction::AskForPlay:
#ifdef BJDEBUG
      std::cout << "please play" << std::endl;
#endif
      can_double_split();
      player->actionRequired = lbj::PlayerActionRequired::Play;
      nextAction = lbj::DealerAction::AskForPlay;
      return;
    break;
    
    case lbj::DealerAction::MoveOnToNextHand:
      // see if we finished all the player's hands
      if (++playerStats.currentHand != playerStats.hands.end()) {
        unsigned int playerCard = draw(&(*playerStats.currentHand));
        player->value_player = playerStats.currentHand->value();
        info(lbj::Info::CardPlayer, playerCard, playerStats.currentHand->id);
#ifdef BJDEBUG
        std::cout << "card player " << card[playerCard].utf8() << std::endl;
#endif

        if (std::abs(player->value_player) == 21) {
          player->actionRequired = lbj::PlayerActionRequired::None;
          nextAction = lbj::DealerAction::MoveOnToNextHand;
          return;
        } else {
          can_double_split();
          player->actionRequired = lbj::PlayerActionRequired::Play;
          nextAction = lbj::DealerAction::AskForPlay;
          return;
        }
      } else {
        // assume the player busted in all the hands
        bool bustedAllHands = true;
        for (auto playerHand = playerStats.hands.begin(); playerHand != playerStats.hands.end(); playerHand++) {
          // if he (she) did not bust, set to false
          if (playerHand->busted() == false) {
            bustedAllHands = false;
            break;
          }
        }

        if (bustedAllHands) {
          if (enhc == false) {  
            info(lbj::Info::CardDealerRevealsHole, dealer_hole_card);
#ifdef BJDEBUG
            std::cout << "hole " << card[dealer_hole_card].utf8() << std::endl;
#endif
          }
          
          player->actionRequired = lbj::PlayerActionRequired::None;
          nextAction = lbj::DealerAction::StartNewHand;
          return;
        }  else {
          player->actionRequired = lbj::PlayerActionRequired::None;
          nextAction = lbj::DealerAction::HitDealerHand;
          return;
        }
      }        
    break;
    
    case lbj::DealerAction::HitDealerHand:
        
      if (enhc == false) {
        info(lbj::Info::CardDealerRevealsHole, dealer_hole_card);
#ifdef BJDEBUG
        std::cout << "hole " << card[dealer_hole_card].utf8() << std::endl;
#endif
      }

      // hit while count is less than 17 (or equal to soft 17 if hit_soft_17 is true)
      player->value_dealer = hand.value();
      while ((std::abs(player->value_dealer) < 17 || (h17 && player->value_dealer == -17)) && hand.busted() == 0) {
        unsigned int dealerCard = draw(&hand);
        info(lbj::Info::CardDealer, dealerCard);
#ifdef BJDEBUG
        std::cout << "dealer " << card[dealerCard].utf8() << std::endl;
#endif
        player->value_dealer = hand.value();
      }
      
      if (enhc == true && hand.blackjack())  {
        info(lbj::Info::DealerBlackjack);
#ifdef BJDEBUG
        std::cout << "dealer blackjack " << card[dealer_hole_card].utf8() << std::endl;
#endif
        playerStats.blackjacksDealer++;
        
        // the player loses all the hands

        for (auto &player_hand : playerStats.hands) {
          if (player_hand.insured) {
            // pay him (her)
            playerStats.bankroll += (1.0 + 0.5) * player_hand.bet;
            playerStats.currentOutcome += player_hand.bet;
            info(lbj::Info::PlayerWinsInsurance, 1e3*playerStats.currentHand->bet);
            playerStats.winsInsured++;
          }

          playerStats.currentOutcome -= player_hand.bet;
          info(lbj::Info::PlayerLosses, 1e3*player_hand.bet);
          playerStats.losses++;
        }

        nextAction = lbj::DealerAction::StartNewHand;
        player->actionRequired = lbj::PlayerActionRequired::None;
        return;
      }
        
      if (hand.busted()) {
        info(lbj::Info::DealerBusts, player->value_dealer);
        playerStats.bustsDealer++;
        for (auto playerHand : playerStats.hands) {
          if (playerHand.busted() == false) {
            // pay him (her)
            playerStats.bankroll += 2 * playerHand.bet;
            playerStats.currentOutcome += playerHand.bet;
            info(lbj::Info::PlayerWins, 1e3*playerHand.bet);
            
            playerStats.wins++;
            playerStats.winsDoubled += playerHand.doubled;
          }
        }
      } else {
        for (auto playerHand : playerStats.hands) {
          if (playerHand.busted() == false) {  // busted hands have already been solved
            player->value_player = playerHand.value();
           
            if (std::abs(player->value_dealer) > std::abs(player->value_player)) {
                
              playerStats.currentOutcome -= playerHand.bet;
              info(lbj::Info::PlayerLosses, 1e3*playerHand.bet, player->value_player);
              playerStats.losses++;
                
            } else if (std::abs(player->value_dealer) == std::abs(player->value_player)) {
                  
              // give him his (her her) money back
              playerStats.bankroll += playerHand.bet;
              info(lbj::Info::PlayerPushes, 1e3*playerHand.bet);
              playerStats.pushes++;
                
            } else {
                
              // pay him (her)  
              playerStats.bankroll += 2 * playerHand.bet;
              playerStats.currentOutcome += playerHand.bet;
              info(lbj::Info::PlayerWins, 1e3*playerHand.bet, player->value_player);
              playerStats.wins++;
              playerStats.winsDoubled += playerHand.doubled;
              
            }
          }
        }
      }

      player->actionRequired = lbj::PlayerActionRequired::None;
      nextAction = lbj::DealerAction::StartNewHand;
      return;
    break;

    case lbj::DealerAction::None:
    break;

  }
          
}

// returns zero if it is a common command and we need to ask again
// returns positive if what was asked was answered
// returns negative if what was aked was not asnwered or the command does not apply
int Blackjack::process(void) {
  
  unsigned int playerCard;
  unsigned int firstCard;
  unsigned int secondCard;
    
  switch (player->actionTaken) {

  // we first check common commands
///ig+quit+name quit
///ig+quit+desc Finish the game
///ig+quit+detail Upon receiving this command, the game is finished
///ig+quit+detail immediately without even finishing the hand.
///ig+quit+detail All IPC resources are unlocked, removed and/or destroyed.
///ig+quit+detail The YAML report is written before exiting.
    case lbj::PlayerActionTaken::Quit:
      finished(true);
      return 1;
    break;    
        
///ig+help+name help
///ig+help+desc Ask for help
///ig+help+detail A succinct help message is written on the standard output.
///ig+help+detail This command makes sense only when issued by a human player.
    case lbj::PlayerActionTaken::Help:
      info(lbj::Info::Help);  
      return 0;
    break;  

///ig+rules+name rules
///ig+rules+desc Ask what the current rules are
    case lbj::PlayerActionTaken::Rules:
      info(lbj::Info::Rules);  
      return 0;
    break;  
    
///ig+bankroll+name bankroll
///ig+bankroll+desc Ask for the player’s current bankroll
    case lbj::PlayerActionTaken::Bankroll:
      info(lbj::Info::Bankroll, 1e3*playerStats.bankroll);  
      return 0;
    break;  
    
    case lbj::PlayerActionTaken::None:
      return 0;
    break;
      
      
    // if we made it this far, the command is particular
    case lbj::PlayerActionTaken::Bet:
      // TODO: bet = 0 -> wonging
      if (player->current_bet == 0) {
        info(lbj::Info::BetInvalid, player->current_bet);
        return 0;
      } else if (player->current_bet < 0) {
        info(lbj::Info::BetInvalid, player->current_bet);
        return 0;
      } else if (max_bet != 0  && player->current_bet > max_bet) {
        info(lbj::Info::BetInvalid, player->current_bet);
        return 0;
      } else {
          
        // ok, valid bet, copy the player's bet and use the local copy
        // (to prevent cheating players from changing the bet after dealing)
        playerStats.currentHand->bet = player->current_bet;
          
        // and take his (her) money
        playerStats.bankroll -= playerStats.currentHand->bet;
        if (playerStats.bankroll < playerStats.worstBankroll) {
          playerStats.worstBankroll = playerStats.bankroll;
        }
        playerStats.totalMoneyWaged += playerStats.currentHand->bet;
        
        nextAction = lbj::DealerAction::DealPlayerFirstCard;
        return 1;
        
      }
    break;

    case lbj::PlayerActionTaken::Insure:
      // TODO: allow insurance for less than one half of the original bet
      // take his (her) money
      playerStats.bankroll -= 0.5 * playerStats.currentHand->bet;
      if (playerStats.bankroll < playerStats.worstBankroll) {
        playerStats.worstBankroll = playerStats.bankroll;
      }
      playerStats.currentHand->insured = true;
      playerStats.handsInsured++;
         
      player->actionRequired = lbj::PlayerActionRequired::None;
      nextAction = lbj::DealerAction::CheckforBlackjacks;
      return 1;
    break;

    case lbj::PlayerActionTaken::DontInsure:
      playerStats.currentHand->insured = false;
      player->actionRequired = lbj::PlayerActionRequired::None;
      nextAction = lbj::DealerAction::CheckforBlackjacks;
      return 1;
    break;

///ip+stand+name stand
///ip+stand+desc Stand on the current hand
///ip+stand+detail When the player stands on a hand, the dealer moves on to
///ip+stand+detail the next one. If the player had split, a new card is
///ip+stand+detail dealt to the next split hand if there is one.
///ip+stand+detail Otherwise the dealer reveals his hole card and deals
///ip+stand+detail himself more cards if needed.
///ip+stand+detail This command can be abbreviated as `s`.
    case lbj::PlayerActionTaken::Stand:
      player->actionRequired = lbj::PlayerActionRequired::None;
      nextAction = lbj::DealerAction::MoveOnToNextHand;
      return 1;
    break;  
    
///ip+double+name double
///ip+double+desc Double down on the current hand
///ip+double+detail The player adds the same amount waged on the current hand
///ip+double+detail and in exchange she receives only one hand.
///ip+double+detail Doubling down is allowed only after receiving the first
///ip+double+detail two cards.
///ip+double+detail This command can be abbreviated as `d`.
    case lbj::PlayerActionTaken::Double:
      can_double_split();
      if (player->can_double == true) {

        // TODO: check bankroll
        // take his (her) money
        playerStats.bankroll -= playerStats.currentHand->bet;
        if (playerStats.bankroll < playerStats.worstBankroll) {
          playerStats.worstBankroll = playerStats.bankroll;
        }
        playerStats.totalMoneyWaged += playerStats.currentHand->bet;

        playerStats.currentHand->bet *= 2;
        playerStats.currentHand->doubled = true;
        playerStats.handsDoubled++;

        playerCard = draw(&(*playerStats.currentHand));
        player->value_player = playerStats.currentHand->value();
        info(lbj::Info::CardPlayer, playerCard, playerStats.currentHand->id);
        
        if (playerStats.currentHand->busted()) {
          info(lbj::Info::PlayerLosses, 1e3*playerStats.currentHand->bet, player->value_player);
          playerStats.currentOutcome -= playerStats.currentHand->bet;
          playerStats.bustsPlayer++;
          playerStats.losses++;
        }

        player->actionRequired = lbj::PlayerActionRequired::None;
        nextAction = lbj::DealerAction::MoveOnToNextHand;
        return 1;
        
      } else {
          
        info(lbj::Info::PlayerDoubleInvalid);
        return -1;
          
      }
    break;  

///ip+split+name split
///ip+split+desc Split the current hand. Adds an additional wage equal to the original one.
///ip+split+detail 
///ip+split+detail This command can be abbreviated as `p` (for pair).
    case lbj::PlayerActionTaken::Split:

      // TODO: front() and front()+1  
      firstCard  = *(playerStats.currentHand->cards.begin());
      secondCard = *(++playerStats.currentHand->cards.begin());
      
      // up to three splits (i.e. four hands)
      // TODO: check bankroll to see if player can split
      if (playerStats.splits < resplits && playerStats.currentHand->cards.size() == 2 && card[firstCard].value == card[secondCard].value) {
        
        // take player's money
        playerStats.bankroll -= playerStats.currentHand->bet;
        if (playerStats.bankroll < playerStats.worstBankroll) {
          playerStats.worstBankroll = playerStats.bankroll;
        }
        playerStats.totalMoneyWaged += playerStats.currentHand->bet;
          
        // tell the player the split is valid
        info(lbj::Info::PlayerSplitOk, playerStats.currentHand->id);
        
        // mark that we split to put ids in the hands and to limi the number of spltis
        playerStats.splits++;

        // the first hand is id=1, the rest have the id of the size of the list
        if (playerStats.currentHand == playerStats.hands.begin()) {
          playerStats.currentHand->id = 1;
        }
        
        // create a new hand
        PlayerHand newHand;
        newHand.id = playerStats.hands.size() + 1;
        newHand.bet = playerStats.currentHand->bet;
        
        // remove second the card from the first hand
        playerStats.currentHand->cards.pop_back();
        
        // and put it into the second hand
        newHand.cards.push_back(secondCard);

        // add the new hand to the list of hands        
        playerStats.hands.push_back(std::move(newHand));

        // tell the player what the ids are
        info(lbj::Info::PlayerSplitIds, playerStats.currentHand->id, newHand.id);
        
        // deal a card to the first hand
        playerCard = draw(&(*playerStats.currentHand));
        player->value_player = playerStats.currentHand->value();
        info(lbj::Info::CardPlayer, playerCard, playerStats.currentHand->id);

        // aces get dealt only one card
        // also, if the player gets 21 then we move on to the next hand
        if (card[*playerStats.currentHand->cards.begin()].value == 11 || std::abs(playerStats.currentHand->value()) == 21) {
          if (++playerStats.currentHand != playerStats.hands.end()) {
            info(lbj::Info::PlayerNextHand, (*playerStats.currentHand).id);
            playerCard = draw(&(*playerStats.currentHand));
            player->value_player = playerStats.currentHand->value();
            info(lbj::Info::CardPlayer, playerCard, playerStats.currentHand->id);

            // if the player got an ace or 21 again, we are done
            if (card[*playerStats.currentHand->cards.begin()].value == 11 || std::abs(playerStats.currentHand->value()) == 21) {
              player->actionRequired = lbj::PlayerActionRequired::None;
              nextAction = lbj::DealerAction::MoveOnToNextHand;
              return 1;
            } else {
              can_double_split();
              player->actionRequired = lbj::PlayerActionRequired::Play;
              nextAction = lbj::DealerAction::AskForPlay;
              return 1;
            }
          } else {
            player->actionRequired = lbj::PlayerActionRequired::None;
            nextAction = lbj::DealerAction::MoveOnToNextHand;
            return 1;
          }  
        } else {
          can_double_split();
          player->actionRequired = lbj::PlayerActionRequired::Play;
          nextAction = lbj::DealerAction::AskForPlay;
          return 1;
        }
      } else {

        info(lbj::Info::PlayerSplitInvalid);
        return -1;
          
      }
    break;
      
    case lbj::PlayerActionTaken::Hit:
///ip+hit+name hit
///ip+hit+desc Hit on the current hand
///ip+hit+detail 
///ip+hit+detail This command can be abbreviated as `h`.
      playerCard = draw(&(*playerStats.currentHand));        
      player->value_player = playerStats.currentHand->value();
      info(lbj::Info::CardPlayer, playerCard, playerStats.currentHand->id);

      if (playerStats.currentHand->busted()) {
          
        playerStats.currentOutcome -= playerStats.currentHand->bet;
        info(lbj::Info::PlayerLosses, 1e3*playerStats.currentHand->bet);
        playerStats.bustsPlayer++;
        playerStats.losses++;

        player->actionRequired = lbj::PlayerActionRequired::None;
        nextAction = lbj::DealerAction::MoveOnToNextHand;
        return 1;
        
      } else if (std::abs(playerStats.currentHand->value()) == 21) {
          
        player->actionRequired = lbj::PlayerActionRequired::None;
        nextAction = lbj::DealerAction::MoveOnToNextHand;
        return 1;
        
      } else {
          
        can_double_split();
        player->actionRequired = lbj::PlayerActionRequired::Play;
        nextAction = lbj::DealerAction::AskForPlay;
        return 1;
        
      }
    break;
    
    default:

      info(lbj::Info::CommandInvalid);
      return -1;
  
    break;
  }
  
  return 0;
}


void Blackjack::shuffle() {
    
  // for infinite decks there is no need to shuffle (how would one do it?)
  // we just pick a random card when we need to deal and that's it
  if (n_decks > 0) {
    std::shuffle(shoe.begin(), shoe.end(), rng);
    pos = 0;
    i_arranged_cards = 0;
    n_shuffles++;
  }
  
  return;
}


unsigned int Blackjack::draw(Hand *hand) {
    
  unsigned int tag = 0; 

  if (n_decks == 0) {
      
    if (n_arranged_cards == 0 || i_arranged_cards >= n_arranged_cards) {
      tag = fiftyTwoCards(rng);
    } else {
      // negative (or invalid) values are placeholder for random cards  
      if ((tag = arranged_cards[i_arranged_cards++]) <= 0 || tag > 52) {
        tag = fiftyTwoCards(rng);
      }
      
      if (quit_when_arranged_cards_run_out && i_arranged_cards == n_arranged_cards) {
        finished(true);
      }
    }  
    
  } else {
      
    if (n_arranged_cards == 0 || i_arranged_cards >= n_arranged_cards) {
      last_pass = (pos >= cut_card_position) || shuffle_every_hand;
      if (pos >= 52 * n_decks) {
        shuffle();
      }
    
    } else {
      if ((tag = arranged_cards[i_arranged_cards++]) > 0 && tag < 52) {

        // find the original position of the card tag
        auto it = std::find(shoe.begin() + pos, shoe.end(), tag);

        // Check if 'tag' was found and pos is valid
        if (it != shoe.end()) {
          // Get the index of the first occurrence of 'tag'
          size_t tag_index = std::distance(shoe.begin(), it);
    
          // Only swap if they're different positions
          if (pos != tag_index) {
            std::swap(shoe[pos], shoe[tag_index]);
          }
        } else {
          std::cerr << "error: no more cards " << tag << " in the shoe" << std::endl;
          exit(1);
        }
      }
    }
    tag = shoe[pos++];
    
  }
    
  if (hand != nullptr) {
    hand->cards.push_back(tag);
  }
  
  return tag;
}

std::string Blackjack::rules(void) {
  return ((enhc) ? "enhc" : "ahc")  + std::string(" ") +
         ((h17)  ? "h17"  : "s17")  + std::string(" ") +
         ((das)  ? "das"  : "ndas") + std::string(" ") +
         ((doa)  ? "doa"  : "do9")  + std::string(" ") +
//         ((rsa)  ? "rsa"  : "nrsa") + std::string(" ") +
         std::to_string(resplits) + "rsp " +
         std::to_string(n_decks) + "decks" ;
}
}
