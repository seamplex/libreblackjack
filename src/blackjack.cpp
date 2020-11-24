/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - standard blackjack dealer
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

Blackjack::Blackjack(Configuration &conf) : rng(dev_random()), fiftyTwoCards(1, 52) {

  conf.set(&n_hands, {"n_hands", "hands"});  
  conf.set(&n_decks, {"decks", "n_decks"});

  conf.set(&max_bet, {"max_bet", "maxbet"});
  conf.set(&hit_soft_17, {"h17", "hit_soft_17"});
  conf.set(&double_after_split, {"das", "double_after_split"});
  conf.set(&blackjack_pays, {"blackjack_pays"});
  
  conf.set(&playerStats.bankroll, {"bankroll"});
  
  conf.set(&number_of_burnt_cards, {"number_of_burnt_cards", "n_burnt_cards", "burnt_cards"});
  conf.set(&penetration, {"penetration"});
  conf.set(&penetration_sigma, {"penetration_sigma", "penetration_dispersion"});
  conf.set(&shuffle_every_hand, {"shuffle_every_hand"});
  
  if (conf.exists("arranged_cards")) {
    std::istringstream stream(conf.getString("arranged_cards"));
    std::string token;
    while(std::getline(stream, token, ',')) {
      arranged_cards.push_back(std::stoi(token));
    }
  }
  
  n_arranged_cards = arranged_cards.size();
  
  bool explicit_seed = conf.set(&rng_seed, {"rng_seed", "seed"});
  
  if (explicit_seed) {
    rng = std::mt19937(rng_seed);
  }
}

Blackjack::~Blackjack() {
  return;    
}

