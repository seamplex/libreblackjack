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
//#include <random>
#include <cstdlib>
#include <ctime>

#include "blackjack.h"

Blackjack::Blackjack() : mt19937(dev_random()), fiftyTwoCards(0, 51) {
  std::cout << "I'm your Blackjack dealer!" << std::endl;
  
  // TODO: seed instead of dev_random
  std::random_device random_device;
  std::mt19937 random_engine(random_device());
  std::uniform_int_distribution<int> distribution_1_100(1, 100);

}

Blackjack::~Blackjack() {
  std::cout << "Bye bye! We'll play Blackjack again next time." << std::endl;
}

void Blackjack::deal(Player *player) {
  
  int playerTotal = 0;
  int dealerTotal = 0;
  bool playerBlackack = false;
  // let's start by assuming the player does not need to do anything
  player->actionRequired = PlayerActionRequired::None;
    
  switch(next_action) {
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

      infinite_decks_card_number_for_arranged_ones = 0;
      n_hand++;
      
      // clear dealer's hand
      hand.holeCardShown = false;
      hand.cards.clear();

      // erase all the player's, create one, add and make it the current one
      player->hands.clear();
      player->hands.push_back(std::move(PlayerHand()));
      player->currentHand = player->hands.begin();
      
      // state that the player did not win anything nor splitted nor doubled down
      player->current_result = 0;
      player->hasSplit = 0;
      player->hasDoubled = 0;
      
      if (lastPass) {
        // TODO: send informative messages to the player
        // tell people we are shuffling
        //  bjcall (blackjack.current_player->write (player, "shuffling"));
          
        // shuffle the cards          
        shuffle();        
          
        // TODO: reset card counting systems
        // burn as many cards as asked
        for (int i = 0; i < number_of_burnt_cards; i++) {
//          dealCard();
        }
        lastPass = false;
      }
      
      if (player->flatBet) {
        player->currentHand->bet = player->flatBet;
        setNextAction(DealerAction::DealPlayerFirstCard);
      } else {
        setNextAction(DealerAction::AskForBets);
      }

      std::cout << "new_hand" << std::endl;
      return;
      
    break;
    
    
    // -------------------------------------------------------------------------  
    case DealerAction::AskForBets:
      // step 1. ask for bets
      // TODO: use an output buffer to re-ask in case no number comes back
      std::cout << "bet?" << std::endl;
      // TODO: setter
      player->actionRequired = PlayerActionRequired::Bet;
      return;
    break;

    case DealerAction::DealPlayerFirstCard:
      // where's step 2? <- probably that's the player's bet
      // step 3. deal the first card to each player
      player->n_hands++;        // splits are counted as a single hand
      player->total_money_waged += player->currentHand->bet;

      playerFirstCard = dealCard(&(*player->currentHand));
//       std::cout << "card_player_first " << card[playerFirstCard].ascii() << std::endl;
//       std::cout << "card_player_first " << card[playerFirstCard].text() << std::endl;
      std::cout << "card_player_first " << card[playerFirstCard].utf8() << std::endl;
      
      // step 4. show dealer's upcard
      upCard = dealCard(&hand);
      std::cout << "card_dealer_up " << card[upCard].utf8() << std::endl;

      // step 5. deal the second card to each player
      playerSecondCard = dealCard(&(*player->currentHand));
      std::cout << "card_player_second " << card[playerSecondCard].utf8() << std::endl;
      
      
      // step 6. deal the dealer's hole card 
      holeCard = dealCard(&hand);
      std::cout << "card_dealer_hole" << std::endl;

      hand.render(hand.holeCardShown);
      player->currentHand->render();

      // step 7.a. if the upcard is an ace ask for insurance
      if (card[upCard].value == 11) {
        if (player->no_insurance == false && player->always_insure == false) {
          player->actionRequired = PlayerActionRequired::Insurance;
          setNextAction(DealerAction::AskForInsurance);
          std::cout << "next ask insurance" << std::endl;
          return;
        
        } else if (player->always_insure) {
          player->currentHand->insured = true;
          // TODO: allow insurance for less than one half of the original bet
          player->current_result -= 0.5 * player->currentHand->bet;
          player->bankroll -= 0.5 * player->currentHand->bet;
          player->handsInsured++;
          
          player->actionRequired = PlayerActionRequired::None;
          setNextAction(DealerAction::CheckforBlackjacks);
          return;
        }
      }
      
      // step 7.b. if either the dealer or the player has a chance to have a blackjack, check
      playerTotal = player->currentHand->total();
      if ((card[upCard].value == 10 || card[upCard].value == 11) || playerTotal == 21) {
        player->actionRequired = PlayerActionRequired::None;
        setNextAction(DealerAction::CheckforBlackjacks);
        std::cout << "next check BJs" << std::endl;
        return;
      }

      // step 7.c. ask the player to play
      player->actionRequired = PlayerActionRequired::Play;
      setNextAction(DealerAction::AskForPlay);
      std::cout << "dealer upcard is " << card[upCard].utf8() << std::endl;
      std::cout << "your total is " << playerTotal << std::endl;
      std::cout << "play please" << std::endl;
      return;
    break;
    
    case DealerAction::AskForInsurance:
      std::cout << "next action do you want insurance?" << std::endl;  
      return;
    break;
    
    case DealerAction::CheckforBlackjacks:
      // step 8. check if there are any blackjack
      playerBlackack = player->currentHand->blackjack();
      if (hand.blackjack()) {
        std::cout << "card_dealer_hole" << card[holeCard].utf8() << std::endl;
        std::cout << "blackjack_dealer" << std::endl;
        player->blackjacksDealer++;
        // print_hand_art (blackjack.dealer_hand);

        if (player->currentHand->insured) {
          std::cout << "player_wins_insurance " << player->currentHand->bet << std::endl;
          player->current_result += player->currentHand->bet;
          player->bankroll += player->currentHand->bet;
          player->winsInsured++;
        }

        if (playerBlackack) {
          std::cout << "blackjack_player_also" << std::endl;
          player->blackjacksPlayer++;
          if (player->hasSplit) {
            std::cout << "player_pushes " << player->currentHand->bet << " #" << player->currentHand->id << std::endl;
          } else {
            std::cout << "player_pushes " << player->currentHand->bet << std::endl;
          }
          player->pushes++;
          
          //  print_hand_art (player->current_hand);
        } else {
          if (player->hasSplit) {
            std::cout << "player_losses " << player->currentHand->bet << " #" << player->currentHand->id << std::endl;
          } else {
            std::cout << "player_losses " << player->currentHand->bet << std::endl;
          }
          player->current_result -= player->currentHand->bet;
          player->bankroll -= player->currentHand->bet;
          if (player->bankroll < player->worst_bankroll) {
            player->worst_bankroll = player->bankroll;
          }
          player->losses++;
        }

        setNextAction(DealerAction::StartNewHand);
        player->actionRequired = PlayerActionRequired::None;
        std::cout << "next start a new hand" << std::endl;
        return;
        
      } else if (playerBlackack) {
        std::cout << "blackjack_player" << std::endl;
        player->current_result += blackjack_pays * player->currentHand->bet;
        player->bankroll += blackjack_pays * player->currentHand->bet;
        player->blackjacksPlayer++;
          
        std::cout << "player_wins " << blackjack_pays * player->currentHand->bet << std::endl;
        player->wins++;
        player->winsBlackjack++;

        setNextAction(DealerAction::StartNewHand);
        player->actionRequired = PlayerActionRequired::None;
        std::cout << "next start a new hand" << std::endl;
        return;
        
      } else {
        // only if the dealer had the chance to have a blackjack we say "no_blackjacks"
        if (card[upCard].value == 10 || card[upCard].value == 11) {
          std::cout << "no_blackjacks" << std::endl;
        }
        
        setNextAction(DealerAction::AskForPlay);
        player->actionRequired = PlayerActionRequired::Play;
        std::cout << "prepare to play" << std::endl;
        return;
      }        
    break;
    
/*
    case DealerAction::PayOrTakeInsuranceBets:
    break;
 */
    case DealerAction::AskForPlay:

      player->actionRequired = PlayerActionRequired::Play;
      setNextAction(DealerAction::AskForPlay);
      
      hand.render(hand.holeCardShown);
      player->currentHand->render();
      
      std::cout << "dealer upcard is " << card[upCard].utf8() << std::endl;
      std::cout << "your total is " << playerTotal << std::endl;
      std::cout << "play please" << std::endl;
      return;
    break;
    case DealerAction::MoveOnToNextHand:
      // see if we finished all the player's hands
      if (++player->currentHand != player->hands.end()) {
        unsigned int playerCard = dealCard(&(*player->currentHand));
        if (player->hasSplit && player->currentHand->cards.size() == 2) {
          std::cout << "card_player_second " << card[playerCard].utf8() << std::endl;
        } else {
          std::cout << "card_player " << card[playerCard].utf8() << std::endl;
        }  
        player->currentHand->render();

        if (player->currentHand->total() == 21) {
          player->actionRequired = PlayerActionRequired::None;
          setNextAction(DealerAction::MoveOnToNextHand);
          return;
        }  else  {
          player->actionRequired = PlayerActionRequired::Play;
          setNextAction(DealerAction::AskForPlay);
          return;
        }
      } else {
        // assume the player busted in all the hands
        player->bustedAllHands = true;
        for (auto playerHand : player->hands) {
          // if she did not bust, set zero
          if (playerHand.busted() == false) {
            player->bustedAllHands = false;
          }
        }

        if (player->bustedAllHands) {
          std::cout << "player_bustd_all_hands" << std::endl;
          std::cout << "card_dealer_hole " << card[holeCard].utf8() << std::endl;
          hand.holeCardShown = true;
          std::cout << "dealer_hand" << std::endl;
          hand.render(hand.holeCardShown); 
          
          // TODO: no tengo que sacarle todo el dinero?
          
          player->actionRequired = PlayerActionRequired::None;
          setNextAction(DealerAction::StartNewHand);
          return;
        }  else {
          player->actionRequired = PlayerActionRequired::None;
          setNextAction(DealerAction::HitDealerHand);
          return;
        }
      }        
    break;
    
    case DealerAction::HitDealerHand:
        
      std::cout << "card_dealer_hole" << card[holeCard].utf8() << std::endl;
      hand.holeCardShown = true;

      hand.render(hand.holeCardShown);

      // TODO: print "soft"
      std::cout << "dealer_count " << hand.total() << std::endl;

      // hit if count is less than 17 (or equalt to soft 17 if hit_soft_17 is true)
      dealerTotal = hand.total();
      while (((abs(dealerTotal) < 17 || (hit_soft_17 && dealerTotal == -17))) && hand.busted() == 0) {
        unsigned int dealerCard = dealCard(&hand);
        std::cout << "card_dealer " << card[dealerCard].utf8() << std::endl;
        hand.render(hand.holeCardShown);
                
        dealerTotal = abs(hand.total());
        std::cout << "dealer_count " << dealerTotal << std::endl;

        if (hand.busted()) {
          std::cout << "busted_dealer " << dealerTotal << std::endl;
	  player->bustsDealer++;
	  for (auto playerHand : player->hands) {
            if (playerHand.busted() == false) {
              // TODO: split
              std::cout << "player_wins " << playerHand.bet << std::endl;
		    
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
	} else {
	  for (auto playerHand : player->hands) {
            if (playerHand.busted() == false) {  // busted hands have already been solved
              unsigned int playerTotal = abs(playerHand.total());
            
              if (dealerTotal > playerTotal) {
                  
                std::cout << "player_losses " << playerHand.bet << std::endl;
                player->bankroll -= playerHand.bet;
		if (player->bankroll < player->worst_bankroll) {
	          player->worst_bankroll = player->bankroll;
		}
		player->losses++;
                
	      } else if (dealerTotal == playerTotal) {
                  
                std::cout << "player_pushes " << playerHand.bet << std::endl;
                player->pushes++;
                
              } else {
                
                std::cout << "player_wins " << playerHand.bet << std::endl;
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
      }
               

      player->actionRequired = PlayerActionRequired::None;
      setNextAction(DealerAction::StartNewHand);
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
  
  switch (player->actionTaken) {

   // TODO: maybe we have to call a basic method with common commands?   
      
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
      // TODO
    case PlayerActionTaken::Help:
      std::cout << "help yourself" << std::endl;
      return 0;
    break;  
      
    // TODO:
    case PlayerActionTaken::Count:
    break;
    case PlayerActionTaken::UpcardValue:
    break;
    case PlayerActionTaken::Bankroll:
    break;
    case PlayerActionTaken::Hands:
    break;
    case PlayerActionTaken::Table:
    break;
    case PlayerActionTaken::None:
      return 0;
    break;
      
      
    // if we made it this far, the command is particular
    case PlayerActionTaken::Bet:
      // TODO: bet = 0 -> wonging
      if (player->currentBet == 0) {
        std::cout << "bet_zero" << std::endl;
        return 0;
      } else if (player->currentBet < 0) {
        std::cout << "bet_negative" << std::endl;
        return 0;
      } else if (max_bet != 0  && player->currentBet > max_bet) {
        std::cout << "bet_maximum" << max_bet << std::endl;
        return 0;
      } else {
        // ok, valid bet
        player->currentHand->bet = player->currentBet;
        setNextAction(DealerAction::DealPlayerFirstCard);
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
      setNextAction(DealerAction::CheckforBlackjacks);
      return 1;
    break;

    case PlayerActionTaken::DontInsure:
      player->currentHand->insured = false;
      player->actionRequired = PlayerActionRequired::None;
      setNextAction(DealerAction::CheckforBlackjacks);
      return 1;
    break;
      
    case PlayerActionTaken::Hit:
      std::cout << "ok, you hit" << std::endl;
      finished(true);
      return 1;
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


int Blackjack::dealCard(Hand *hand) {
    
  unsigned int tag = 0;

  if (n_decks == -1) {
      
    // TODO: arranged cards
    tag = fiftyTwoCards(mt19937);
    
  } else {
    // TODO: shoes
    tag = 0;
  }
    
  if (hand != nullptr) {
    hand->cards.push_back(tag);
  }
  
  return tag;
}
