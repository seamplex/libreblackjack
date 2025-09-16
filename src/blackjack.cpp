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
#include <algorithm>
#include <iterator>
#include <list>

#include "blackjack.h"

namespace lbj {
Blackjack::Blackjack(Configuration &conf) : rng(dev_random()), fiftyTwoCards(1, 52) {

  conf.set(&n_hands, {"n_hands", "hands"});
  conf.set(&n_decks, {"decks", "n_decks"});

  conf.set(&max_bet, {"max_bet", "maxbet"});
  
  
  // rules are base, particular options take precedence
  if (conf.exists("rules")) {
    std::istringstream iss(conf.getString("rules"));
    std::string token;
    while(iss >> token) {
      // I cannot believe there is no case-insensitive string comparison in C++
      if (token == "enhc" || token == "ENHC") {
        enhc = true;
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
      } else if (token == "ndoa" || token == "NDOA") {
        doa = false;
      } else if (token == "rsa" || token == "RSA") {
        rsa = true;
      } else if (token == "nrsa" || token == "NRSA") {
        rsa = false;
      } else {
        std::cerr << "error: unknown rule " << token << std::endl;
        exit(1);
      }
    }
    conf.markUsed("rules");
  }
  
  conf.set(&h17, {"h17", "hit_soft_17"});
  conf.set(&das, {"das", "double_after_split"});
  conf.set(&doa, {"doa", "double_on_any"});
  conf.set(&rsa, {"rsa", "resplit_aces"});
  conf.set(&enhc, {"enhc", "european_no_hole_card"});
  // TODO:
  // * rsa
  // * enhc
  conf.set(&blackjack_pays, {"blackjack_pays"});
  
  conf.set(&playerStats.bankroll, {"bankroll", "initial_bankroll"});
  
  // TODO: these should be go in the parent dealer class
  conf.set(&error_standard_deviations, {"error_standard_deviations"});
  conf.set(report_file_path, {"report_file_path", "report"});
  conf.set(&report_verbosity, {"report_verbosity", "report_level"});
  
  conf.set(&number_of_burnt_cards, {"number_of_burnt_cards", "n_burnt_cards", "burnt_cards"});
  conf.set(&penetration, {"penetration"});
  conf.set(&penetration_sigma, {"penetration_sigma", "penetration_dispersion"});
  conf.set(&shuffle_every_hand, {"shuffle", "shuffle_every_hand"});
  
  // TODO: read cards from file
  if (conf.exists("cards_as_ints")) {
    std::istringstream iss(conf.getString("cards_as_ints"));
    std::string token;
    while(iss >> token) {
      int n = std::stoi(token);
      if (n <= 0 || n > 52) {
        std::cerr << "error: invalid integer card " << token << std::endl;
        exit(1);
      }
      arranged_cards.push_back(n);
    }
    conf.markUsed("cards_as_ints");
  } else if (conf.exists("cards")) {
    std::istringstream iss(conf.getString("cards"));
    std::string token;
    while(iss >> token) {
      char number = token[0];
      char suit = token[1];
      int n = 0;
      if (number == 'A') {
        n = 1;    
      } else if (number == 'T') {
        n = 10;    
      } else if (number == 'J') {
        n = 10;    
      } else if (number == 'Q') {
        n = 12;    
      } else if (number == 'K') {
        n = 13;    
      } else {
        n = number - '0';
      }
      if (n < 1 || n > 13) {
        std::cerr << "error: invalid card " << token << std::endl;
        exit(1);
      }
      
      if (suit == 'C') {
        n += static_cast<int>(lbj::Suit::Clubs) * 13;
      } else if (suit == 'D') {
        n += static_cast<int>(lbj::Suit::Diamonds) * 13;
      } else if (suit == 'H') {
        n += static_cast<int>(lbj::Suit::Hearts) * 13;
      } else if (suit == 'S') {
        n += static_cast<int>(lbj::Suit::Spades) * 13;
      } else {
        std::cerr << "error: invalid card " << token << std::endl;
        exit(1);
      }
      
      arranged_cards.push_back(n);
    }
    conf.markUsed("cards");
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
    cut_card_position = static_cast<int>(penetration * 52 * n_decks);
  }
}

Blackjack::~Blackjack() {
  return;    
}

void Blackjack::can_double_split(void) {
  int n_cards = playerStats.currentHand->cards.size();
  player->canDouble = (n_cards == 2);
  if (das == false) {
    player->canDouble &= (playerStats.splits == 0);
  }
  if (doa == false) {
    int value = playerStats.currentHand->value();
    player->canDouble &= (value == 9 || value == 10 || value == 11);
  }
      
  player->canSplit = n_cards == 2 && (card[*(playerStats.currentHand->cards.begin())].value == card[*(++playerStats.currentHand->cards.begin())].value);
  return;
}  


void Blackjack::deal(void) {
  
  bool playerBlackjack = false;
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

      i_arranged_cards = 0;
      playerStats.currentOutcome = 0;
      n_hand++;
      
      // clear dealer's hand
      hand.cards.clear();

      // erase all the player's hands, create one, add and make it the current one
      for (auto playerHand = playerStats.hands.begin(); playerHand != playerStats.hands.end(); ++playerHand) {
        playerHand->cards.clear();
      }
      playerStats.hands.clear();
      playerStats.hands.push_back(std::move(PlayerHand()));
      playerStats.currentHand = playerStats.hands.begin();
      
      // state that the player did not win anything nor split nor doubled down
      playerStats.splits = 0;
      
      if (last_pass) {
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
      playerFirstCard = draw(&(*playerStats.currentHand));
      info(lbj::Info::CardPlayer, playerFirstCard);
#ifdef BJDEBUG
      std::cout << "first card " << card[playerFirstCard].utf8() << std::endl;
#endif
      // step 4. show dealer's upcard
      upCard = draw(&hand);
      info(lbj::Info::CardDealer, upCard);
#ifdef BJDEBUG
      std::cout << "up card " << card[upCard].utf8() << std::endl;
#endif
      player->dealerValue = hand.value();

      // step 5. deal the second card to each player
      playerSecondCard = draw(&(*playerStats.currentHand));
      info(lbj::Info::CardPlayer, playerSecondCard);
      player->playerValue = playerStats.currentHand->value();
#ifdef BJDEBUG
      std::cout << "second card " << card[playerSecondCard].utf8() << std::endl;
#endif
      
      if (enhc == false) {
        // step 6. deal the dealer's hole card 
        holeCard = draw(&hand);
        info(lbj::Info::CardDealer);

        // step 7.a. if the upcard is an ace ask for insurance
        if (card[upCard].value == 11) {
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
      }
      
      // step 7.b. if either the dealer or the player has a chance to have a blackjack, check
      if ((card[upCard].value == 10 || card[upCard].value == 11) || std::abs(player->playerValue) == 21) {
        player->actionRequired = lbj::PlayerActionRequired::None;
        nextAction = lbj::DealerAction::CheckforBlackjacks;
        return;
      }

      // step 7.c. ask the player to play
      can_double_split();
      player->actionRequired = lbj::PlayerActionRequired::Play;
      nextAction = lbj::DealerAction::AskForPlay;
      return;
    break;
 
    case lbj::DealerAction::CheckforBlackjacks:
      // step 8. check if there are any blackjack
      playerBlackjack = playerStats.currentHand->blackjack();
      if (hand.blackjack()) {
        info(lbj::Info::CardDealerRevealsHole, holeCard);
        info(lbj::Info::DealerBlackjack);
#ifdef BJDEBUG
        std::cout << "dealer blakjack " << card[holeCard].utf8() << std::endl;
#endif
        playerStats.blackjacksDealer++;

        if (playerStats.currentHand->insured) {
          
          // pay him (her)
          playerStats.bankroll += (1.0 + 0.5) * playerStats.currentHand->bet;
          playerStats.currentOutcome += playerStats.currentHand->bet;
          info(lbj::Info::PlayerWinsInsurance, 1e3*playerStats.currentHand->bet);

          playerStats.winsInsured++;
        }

        if (playerBlackjack) {
          info(lbj::Info::PlayerBlackjackAlso);
#ifdef BJDEBUG
          std::cout << "both blackjack " << card[holeCard].utf8() << std::endl;
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
        
      } else if (playerBlackjack) {

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
        if (enhc == false && (card[upCard].value == 10 || card[upCard].value == 11)) {
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
      std::cout << "pistola" << std::endl;
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
        player->playerValue = playerStats.currentHand->value();
        info(lbj::Info::CardPlayer, playerCard, playerStats.currentHand->id);
#ifdef BJDEBUG
        std::cout << "card player " << card[playerCard].utf8() << std::endl;
#endif

        if (std::abs(player->playerValue) == 21) {
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
            info(lbj::Info::CardDealerRevealsHole, holeCard);
#ifdef BJDEBUG
            std::cout << "hole " << card[holeCard].utf8() << std::endl;
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
        info(lbj::Info::CardDealerRevealsHole, holeCard);
#ifdef BJDEBUG
        std::cout << "hole " << card[holeCard].utf8() << std::endl;
#endif
      }

      // hit while count is less than 17 (or equal to soft 17 if hit_soft_17 is true)
      player->dealerValue = hand.value();
      while ((std::abs(player->dealerValue) < 17 || (h17 && player->dealerValue == -17)) && hand.busted() == 0) {
        unsigned int dealerCard = draw(&hand);
        info(lbj::Info::CardDealer, dealerCard);
#ifdef BJDEBUG
        std::cout << "dealer " << card[dealerCard].utf8() << std::endl;
#endif
        player->dealerValue = hand.value();
      }
        
      if (hand.busted()) {
        info(lbj::Info::DealerBusts, player->dealerValue);
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
            player->playerValue = playerHand.value();
           
            if (std::abs(player->dealerValue) > std::abs(player->playerValue)) {
                
              playerStats.currentOutcome -= playerHand.bet;
              info(lbj::Info::PlayerLosses, 1e3*playerHand.bet, player->playerValue);
              playerStats.losses++;
                
            } else if (std::abs(player->dealerValue) == std::abs(player->playerValue)) {
                  
              // give him his (her her) money back
              playerStats.bankroll += playerHand.bet;
              info(lbj::Info::PlayerPushes, 1e3*playerHand.bet);
              playerStats.pushes++;
                
            } else {
                
              // pay him (her)  
              playerStats.bankroll += 2 * playerHand.bet;
              playerStats.currentOutcome += playerHand.bet;
              info(lbj::Info::PlayerWins, 1e3*playerHand.bet, player->playerValue);
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

///ig+bankroll+name help
///ig+bankroll+desc Ask for help
///ig+bankroll+detail Ask for the bankroll.
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
      if (player->currentBet == 0) {
        info(lbj::Info::BetInvalid, player->currentBet);
        return 0;
      } else if (player->currentBet < 0) {
        info(lbj::Info::BetInvalid, player->currentBet);
        return 0;
      } else if (max_bet != 0  && player->currentBet > max_bet) {
        info(lbj::Info::BetInvalid, player->currentBet);
        return 0;
      } else {
          
        // ok, valid bet, copy the player's bet and use the local copy
        // (to prevent cheating players from changing the bet after dealing)
        playerStats.currentHand->bet = player->currentBet;
          
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
      if (player->canDouble == true) {

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
        player->playerValue = playerStats.currentHand->value();
        info(lbj::Info::CardPlayer, playerCard, playerStats.currentHand->id);
        
        if (playerStats.currentHand->busted()) {
          info(lbj::Info::PlayerLosses, 1e3*playerStats.currentHand->bet, player->playerValue);
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
      // TODO: choose through conf how many max splits are available
      // TODO: check bankroll to see if player can split
//      if (playerStats.splits < 3 && playerStats.currentHand->cards.size() == 2 && card[firstCard].value == card[secondCard].value) {
      if (playerStats.currentHand->cards.size() == 2 && card[firstCard].value == card[secondCard].value) {
        
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
        player->playerValue = playerStats.currentHand->value();
        info(lbj::Info::CardPlayer, playerCard, playerStats.currentHand->id);

        // aces get dealt only one card
        // also, if the player gets 21 then we move on to the next hand
        if (card[*playerStats.currentHand->cards.begin()].value == 11 || std::abs(playerStats.currentHand->value()) == 21) {
          if (++playerStats.currentHand != playerStats.hands.end()) {
            info(lbj::Info::PlayerNextHand, (*playerStats.currentHand).id);
            playerCard = draw(&(*playerStats.currentHand));
            player->playerValue = playerStats.currentHand->value();
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
      player->playerValue = playerStats.currentHand->value();
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
    n_shuffles++;
  }
  
  return;
}


unsigned int Blackjack::draw(Hand *hand) {
    
  unsigned int tag = 0; 

  if (n_decks == 0) {
      
    if (n_arranged_cards != 0 && i_arranged_cards < n_arranged_cards) {
      // negative (or invalid) values are placeholder for random cards  
      if ((tag = arranged_cards[i_arranged_cards++]) <= 0 || tag > 52) {
        tag = fiftyTwoCards(rng);
      }
    } else {
      tag = fiftyTwoCards(rng);
    }  
    
  } else {
      
    last_pass = (pos >= cut_card_position) || shuffle_every_hand;
    if (pos >= 52 * n_decks) {
      shuffle();
    }
    
    tag = shoe[pos++];
  }
    
  if (hand != nullptr) {
    hand->cards.push_back(tag);
  }
  
  return tag;
}
}