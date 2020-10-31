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

#include "blackjack.h"

Blackjack::Blackjack() {
  std::cout << "I'm your Blackjack dealer!" << std::endl;
}

Blackjack::~Blackjack() {
  std::cout << "Bye bye! We'll play Blackjack again next time." << std::endl;
}

void Blackjack::deal(Player *player) {
    
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
      
      // clear dealer's hand, create a new one and add it to the list
      hands.clear();
      hands.push_back(std::move(Hand()));

      // erase all the player's, create one, add and make it the current one
      player->hands.clear();
      player->hands.push_back(std::move(Hand()));
      player->currentHand = player->hands.begin();
      
      // state that the player did not win anything nor splitted nor doubled down
      player->current_result = 0;
      player->hasSplit = 0;
      player->hasDoubled = 0;
      
      if (lastPass) {
        // tell people we are shuffling
	//  bjcall (blackjack.current_player->write (player, "shuffling"));
          
        // shuffle the cards          
	// shuffle_shoe ();	
          
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

//      bjcall (blackjack.current_player->write (player, "new_hand"));
      // TODO: think!
      std::cout << "new_hand" << std::endl;
      
    break;
    
    
    // -------------------------------------------------------------------------  
    case DealerAction::AskForBets:
      // step 1. ask for bets
      // TODO: use an output buffer to re-ask in case no number comes back
      std::cout << "bet?" << std::endl;
      // TODO: setter
      player->actionRequired = PlayerActionRequired::Bet;
      setInputNeeded(true);
    break;

    case DealerAction::DealPlayerFirstCard:
      // where's step 2?
      // step 3. deal the first card to each player
      player->n_hands++;	// splits are counted as a single hand
      player->total_money_waged += player->currentHand->bet;

      Card card;
      card.tag = 7;
      player->currentHand->cards.push_back(card);
      std::cout << "card_player_first " << player->currentHand->cards.begin()->tag << std::endl;
//      bjcall (write_formatted_card (player, 0, "card_player_first", card));
      
/*
      // step 4. show dealer's upcard
      card = deal_card_to_hand (blackjack.dealer_hand);
      bjcall (write_formatted_card (player, 0, "card_dealer_up", card));
      if (stdout_opts.isatty)
	{
	  print_card_art (card);
	}

      // step 5. deal the second card to each player
      card = deal_card_to_hand (player->current_hand);
      bjcall (write_formatted_card (player, 0, "card_player_second", card));
      if (stdout_opts.isatty)
	{
	  print_hand_art (player->current_hand);
	}

      // TODO: ENHC
      blackjack.next_dealer_action = DEAL_DEALERS_HOLE_CARD;
      break;
*/
        
    break;
    
    
/*    
    case DealerAction::DealDealerHoleCard:
    break;
    case DealerAction::AskForInsurance:
    break;
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
