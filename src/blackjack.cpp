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

void Blackjack::deal(Player *player) {
  
  int playerTotal = 0;
  int dealerTotal = 0;
  bool playerBlackack = false;
  // let's start by assuming the player does not need to do anything
  player->actionRequired = PlayerActionRequired::None;
    
  switch(nextAction) {
    // -------------------------------------------------------------------------  
    case DealerAction::StartNewHand:
        
      // check if we are done
      if (n_hands > 0 && n_hand >= n_hands) {
        finished(true);
        return;
      }
      
      // update the uncertainty (knuth citing welford)
      // The Art of Computer Programming, volume 2: Seminumerical Algorithms, 3rd edn., p. 232
      // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford's_online_algorithm

      if (n_hand != 0) {
        double delta = player->current_result - player->mean;
        player->mean += delta / (double)(n_hand);
        player->M2 += delta * (player->current_result - player->mean);
        player->variance = player->M2 / (double)(n_hand);
      }

      // reset this index
      i_arranged_cards = 0;
      n_hand++;
      
      // clear dealer's hand
      hand.cards.clear();

      // erase all the player's hands, create one, add and make it the current one
      for (auto playerHand : player->hands) {
        playerHand.cards.clear();
      }
      player->hands.clear();
      player->hands.push_back(std::move(PlayerHand()));
      player->currentHand = player->hands.begin();
      
      // state that the player did not win anything nor splitted nor doubled down
      player->current_result = 0;
      player->currentSplits = 0;
      
      if (lastPass) {
        info(player, Info::Shuffle);
          
        // shuffle the cards          
        shuffle();        
          
        // burn as many cards as asked
        for (unsigned int i = 0; i < number_of_burnt_cards; i++) {
          drawCard();
        }
        lastPass = false;
      }
      
      if (player->flat_bet) {
        player->currentHand->bet = player->flat_bet;
        player->actionRequired = PlayerActionRequired::None;
        nextAction = DealerAction::DealPlayerFirstCard;
      } else {
        player->actionRequired = PlayerActionRequired::Bet;
        nextAction = DealerAction::AskForBets;

      }

      info(player, Info::NewHand, n_hand, 1e3*player->bankroll);
      return;
      
    break;
    
    case DealerAction::AskForBets:
    break;
    
    // -------------------------------------------------------------------------  
    case DealerAction::DealPlayerFirstCard:
      // where's step 2? <- probably that's the player's bet
      // step 3. deal the first card to each player
      player->n_hands++;        // splits are counted as a single hand
      player->total_money_waged += player->currentHand->bet;

      playerFirstCard = drawCard(&(*player->currentHand));
      info(player, Info::CardPlayer, playerFirstCard);
            
      // step 4. show dealer's upcard
      upCard = drawCard(&hand);
      info(player, Info::CardDealer, upCard);

      // step 5. deal the second card to each player
      playerSecondCard = drawCard(&(*player->currentHand));
      info(player, Info::CardPlayer, playerSecondCard);
      
      // step 6. deal the dealer's hole card 
      holeCard = drawCard(&hand);
      info(player, Info::CardDealer);

      // step 7.a. if the upcard is an ace ask for insurance
      if (card[upCard].value == 11) {
        if (player->no_insurance == false && player->always_insure == false) {
          player->actionRequired = PlayerActionRequired::Insurance;
          nextAction = DealerAction::AskForInsurance;
          return;
        
        } else if (player->always_insure) {
          player->currentHand->insured = true;
          // TODO: allow insurance for less than one half of the original bet
          player->current_result -= 0.5 * player->currentHand->bet;
          player->bankroll -= 0.5 * player->currentHand->bet;
          player->handsInsured++;
          
          player->actionRequired = PlayerActionRequired::None;
          nextAction = DealerAction::CheckforBlackjacks;
          return;
        }
      }
      
      // step 7.b. if either the dealer or the player has a chance to have a blackjack, check
      playerTotal = player->currentHand->total();
      if ((card[upCard].value == 10 || card[upCard].value == 11) || std::abs(playerTotal) == 21) {
        player->actionRequired = PlayerActionRequired::None;
        nextAction = DealerAction::CheckforBlackjacks;
        return;
      }

      // step 7.c. ask the player to play
      player->actionRequired = PlayerActionRequired::Play;
      nextAction = DealerAction::AskForPlay;
      return;
    break;
 
    case DealerAction::AskForInsurance:
      return;
    break;

    case DealerAction::CheckforBlackjacks:
      // step 8. check if there are any blackjack
      playerBlackack = player->currentHand->blackjack();
      if (hand.blackjack()) {
        info(player, Info::CardDealerRevealsHole, holeCard);
        info(player, Info::DealerBlackjack);
        player->blackjacksDealer++;

        if (player->currentHand->insured) {
          info(player, Info::PlayerWinsInsurance, 1e3*player->currentHand->bet);
          player->current_result += player->currentHand->bet;
          player->bankroll += player->currentHand->bet;
          player->winsInsured++;
        }

        if (playerBlackack) {
          info(player, Info::PlayerBlackjackAlso);
          info(player, Info::PlayerPushes, 1e3*player->currentHand->bet);
          player->blackjacksPlayer++;
          player->pushes++;
          
        } else {
          info(player, Info::PlayerLosses, 1e3*player->currentHand->bet);
          player->current_result -= player->currentHand->bet;
          player->bankroll -= player->currentHand->bet;
          if (player->bankroll < player->worst_bankroll) {
            player->worst_bankroll = player->bankroll;
          }
          player->losses++;
        }

        nextAction = DealerAction::StartNewHand;
        player->actionRequired = PlayerActionRequired::None;
        return;
        
      } else if (playerBlackack) {
        player->current_result += blackjack_pays * player->currentHand->bet;
        player->bankroll += blackjack_pays * player->currentHand->bet;
        player->blackjacksPlayer++;
        
        info(player, Info::PlayerWins, 1e3 * blackjack_pays*player->currentHand->bet);
        player->wins++;
        player->winsBlackjack++;

        nextAction = DealerAction::StartNewHand;
        player->actionRequired = PlayerActionRequired::None;
        return;
        
      } else {
        // only if the dealer had the chance to have a blackjack we say "no_blackjacks"
        if (card[upCard].value == 10 || card[upCard].value == 11) {
          info(player, Info::NoBlackjacks);
        }
        
        nextAction = DealerAction::AskForPlay;
        player->actionRequired = PlayerActionRequired::Play;
        return;
      }        
    break;
    
    case DealerAction::AskForPlay:
      player->actionRequired = PlayerActionRequired::Play;
      nextAction = DealerAction::AskForPlay;
      return;
    break;
    
    case DealerAction::MoveOnToNextHand:
      // see if we finished all the player's hands
      if (++player->currentHand != player->hands.end()) {
        unsigned int playerCard = drawCard(&(*player->currentHand));
        info(player, Info::CardPlayer, playerCard);

        if (std::abs(player->currentHand->total()) == 21) {
          player->actionRequired = PlayerActionRequired::None;
          nextAction = DealerAction::MoveOnToNextHand;
          return;
        }  else  {
          player->actionRequired = PlayerActionRequired::Play;
          nextAction = DealerAction::AskForPlay;
          return;
        }
      } else {
        // assume the player busted in all the hands
        bool bustedAllHands = true;
        for (auto playerHand : player->hands) {
          // if she did not bust, set false
          if (playerHand.busted() == false) {
            bustedAllHands = false;
          }
        }

        if (bustedAllHands) {
          info(player, Info::CardDealerRevealsHole, holeCard);
          
          player->actionRequired = PlayerActionRequired::None;
          nextAction = DealerAction::StartNewHand;
          return;
        }  else {
          player->actionRequired = PlayerActionRequired::None;
          nextAction = DealerAction::HitDealerHand;
          return;
        }
      }        
    break;
    
    case DealerAction::HitDealerHand:
        
      info(player, Info::CardDealerRevealsHole, holeCard);

      // hit while count is less than 17 (or equal to soft 17 if hit_soft_17 is true)
      dealerTotal = hand.total();
      while ((std::abs(dealerTotal) < 17 || (hit_soft_17 && dealerTotal == -17)) && hand.busted() == 0) {
        unsigned int dealerCard = drawCard(&hand);
        info(player, Info::CardDealer, dealerCard);
        dealerTotal = hand.total();
      }
        

      if (hand.busted()) {
        info(player, Info::DealerBusts, dealerTotal);
        player->bustsDealer++;
        for (auto playerHand : player->hands) {
          if (playerHand.busted() == false) {
            info(player, Info::PlayerWins, 1e3*playerHand.bet);
            player->current_result += playerHand.bet;
            player->bankroll += playerHand.bet;
            player->wins++;
            player->winsDoubled += playerHand.doubled;
          }
        }
      } else {
        for (auto playerHand : player->hands) {
          if (playerHand.busted() == false) {  // busted hands have already been solved
            playerTotal = std::abs(playerHand.total());
           
            if (dealerTotal > playerTotal) {
                
              info(player, Info::PlayerLosses, 1e3*playerHand.bet, playerTotal);
              player->bankroll -= playerHand.bet;
              if (player->bankroll < player->worst_bankroll) {
                player->worst_bankroll = player->bankroll;
              }
              player->losses++;
                
            } else if (dealerTotal == playerTotal) {
                  
              info(player, Info::PlayerPushes, 1e3*playerHand.bet);
              player->pushes++;
                
            } else {
                
              info(player, Info::PlayerWins, 1e3*playerHand.bet, playerTotal);
              player->current_result += playerHand.bet;
              player->bankroll += playerHand.bet;
              player->wins++;

              if (playerHand.doubled) {
                player->winsDoubled++;
              } else {
                player->wins++;
              }
            }
          }
        }
      }
               

      player->actionRequired = PlayerActionRequired::None;
      nextAction = DealerAction::StartNewHand;
      return;
    break;

    case DealerAction::None:
    break;

  }
          
}

