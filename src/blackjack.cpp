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

Blackjack::Blackjack() {
  std::cout << "I'm your Blackjack dealer!" << std::endl;
  
  // TODO: better RNGs 
  // https://codeforces.com/blog/entry/61587
  srand((int)time(0));

}

Blackjack::~Blackjack() {
  std::cout << "Bye bye! We'll play Blackjack again next time." << std::endl;
}

void Blackjack::deal(Player *player) {
  
  // let's start by assuming the player does not need to do anything
  setInputNeeded(false);    
    
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
      player->hands.push_back(std::move(Hand()));
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
      setInputNeeded(true);
      return;
    break;

    case DealerAction::DealPlayerFirstCard:
      // where's step 2? <- probably that's the player's bet
      // step 3. deal the first card to each player
      player->n_hands++;	// splits are counted as a single hand
      player->total_money_waged += player->currentHand->bet;

      playerFirstCard = dealCard(&(*player->currentHand));
      std::cout << "card_player_first " << card[playerFirstCard].ascii() << std::endl;
      std::cout << "card_player_first " << card[playerFirstCard].text() << std::endl;
      std::cout << "card_player_first " << card[playerFirstCard].utf8() << std::endl;
      
      // step 4. show dealer's upcard
      upCard = dealCard(&hand);
      std::cout << "card_dealer_up " << card[upCard].text() << std::endl;

      // step 5. deal the second card to each player
      playerSecondCard = dealCard(&(*player->currentHand));
      std::cout << "card_player_second " << card[playerSecondCard].utf8() << std::endl;
      
      
      // step 6. deal the dealer's hole card 
      holeCard = dealCard(&hand);
      std::cout << "card_dealer_hole" << std::endl;

      // TODO: print (draw) the hand

      // step 7.a. if the upcard is an ace ask for insurance
      if (card[upCard].value == 11) {
        if (player->no_insurance == false && player->always_insure == false) {
          player->actionRequired = PlayerActionRequired::Insurance;
          setNextAction(DealerAction::AskForInsurance);
          setInputNeeded(true);
          return;
        
        } else if (player->always_insure) {
          player->currentHand->insured = true;
          // TODO: allow insurance for less than one half of the original bet
          player->current_result -= 0.5 * player->currentHand->bet;
          player->bankroll -= 0.5 * player->currentHand->bet;
          player->n_insured_hands++;
          
          player->actionRequired = PlayerActionRequired::None;
          setNextAction(DealerAction::CheckforBlackjacks);
          setInputNeeded(false);
          return;
        }
      }
      
      // step 7.b. if either the dealer or the player has a chance to have a blackjack, check
      if ((card[upCard].value == 10 || card[upCard].value == 11) || player->currentHand->total() == 21) {
        player->actionRequired = PlayerActionRequired::None;
        setNextAction(DealerAction::CheckforBlackjacks);
        setInputNeeded(false);
        return;
      }

      // step 7.c. ask the player to play
      player->actionRequired = PlayerActionRequired::Play;
      setNextAction(DealerAction::AskForPlay);
      setInputNeeded(true);
      return;
    break;
    
    case DealerAction::AskForInsurance:
      std::cout << "hola" << std::endl;  
    break;
    
/*    
    case DealerAction::CheckforBlackjacks:
    break;
    case DealerAction::PayOrTakeInsuranceBets:
    break;
    case DealerAction::AskForPlay:
      std::cout << "Here are your cards" << std::endl;
      setInputNeeded(true);
    break;
    case DealerAction::MoveOnToNextHand:
    break;
    case DealerAction::HitDealerHand:
    break;
    case DealerAction::Payout:
    break;
    case DealerAction::None:
    break;
*/
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
      player->currentHand->insured = true;
      return 1;
    break;

    case PlayerActionTaken::DontInsure:
      player->currentHand->insured = false;
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
    
  int dealt_tag = 0;

  if (n_decks == -1) {
      
    // TODO: arranged cards
    int random_integer = random();
    dealt_tag = (random_integer % 32) + (random_integer % 16) + (random_integer % 4);
    
  } else {
    dealt_tag = 1;
  }
    
  if (hand != nullptr) {
      
  }
  
  return dealt_tag;
}