void Blackjack::deal(void) {
  
  bool playerBlackjack = false;
  // let's start by assuming the player does not need to do anything
  player->actionRequired = Libreblackjack::PlayerActionRequired::None;

//  std::list<PlayerHand>::iterator playerHand;
  
  switch(nextAction) {
    // -------------------------------------------------------------------------  
    case Libreblackjack::DealerAction::StartNewHand:
        
      // check if we are done
      if (n_hands > 0 && n_hand >= n_hands) {
        finished(true);
        return;
      }
      
      // update the uncertainty (knuth citing welford)
      // The Art of Computer Programming, volume 2: Seminumerical Algorithms, 3rd edn., p. 232
      // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford's_online_algorithm

      if (n_hand != 0) {
        double delta = playerStats.result - playerStats.mean;
        playerStats.mean += delta / (double)(n_hand);
        playerStats.M2 += delta * (playerStats.result - playerStats.mean);
        playerStats.variance = playerStats.M2 / (double)(n_hand);
      }

      i_arranged_cards = 0;
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
      
      if (lastPass) {
        info(Libreblackjack::Info::Shuffle);
          
        // shuffle the cards          
        shuffle();        
          
        // burn as many cards as asked
        for (unsigned int i = 0; i < number_of_burnt_cards; i++) {
          drawCard();
        }
        lastPass = false;
      }

      info(Libreblackjack::Info::NewHand, n_hand, 1e3*playerStats.bankroll);
      
      if (player->flat_bet) {
          
        // TODO: check bankroll
        playerStats.currentHand->bet = player->flat_bet;
        // take player's money
        playerStats.bankroll -= playerStats.currentHand->bet;
        if (playerStats.bankroll < playerStats.worstBankroll) {
          playerStats.worstBankroll = playerStats.bankroll;
        }
        playerStats.totalMoneyWaged += playerStats.currentHand->bet;
        
        player->actionRequired = Libreblackjack::PlayerActionRequired::None;
        nextAction = Libreblackjack::DealerAction::DealPlayerFirstCard;
        
      } else {
          
        player->actionRequired = Libreblackjack::PlayerActionRequired::Bet;
        nextAction = Libreblackjack::DealerAction::None;
        
      }

      return;
      
    break;
    
    // -------------------------------------------------------------------------  
    case Libreblackjack::DealerAction::DealPlayerFirstCard:
      // where's step 2? <- probably that's the player's bet
      // step 3. deal the first card to each player
      playerFirstCard = drawCard(&(*playerStats.currentHand));
      info(Libreblackjack::Info::CardPlayer, playerFirstCard);
            
      // step 4. show dealer's upcard
      upCard = drawCard(&hand);
      info(Libreblackjack::Info::CardDealer, upCard);
      player->dealerValue = hand.value();

      // step 5. deal the second card to each player
      playerSecondCard = drawCard(&(*playerStats.currentHand));
      info(Libreblackjack::Info::CardPlayer, playerSecondCard);
      
      // step 6. deal the dealer's hole card 
      holeCard = drawCard(&hand);
      info(Libreblackjack::Info::CardDealer);

      // step 7.a. if the upcard is an ace ask for insurance
      if (card[upCard].value == 11) {
        if (player->no_insurance == false && player->always_insure == false) {
          player->actionRequired = Libreblackjack::PlayerActionRequired::Insurance;
          nextAction = Libreblackjack::DealerAction::None;
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
          
          player->actionRequired = Libreblackjack::PlayerActionRequired::None;
          nextAction = Libreblackjack::DealerAction::CheckforBlackjacks;
          return;
        }
      }
      
      // step 7.b. if either the dealer or the player has a chance to have a blackjack, check
      player->playerValue = playerStats.currentHand->value();
      if ((card[upCard].value == 10 || card[upCard].value == 11) || std::abs(player->playerValue) == 21) {
        player->actionRequired = Libreblackjack::PlayerActionRequired::None;
        nextAction = Libreblackjack::DealerAction::CheckforBlackjacks;
        return;
      }

      // step 7.c. ask the player to play
      player->actionRequired = Libreblackjack::PlayerActionRequired::Play;
      nextAction = Libreblackjack::DealerAction::AskForPlay;
      return;
    break;
 
    case Libreblackjack::DealerAction::CheckforBlackjacks:
      // step 8. check if there are any blackjack
      playerBlackjack = playerStats.currentHand->blackjack();
      if (hand.blackjack()) {
        info(Libreblackjack::Info::CardDealerRevealsHole, holeCard);
        info(Libreblackjack::Info::DealerBlackjack);
        playerStats.blackjacksDealer++;

        if (playerStats.currentHand->insured) {
          
          // pay him (her)
          playerStats.bankroll += (1.0 + 0.5) * playerStats.currentHand->bet;
          playerStats.result += playerStats.currentHand->bet;
          info(Libreblackjack::Info::PlayerWinsInsurance, 1e3*playerStats.currentHand->bet);

          playerStats.winsInsured++;
        }

        if (playerBlackjack) {
          info(Libreblackjack::Info::PlayerBlackjackAlso);

          // give him his (her her) money back
          playerStats.bankroll += playerStats.currentHand->bet;
          info(Libreblackjack::Info::PlayerPushes, 1e3*playerStats.currentHand->bet);
          
          playerStats.blackjacksPlayer++;
          playerStats.pushes++;
          
        } else {
          
          playerStats.result -= playerStats.currentHand->bet;
          info(Libreblackjack::Info::PlayerLosses, 1e3*playerStats.currentHand->bet);
          
          playerStats.losses++;
        }

        nextAction = Libreblackjack::DealerAction::StartNewHand;
        player->actionRequired = Libreblackjack::PlayerActionRequired::None;
        return;
        
      } else if (playerBlackjack) {

        // pay him (her)
        playerStats.bankroll += (1.0 + blackjack_pays) * playerStats.currentHand->bet;
        playerStats.result += blackjack_pays * playerStats.currentHand->bet;
        info(Libreblackjack::Info::PlayerWins, 1e3 * blackjack_pays*playerStats.currentHand->bet);
        
        playerStats.blackjacksPlayer++;
        playerStats.wins++;
        playerStats.winsBlackjack++;

        nextAction = Libreblackjack::DealerAction::StartNewHand;
        player->actionRequired = Libreblackjack::PlayerActionRequired::None;
        return;
        
      } else {
        // only if the dealer had the chance to have a blackjack we say "No blackjacks"
        if (card[upCard].value == 10 || card[upCard].value == 11) {
          info(Libreblackjack::Info::NoBlackjacks);
        }
        
        nextAction = Libreblackjack::DealerAction::AskForPlay;
        player->actionRequired = Libreblackjack::PlayerActionRequired::Play;
        return;
      }        
    break;
    
    case Libreblackjack::DealerAction::AskForPlay:
      player->actionRequired = Libreblackjack::PlayerActionRequired::Play;
      nextAction = Libreblackjack::DealerAction::AskForPlay;
      return;
    break;
    
    case Libreblackjack::DealerAction::MoveOnToNextHand:
      // see if we finished all the player's hands
      if (++playerStats.currentHand != playerStats.hands.end()) {
        unsigned int playerCard = drawCard(&(*playerStats.currentHand));
        player->playerValue = playerStats.currentHand->value();
        info(Libreblackjack::Info::CardPlayer, playerCard, playerStats.currentHand->id);

        if (std::abs(player->playerValue) == 21) {
          player->actionRequired = Libreblackjack::PlayerActionRequired::None;
          nextAction = Libreblackjack::DealerAction::MoveOnToNextHand;
          return;
        } else {
          player->actionRequired = Libreblackjack::PlayerActionRequired::Play;
          nextAction = Libreblackjack::DealerAction::AskForPlay;
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
          info(Libreblackjack::Info::CardDealerRevealsHole, holeCard);
          
          player->actionRequired = Libreblackjack::PlayerActionRequired::None;
          nextAction = Libreblackjack::DealerAction::StartNewHand;
          return;
        }  else {
          player->actionRequired = Libreblackjack::PlayerActionRequired::None;
          nextAction = Libreblackjack::DealerAction::HitDealerHand;
          return;
        }
      }        
    break;
    
    case Libreblackjack::DealerAction::HitDealerHand:
        
      info(Libreblackjack::Info::CardDealerRevealsHole, holeCard);

      // hit while count is less than 17 (or equal to soft 17 if hit_soft_17 is true)
      player->dealerValue = hand.value();
      while ((std::abs(player->dealerValue) < 17 || (hit_soft_17 && player->dealerValue == -17)) && hand.busted() == 0) {
        unsigned int dealerCard = drawCard(&hand);
        info(Libreblackjack::Info::CardDealer, dealerCard);
        player->dealerValue = hand.value();
      }
        
      if (hand.busted()) {
        info(Libreblackjack::Info::DealerBusts, player->dealerValue);
        playerStats.bustsDealer++;
        for (auto playerHand : playerStats.hands) {
          if (playerHand.busted() == false) {
            // pay him (her)
            playerStats.bankroll += 2 * playerHand.bet;
            playerStats.result += playerHand.bet;
            info(Libreblackjack::Info::PlayerWins, 1e3*playerHand.bet);
            
            playerStats.wins++;
            playerStats.winsDoubled += playerHand.doubled;
          }
        }
      } else {
        for (auto playerHand : playerStats.hands) {
          if (playerHand.busted() == false) {  // busted hands have already been solved
            player->playerValue = playerHand.value();
           
            if (std::abs(player->dealerValue) > std::abs(player->playerValue)) {
                
              playerStats.result -= playerHand.bet;
              info(Libreblackjack::Info::PlayerLosses, 1e3*playerHand.bet, player->playerValue);
              playerStats.losses++;
                
            } else if (std::abs(player->dealerValue) == std::abs(player->playerValue)) {
                  
              // give him his (her her) money back
              playerStats.bankroll += playerHand.bet;
              info(Libreblackjack::Info::PlayerPushes, 1e3*playerHand.bet);
              playerStats.pushes++;
                
            } else {
                
              // pay him (her)  
              playerStats.bankroll += 2 * playerHand.bet;
              playerStats.result += playerHand.bet;
              info(Libreblackjack::Info::PlayerWins, 1e3*playerHand.bet, player->playerValue);
              playerStats.wins++;
              playerStats.winsDoubled += playerHand.doubled;
              
            }
          }
        }
      }

      player->actionRequired = Libreblackjack::PlayerActionRequired::None;
      nextAction = Libreblackjack::DealerAction::StartNewHand;
      return;
    break;

    case Libreblackjack::DealerAction::None:
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
    case Libreblackjack::PlayerActionTaken::Quit:
      finished(true);
      return 1;
    break;    
        
///ig+help+name help
///ig+help+desc Ask for help
///ig+help+detail A succinct help message is written on the standard output.
///ig+help+detail This command makes sense only when issued by a human player.
    case Libreblackjack::PlayerActionTaken::Help:
      info(Libreblackjack::Info::Help);  
      return 0;
    break;  

///ig+bankroll+name help
///ig+bankroll+desc Ask for help
///ig+bankroll+detail Ask for the bankroll.
    case Libreblackjack::PlayerActionTaken::Bankroll:
      info(Libreblackjack::Info::Bankroll, 1e3*playerStats.bankroll);  
      return 0;
    break;  
    
    case Libreblackjack::PlayerActionTaken::None:
      return 0;
    break;
      
      
    // if we made it this far, the command is particular
    case Libreblackjack::PlayerActionTaken::Bet:
      // TODO: bet = 0 -> wonging
      if (player->currentBet == 0) {
        info(Libreblackjack::Info::BetInvalid, player->currentBet);
        return 0;
      } else if (player->currentBet < 0) {
        info(Libreblackjack::Info::BetInvalid, player->currentBet);
        return 0;
      } else if (max_bet != 0  && player->currentBet > max_bet) {
        info(Libreblackjack::Info::BetInvalid, player->currentBet);
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
        
        nextAction = Libreblackjack::DealerAction::DealPlayerFirstCard;
        return 1;
        
      }
    break;

    case Libreblackjack::PlayerActionTaken::Insure:
      // TODO: allow insurance for less than one half of the original bet
      // take his (her) money
      playerStats.bankroll -= 0.5 * playerStats.currentHand->bet;
      if (playerStats.bankroll < playerStats.worstBankroll) {
        playerStats.worstBankroll = playerStats.bankroll;
      }
      playerStats.currentHand->insured = true;
      playerStats.handsInsured++;
         
      player->actionRequired = Libreblackjack::PlayerActionRequired::None;
      nextAction = Libreblackjack::DealerAction::CheckforBlackjacks;
      return 1;
    break;

    case Libreblackjack::PlayerActionTaken::DontInsure:
      playerStats.currentHand->insured = false;
      player->actionRequired = Libreblackjack::PlayerActionRequired::None;
      nextAction = Libreblackjack::DealerAction::CheckforBlackjacks;
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
    case Libreblackjack::PlayerActionTaken::Stand:
      player->actionRequired = Libreblackjack::PlayerActionRequired::None;
      nextAction = Libreblackjack::DealerAction::MoveOnToNextHand;
      return 1;
    break;  
    
///ip+double+name double
///ip+double+desc Double down on the current hand
///ip+double+detail The player adds the same amount waged on the current hand
///ip+double+detail and in exchange she receives only one hand.
///ip+double+detail Doubling down is allowed only after receiving the first
///ip+double+detail two cards.
///ip+double+detail This command can be abbreviated as `d`.
    case Libreblackjack::PlayerActionTaken::Double:
      // TODO: rule to allow doubling only for 9, 10 or 11
      if (playerStats.currentHand->cards.size() == 2) {

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

        playerCard = drawCard(&(*playerStats.currentHand));
        player->playerValue = playerStats.currentHand->value();
        info(Libreblackjack::Info::CardPlayer, playerCard, playerStats.currentHand->id);
        
        if (playerStats.currentHand->busted()) {
          info(Libreblackjack::Info::PlayerLosses, 1e3*playerStats.currentHand->bet, player->playerValue);
          playerStats.result -= playerStats.currentHand->bet;
          playerStats.bustsPlayer++;
          playerStats.losses++;
        }

        player->actionRequired = Libreblackjack::PlayerActionRequired::None;
        nextAction = Libreblackjack::DealerAction::MoveOnToNextHand;
        return 1;
        
      } else {
          
        info(Libreblackjack::Info::PlayerDoubleInvalid);
        return -1;
          
      }
    break;  

///ip+split+name split
///ip+split+desc Split the current hand. Adds an additional wage equal to the original one.
///ip+split+detail 
///ip+split+detail This command can be abbreviated as `p` (for pair).
    case Libreblackjack::PlayerActionTaken::Split:

      // TODO: front() and front()+1  
      firstCard  = *(playerStats.currentHand->cards.begin());
      secondCard = *(++playerStats.currentHand->cards.begin());
      
      // up to three splits (i.e. four hands)
      // TODO: choose through conf
      // TODO: check bankroll to see if player can split
      if (playerStats.splits < 3 && playerStats.currentHand->cards.size() == 2 && card[firstCard].value == card[secondCard].value) {
        
        // take player's money
        playerStats.bankroll -= playerStats.currentHand->bet;
        if (playerStats.bankroll < playerStats.worstBankroll) {
          playerStats.worstBankroll = playerStats.bankroll;
        }
        playerStats.totalMoneyWaged += playerStats.currentHand->bet;
          
        // tell the player the split is valid
        info(Libreblackjack::Info::PlayerSplitOk, playerStats.currentHand->id);
        
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
        info(Libreblackjack::Info::PlayerSplitIds, playerStats.currentHand->id, newHand.id);
        
        // deal a card to the first hand
        playerCard = drawCard(&(*playerStats.currentHand));
        player->playerValue = playerStats.currentHand->value();
        info(Libreblackjack::Info::CardPlayer, playerCard, playerStats.currentHand->id);

        // aces get dealt only one card
        // also, if the player gets 21 then we move on to the next hand
        if (card[*playerStats.currentHand->cards.begin()].value == 11 || std::abs(playerStats.currentHand->value()) == 21) {
          if (++playerStats.currentHand != playerStats.hands.end()) {
            info(Libreblackjack::Info::PlayerNextHand, (*playerStats.currentHand).id);
            playerCard = drawCard(&(*playerStats.currentHand));
            player->playerValue = playerStats.currentHand->value();
            info(Libreblackjack::Info::CardPlayer, playerCard, playerStats.currentHand->id);

            // if the player got an ace or 21 again, we are done
            if (card[*playerStats.currentHand->cards.begin()].value == 11 || std::abs(playerStats.currentHand->value()) == 21) {
              player->actionRequired = Libreblackjack::PlayerActionRequired::None;
              nextAction = Libreblackjack::DealerAction::MoveOnToNextHand;
              return 1;
            } else {
              player->actionRequired = Libreblackjack::PlayerActionRequired::Play;
              nextAction = Libreblackjack::DealerAction::AskForPlay;
              return 1;
            }
          } else {
            player->actionRequired = Libreblackjack::PlayerActionRequired::None;
            nextAction = Libreblackjack::DealerAction::MoveOnToNextHand;
            return 1;
          }  
        } else {
          player->actionRequired = Libreblackjack::PlayerActionRequired::Play;
          nextAction = Libreblackjack::DealerAction::AskForPlay;
          return 1;
        }
      } else {

        info(Libreblackjack::Info::PlayerSplitInvalid);
        return -1;
          
      }
    break;
      
    case Libreblackjack::PlayerActionTaken::Hit:
///ip+hit+name hit
///ip+hit+desc Hit on the current hand
///ip+hit+detail 
///ip+hit+detail This command can be abbreviated as `h`.
      playerCard = drawCard(&(*playerStats.currentHand));        
      player->playerValue = playerStats.currentHand->value();
      info(Libreblackjack::Info::CardPlayer, playerCard, playerStats.currentHand->id);

      if (playerStats.currentHand->busted()) {
          
        playerStats.result -= playerStats.currentHand->bet;
        info(Libreblackjack::Info::PlayerLosses, 1e3*playerStats.currentHand->bet);
        playerStats.bustsPlayer++;
        playerStats.losses++;

        player->actionRequired = Libreblackjack::PlayerActionRequired::None;
        nextAction = Libreblackjack::DealerAction::MoveOnToNextHand;
        return 1;
        
      } else if (std::abs(playerStats.currentHand->value()) == 21) {
          
        player->actionRequired = Libreblackjack::PlayerActionRequired::None;
        nextAction = Libreblackjack::DealerAction::MoveOnToNextHand;
        return 1;
        
      } else {
          
        player->actionRequired = Libreblackjack::PlayerActionRequired::Play;
        nextAction = Libreblackjack::DealerAction::AskForPlay;
        return 1;
        
      }
    break;
    
    default:

      info(Libreblackjack::Info::CommandInvalid);
      return -1;
  
    break;
  }
  
  return 0;
}


void Blackjack::shuffle() {
    
  // for infinite decks there is no need to shuffle (how would one do it?)
  // we just pick a random card when we need to deal and that's it
  if (n_decks == -1) {
    return;
  }
  
  // TODO: shuffle shoe
  
  return;
    
    
}


unsigned int Blackjack::drawCard(Hand *hand) {
    
  unsigned int tag = 0; 

  if (n_decks == -1) {
    if (n_arranged_cards != 0 && i_arranged_cards < n_arranged_cards) {
      // negative (or invalid) values are placeholder for random cards  
      if ((tag = arranged_cards[i_arranged_cards++]) <= 0 || tag > 52) {
        tag = fiftyTwoCards(rng);
      }
    } else {
      tag = fiftyTwoCards(rng);
    }  
    
  } else {
    // TODO: shoes
    tag = 0;
  }
    
  if (hand != nullptr) {
    hand->cards.push_back(tag);
  }
  
  return tag;
}