// returns zero if it is a common command and we need to ask again
// returns positive if what was asked was answered
// returns negative if what was aked was not asnwered or the command does not apply
int Blackjack::process(Player *player) {
  
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
    case PlayerActionTaken::Quit:
      finished(true);
      return 1;
    break;    
        
///ig+help+name help
///ig+help+desc Ask for help
///ig+help+detail A succinct help message is written on the standard output.
///ig+help+detail This command makes sense only when issued by a human player.
    case PlayerActionTaken::Help:
      info(player, Info::Help);  
      return 0;
    break;  
    
    case PlayerActionTaken::None:
      return 0;
    break;
      
      
    // if we made it this far, the command is particular
    case PlayerActionTaken::Bet:
      // TODO: bet = 0 -> wonging
      if (player->currentBet == 0) {
        info(player, Info::InvalidBet, player->currentBet);
        return 0;
      } else if (player->currentBet < 0) {
        info(player, Info::InvalidBet, player->currentBet);
        return 0;
      } else if (max_bet != 0  && player->currentBet > max_bet) {
        info(player, Info::InvalidBet, player->currentBet);
        return 0;
      } else {
        // ok, valid bet
        player->currentHand->bet = player->currentBet;
        nextAction = DealerAction::DealPlayerFirstCard;
        return 1;
      }
    break;

    case PlayerActionTaken::Insure:
      // TODO: allow insurance for less than one half of the original bet
      player->currentHand->insured = true;
      player->current_result -= 0.5 * player->currentHand->bet;
      player->bankroll -= 0.5 * player->currentHand->bet;
      player->handsInsured++;
         
      player->actionRequired = PlayerActionRequired::None;
      nextAction = DealerAction::CheckforBlackjacks;
      return 1;
    break;

    case PlayerActionTaken::DontInsure:
      player->currentHand->insured = false;
      player->actionRequired = PlayerActionRequired::None;
      nextAction = DealerAction::CheckforBlackjacks;
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
    case PlayerActionTaken::Stand:
      player->actionRequired = PlayerActionRequired::None;
      nextAction = DealerAction::MoveOnToNextHand;
      return 1;
    break;  
    
///ip+double+name double
///ip+double+desc Double down on the current hand
///ip+double+detail The player adds the same amount waged on the current hand
///ip+double+detail and in exchange she receives only one hand.
///ip+double+detail Doubling down is allowed only after receiving the first
///ip+double+detail two cards.
///ip+double+detail This command can be abbreviated as `d`.
    case PlayerActionTaken::Double:
      if (player->currentHand->cards.size() == 2) {

        // TODO: check bankroll
        player->total_money_waged += player->currentHand->bet;
        player->currentHand->bet *= 2;
        player->currentHand->doubled = true;
        player->handsDoubled++;

        playerCard = drawCard(&(*player->currentHand));
        unsigned int playerTotal = player->currentHand->total();
        info(player, Info::CardPlayer, playerCard);

        if (player->currentHand->busted()) {
          info(player, Info::PlayerLosses, 1e3*player->currentHand->bet, playerTotal);
          player->current_result -= player->currentHand->bet;
          player->bankroll -= player->currentHand->bet;
          player->bustsPlayer++;
          player->losses++;
  }

        player->actionRequired = PlayerActionRequired::None;
        nextAction = DealerAction::MoveOnToNextHand;
        return 1;
        
      } else {
          
        std::cout << "cannot_double" << std::endl;
        return -1;
          
      }
    break;  

///ip+split+name split
///ip+split+desc Split the current hand
///ip+split+detail 
///ip+split+detail This command can be abbreviated as `p` (for pair).
    case PlayerActionTaken::Split:

      firstCard  = *(player->currentHand->cards.begin());
      secondCard = *(++player->currentHand->cards.begin());
      
      // up to three splits (i.e. four hands)
      // TODO: choose through conf
      // TODO: check bankroll to see if player can split
      if (player->currentSplits < 3 && player->currentHand->cards.size() == 2 && card[firstCard].value == card[secondCard].value) {
        // mark that we split to put ids in the hands and to limi the number of spltis
        player->currentSplits++;

        // the first hand is id=1, then we add one
        if (player->currentHand == player->hands.begin()) {
          player->currentHand->id = 1;
        }
        
        // create a new hand
        PlayerHand newHand;
        newHand.id = player->currentHand->id + 1;
        newHand.bet = player->currentHand->bet;
        player->total_money_waged += player->currentHand->bet;
        
        // remove second the card from the first hand
        player->currentHand->cards.pop_back();
        
        // and put it into the second hand
        newHand.cards.push_back(secondCard);

        // add the new hand to the list of hands        
        player->hands.push_back(std::move(newHand));

        // deal a card to the first hand
        playerCard = drawCard(&(*player->currentHand));
        info(player, Info::CardPlayer, playerCard);

        // aces get dealt only one card
        // also, if the player gets 21 then we move on to the next hand
        if (card[*player->currentHand->cards.begin()].value == 11 || std::abs(player->currentHand->total()) == 21) {
          if (++player->currentHand != player->hands.end()) {
            info(player, Info::PlayerNextHand, (*player->currentHand).id);
            playerCard = drawCard(&(*player->currentHand));
            info(player, Info::CardPlayer, playerCard);

            // if the player got an ace or 21 again, we are done
            if (card[*player->currentHand->cards.begin()].value == 11 || std::abs(player->currentHand->total()) == 21) {
              player->actionRequired = PlayerActionRequired::None;
              nextAction = DealerAction::MoveOnToNextHand;
              return 1;
            } else {
              player->actionRequired = PlayerActionRequired::Play;
              nextAction = DealerAction::AskForPlay;
              return 1;
            }
          } else {
            player->actionRequired = PlayerActionRequired::None;
            nextAction = DealerAction::MoveOnToNextHand;
            return 1;
          }  
        } else {
          player->actionRequired = PlayerActionRequired::Play;
          nextAction = DealerAction::AskForPlay;
          return 1;
        }
      } else {

        std::cout << "cannot_split" << std::endl;
        return -1;
          
      }
    break;
      
    case PlayerActionTaken::Hit:
        
///ip+hit+name hit
///ip+hit+desc Hit on the current hand
///ip+hit+detail 
///ip+hit+detail This command can be abbreviated as `h`.
      playerCard = drawCard(&(*player->currentHand));        
      info(player, Info::CardPlayer, playerCard);

      if (player->currentHand->busted()) {
        info(player, Info::PlayerLosses, 1e3*player->currentHand->bet);
        player->current_result -= player->currentHand->bet;
        player->bankroll -= player->currentHand->bet;
        player->bustsPlayer++;
        player->losses++;

        player->actionRequired = PlayerActionRequired::None;
        nextAction = DealerAction::MoveOnToNextHand;
        return 1;
      } else if (std::abs(player->currentHand->total()) == 21) {
        player->actionRequired = PlayerActionRequired::None;
        nextAction = DealerAction::MoveOnToNextHand;
        return 1;
      } else {
        player->actionRequired = PlayerActionRequired::Play;
        nextAction = DealerAction::AskForPlay;
        return 1;
      }
    break;
    
    default:

      std::cout << "invalid_command" << std::endl;
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
