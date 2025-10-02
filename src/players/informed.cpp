/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - internal fully-informed automatic player
 *
 *  Copyright (C) 2023, 2025 jeremy theler
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
#include <fstream>
#include <sstream>

#include "../conf.h"
#include "../blackjack.h"
#include "informed.h"

namespace lbj {

// max of 2 and 3
#define max(a,b) (((a)>(b))?(a):(b))
#define max3(a,b,c) (((a)>(b) && (a)>(c))?(a):((b)>(c)?(b):(c)))

// best play in ascii
//  D double down
//  H hit
//  S stand
//  Y split
//  N don't split
#define hard_hsd_ascii(player,dealer) (hard_double[player] > hard_hit[player] && hard_double[player] > hard_stand[player]) ? 'D' : (hard_hit[player] > hard_stand[player]) ? 'H' : 'S'
#define soft_hsd_ascii(player,dealer) (soft_double[player] > soft_hit[player] && soft_double[player] > soft_stand[player]) ? 'D' : (soft_hit[player] > soft_stand[player]) ? 'H' : 'S'

#define hard_hsd_ev(player)    max3(hard_double[player], hard_hit[player], hard_stand[player])
#define soft_hsd_ev(player)    max3(soft_double[player], soft_hit[player], soft_stand[player])

// here the first index is the single card value not the total
#define pair_ascii(player,dealer)     (player < 12) ? ((split[player] > hard_hsd_ev(2*player,dealer)) ? 'Y' : 'N') : ((split[player] > soft_hsd_ev(12,dealer)) ? 'Y' : 'N')


#define hard_hsd_action(player) (hard_double[player] > hard_hit[player] && hard_double[player] > hard_stand[player]) ? lbj::PlayerActionTaken::Double : (hard_hit[player] > hard_stand[player]) ? lbj::PlayerActionTaken::Hit : lbj::PlayerActionTaken::Stand
#define soft_hsd_action(player) (soft_double[player] > soft_hit[player] && soft_double[player] > soft_stand[player]) ? lbj::PlayerActionTaken::Double : (soft_hit[player] > soft_stand[player]) ? lbj::PlayerActionTaken::Hit : lbj::PlayerActionTaken::Stand
#define pair_action(player)     (player < 12) ? ((split[player] > hard_hsd_ev(2*player)) ? true : false) : ((split[player] > soft_hsd_ev(12)) ? true : false)


Informed::Informed(Configuration &conf) : Player(conf) {
  // TODO: read conf

  // TODO. move to reset    
  decks = 1;
  remaining_cards = 52*decks;
  for (int rank = 1; rank < 9; rank++) {
    remaining[rank] = 4*decks;
  }
  // first  4 = 4 ranks (T,J,Q,K)
  // second 4 = 4 suits (H,S,C,D)
  remaining[10] = 4*4*decks;
  
  if (decks != 0) {
    verbose = true;
  }
  
  return;
}

int Informed::play() {

  std::size_t value = 0;
  std::size_t upcard = 0;
  
  switch (actionRequired) {
    case lbj::PlayerActionRequired::Bet:
      // TODO: compute expected value and change bet
      currentBet = 1;
      actionTaken = lbj::PlayerActionTaken::Bet;
    break;

    case lbj::PlayerActionRequired::Insurance:
      // TODO: compute  
      actionTaken = lbj::PlayerActionTaken::DontInsure;
    break;
    
    case lbj::PlayerActionRequired::Play:

#ifdef BJDEBUG
      std::cout << "player " << playerValue << " dealer " << dealerValue << std::endl;
#endif      
      value = std::abs(playerValue);
      upcard = std::abs(dealerValue);
      
      // -------------------------------------------------------
      // compute the expected values
      // -------------------------------------------------------
      init();
      for (int i = 0; i < 8; i++) {
        dealer_bust_european_iteration();
      }
      dealer_european_to_american();

      stand(upcard);
      
      for (int i = 0; i < 8; i++) {
        hit_iteration();
      }
      double_down();
      // -------------------------------------------------------      
      
      actionTaken = lbj::PlayerActionTaken::None;
      if (canSplit) {
        pairs();
        if ((playerValue == -12 && pair_action(11)) || pair_action(value/2)) {
          actionTaken = lbj::PlayerActionTaken::Split;
        }
      }
      
      if (actionTaken == lbj::PlayerActionTaken::None) {
        actionTaken = (playerValue < 0) ? soft_hsd_action(value) : hard_hsd_action(value);
        
        if (canDouble == false && actionTaken == lbj::PlayerActionTaken::Double) {
          actionTaken = lbj::PlayerActionTaken::Hit;
        }
      }
      
    break;  
    
    case lbj::PlayerActionRequired::None:
      std::cout << "mongocho" << std::endl;
        // TODO: count cards!
    break;  
    
  }
  
  return 0;
}


void Informed::init(void) {
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      dealer_hard[i][j] = 0;
    }
  }
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      dealer_soft[i][j] = 0;
    }
  }
  // assume everything is lost
  for (int i = 0; i < SIZE; i++) {
    hard_stand[i] = -1;
    soft_stand[i] = -1;
    hard_hit[i] = -1;
    soft_hit[i] = -1;
    hard_double[i] = -2;
    soft_double[i] = -2;
    split[i] = -1;
  }    
  return;
}  




void Informed::dealer_bust_european_iteration(void) {
  // dealer_hard[final][initial] stores the probability that the dealer will get
  // a total equal to the first (final) index given that
  //  the current hand is equal to the second index (initial)
  // same thing for dealer_soft[][]  
    
  // if the dealer has a hard 17 he has to stand
  // therefore, the probability of getting a total equal to 17 given that he has 17 is one
  // and same thing with an 18 through a 21.
  // same for soft hands, and let's assume s17
  // TODO: h17
  for (int total = 17; total < 22; total++) {
    dealer_hard[total][total] = 1;    
    dealer_soft[total][total] = 1;
  }
  
  // if the dealer has hard 22 or more, chances of busting are 100%
  for (int total = 22; total < 32; total++) {
    dealer_hard[22][total] = 1;    
  }

  //
  for (int final_total = 17; final_total < 23; final_total++) {
    for (int initial = 16; initial > 1; initial--) {
      // the probability of getting final_total starting from initial_total is 
      // the sum over n of the existing probabilities (initial+n)->final * chances of getting a card equal to n
      // TODO: real cards left
      dealer_hard[final_total][initial] = 1.0/13.0*(dealer_hard[final_total][initial+2]) +
                                          1.0/13.0*(dealer_hard[final_total][initial+3]) +
                                          1.0/13.0*(dealer_hard[final_total][initial+4]) +
                                          1.0/13.0*(dealer_hard[final_total][initial+5]) +
                                          1.0/13.0*(dealer_hard[final_total][initial+6]) +
                                          1.0/13.0*(dealer_hard[final_total][initial+7]) +
                                          1.0/13.0*(dealer_hard[final_total][initial+8]) +
                                          1.0/13.0*(dealer_hard[final_total][initial+9]) +
                                          4.0/13.0*(dealer_hard[final_total][initial+10]) +
                                          1.0/13.0*(dealer_soft[final_total][initial+11]);
    }

    // With a soft 22, that's going to be the same thing as a hard 12.  
    for (int initial = 31; initial > 21; initial--) {
       dealer_soft[final_total][initial] = dealer_hard[final_total][initial-10];
    }
    
    for (int initial = 16; initial > 11; initial--) {
      dealer_soft[final_total][initial] = 1.0/13.0*(dealer_soft[final_total][initial+1]) +
                                          1.0/13.0*(dealer_soft[final_total][initial+2]) +
                                          1.0/13.0*(dealer_soft[final_total][initial+3]) +
                                          1.0/13.0*(dealer_soft[final_total][initial+4]) +
                                          1.0/13.0*(dealer_soft[final_total][initial+5]) +
                                          1.0/13.0*(dealer_soft[final_total][initial+6]) +
                                          1.0/13.0*(dealer_soft[final_total][initial+7]) +
                                          1.0/13.0*(dealer_soft[final_total][initial+8]) +
                                          1.0/13.0*(dealer_soft[final_total][initial+9]) +
                                          4.0/13.0*(dealer_soft[final_total][initial+10]);
    }
  }
  
  return;
}

void Informed::dealer_european_to_american(void) {

  // TODO: explain!
  for (int outcome = 17; outcome < 23; outcome++) {
    for (int total = 2; total < 10; total++) {
      dealer_american[outcome][total] = dealer_hard[outcome][total];
    }
    
    dealer_american[outcome][10] = 1.0/12.0*(dealer_hard[outcome][10+2]) +
                                   1.0/12.0*(dealer_hard[outcome][10+3]) +
                                   1.0/12.0*(dealer_hard[outcome][10+4]) +
                                   1.0/12.0*(dealer_hard[outcome][10+5]) +
                                   1.0/12.0*(dealer_hard[outcome][10+6]) +
                                   1.0/12.0*(dealer_hard[outcome][10+7]) +
                                   1.0/12.0*(dealer_hard[outcome][10+8]) +
                                   1.0/12.0*(dealer_hard[outcome][10+9]) +
                                   4.0/12.0*(dealer_hard[outcome][10+10]);
    
    dealer_american[outcome][11] = 1.0/9.0*(dealer_soft[outcome][11+1] +
                                            dealer_soft[outcome][11+2] +
                                            dealer_soft[outcome][11+3] +
                                            dealer_soft[outcome][11+4] +
                                            dealer_soft[outcome][11+5] +
                                            dealer_soft[outcome][11+6] +
                                            dealer_soft[outcome][11+7] +
                                            dealer_soft[outcome][11+8] +
                                            dealer_soft[outcome][11+9]);
  }
  
  return;
}

void Informed::hit_iteration() {
  
  // do not go below 3 if not needed by value  
  for (int player = 20; player > 3; player--) {
    hard_hit[player] = 1.0/13.0*(max(hard_stand[player+2], hard_hit[player+2])) +
                       1.0/13.0*(max(hard_stand[player+3], hard_hit[player+3])) +
                       1.0/13.0*(max(hard_stand[player+4], hard_hit[player+4])) +
                       1.0/13.0*(max(hard_stand[player+5], hard_hit[player+5])) +
                       1.0/13.0*(max(hard_stand[player+6], hard_hit[player+6])) +
                       1.0/13.0*(max(hard_stand[player+7], hard_hit[player+7])) +
                       1.0/13.0*(max(hard_stand[player+8], hard_hit[player+8])) +
                       1.0/13.0*(max(hard_stand[player+9], hard_hit[player+9])) +
                       4.0/13.0*(max(hard_stand[player+10], hard_hit[player+10])) +
                       1.0/13.0*(max(soft_stand[player+11], soft_hit[player+11]));
  }

  for (int player = 31; player > 21; player--) {
    soft_hit[player] = hard_hit[player-10];
  }

  for (int player = 21; player > 11; player--) {
    soft_hit[player] = 1.0/13.0*(max(soft_stand[player+1], soft_hit[player+1])) +
                       1.0/13.0*(max(soft_stand[player+2], soft_hit[player+2])) +
                       1.0/13.0*(max(soft_stand[player+3], soft_hit[player+3])) +
                       1.0/13.0*(max(soft_stand[player+4], soft_hit[player+4])) +
                       1.0/13.0*(max(soft_stand[player+5], soft_hit[player+5])) +
                       1.0/13.0*(max(soft_stand[player+6], soft_hit[player+6])) +
                       1.0/13.0*(max(soft_stand[player+7], soft_hit[player+7])) +
                       1.0/13.0*(max(soft_stand[player+8], soft_hit[player+8])) +
                       1.0/13.0*(max(soft_stand[player+9], soft_hit[player+9])) +
                       4.0/13.0*(max(soft_stand[player+10], soft_hit[player+10]));
  }
  
  return;
}

void Informed::stand(int upcard) {
  // What if the player stands on a four against a two?
  // The only way he's going to win is if the dealer busts.
  // His expected value is the probability of the dealer busting
  // minus the probability of anything else happening.
  //So, he can expect to lose by standing on a four against a two of about 29.3% of his bet. That's the same number for standing on everything all the way through a 16 because a 16 is no better than a four or a zero.
  for (int player = 4; player < 17; player++) {
    hard_stand[player] = dealer_american[22][upcard] - dealer_american[21][upcard] - dealer_american[20][upcard] - dealer_american[19][upcard] - dealer_american[18][upcard] - dealer_american[17][upcard];
  }
  
  hard_stand[17] = dealer_american[22][upcard] - dealer_american[21][upcard] - dealer_american[20][upcard] - dealer_american[19][upcard] - dealer_american[18][upcard];
  hard_stand[18] = dealer_american[22][upcard] - dealer_american[21][upcard] - dealer_american[20][upcard] - dealer_american[19][upcard] + dealer_american[17][upcard] ;
  hard_stand[19] = dealer_american[22][upcard] - dealer_american[21][upcard] - dealer_american[20][upcard] + dealer_american[18][upcard] + dealer_american[17][upcard] ;
  hard_stand[20] = dealer_american[22][upcard] - dealer_american[21][upcard] + dealer_american[19][upcard] + dealer_american[18][upcard] + dealer_american[17][upcard] ;
  hard_stand[21] = dealer_american[22][upcard] + dealer_american[20][upcard] + dealer_american[19][upcard] + dealer_american[18][upcard] + dealer_american[17][upcard] ;
  
  // soft stand
  for (int player = 12; player < 22; player++) {
    soft_stand[player] = hard_stand[player];
  }
  for (int player = 22; player < 32; player++) {
    soft_stand[player] = hard_stand[player-10];
  }

  return; 
}

void Informed::double_down() {
  // The doubling sheet is going to be based on the stand sheet,
  // because when you double, you get one card only. There's no option to hit after that.
  for (int dealer = 2; dealer < 12; dealer++) {
    for (int player = 4; player < 12; player++) {
      hard_double[player] = 2.0/13.0*(hard_stand[player+2]) +
                            2.0/13.0*(hard_stand[player+3]) +    
                            2.0/13.0*(hard_stand[player+4]) +    
                            2.0/13.0*(hard_stand[player+5]) +    
                            2.0/13.0*(hard_stand[player+6]) +    
                            2.0/13.0*(hard_stand[player+7]) +    
                            2.0/13.0*(hard_stand[player+8]) +    
                            2.0/13.0*(hard_stand[player+9]) +    
                            8.0/13.0*(hard_stand[player+10]) +    
                            2.0/13.0*(soft_stand[player+11]);
    }
    
    // With the 12, it gets a little bit different because now the ace is going to count as-- it must count as a one.
    for (int player = 12; player < 22; player++) {
      hard_double[player] = 2.0/13.0*(hard_stand[player+1]) +
                            2.0/13.0*(hard_stand[player+2]) +
                            2.0/13.0*(hard_stand[player+3]) +    
                            2.0/13.0*(hard_stand[player+4]) +    
                            2.0/13.0*(hard_stand[player+5]) +    
                            2.0/13.0*(hard_stand[player+6]) +    
                            2.0/13.0*(hard_stand[player+7]) +    
                            2.0/13.0*(hard_stand[player+8]) +    
                            2.0/13.0*(hard_stand[player+9]) +    
                            8.0/13.0*(hard_stand[player+10]);
      
      soft_double[player] = 2.0/13.0*(soft_stand[player+1]) +
                            2.0/13.0*(soft_stand[player+2]) +
                            2.0/13.0*(soft_stand[player+3]) +    
                            2.0/13.0*(soft_stand[player+4]) +    
                            2.0/13.0*(soft_stand[player+5]) +    
                            2.0/13.0*(soft_stand[player+6]) +    
                            2.0/13.0*(soft_stand[player+7]) +    
                            2.0/13.0*(soft_stand[player+8]) +    
                            2.0/13.0*(soft_stand[player+9]) +    
                            8.0/13.0*(soft_stand[player+10]);
      
    } 
    
    for (int player = 22; player < 32; player++) {
      soft_double[player] = hard_double[player-10];
    }
    
  }
  
  return;
  
}

void Informed::pairs() {
  // TODO: allow resplitting
  for (int player = 2; player < 11; player++) {
    // note that first index here is the single card not the total
    split[player] = 2.0/13.0*(hard_hsd_ev(player+2)) +
                    2.0/13.0*(hard_hsd_ev(player+3)) +    
                    2.0/13.0*(hard_hsd_ev(player+4)) +    
                    2.0/13.0*(hard_hsd_ev(player+5)) +    
                    2.0/13.0*(hard_hsd_ev(player+6)) +    
                    2.0/13.0*(hard_hsd_ev(player+7)) +    
                    2.0/13.0*(hard_hsd_ev(player+8)) +    
                    2.0/13.0*(hard_hsd_ev(player+9)) +    
                    8.0/13.0*(hard_hsd_ev(player+10)) +    
                    2.0/13.0*(soft_hsd_ev(player+11));

    // aces get only one card so they are different
    split[11] = 2.0/13.0*(soft_stand[11+1]) +
                2.0/13.0*(soft_stand[11+2]) +
                2.0/13.0*(soft_stand[11+3]) +
                2.0/13.0*(soft_stand[11+4]) +
                2.0/13.0*(soft_stand[11+5]) +
                2.0/13.0*(soft_stand[11+6]) +
                2.0/13.0*(soft_stand[11+7]) +
                2.0/13.0*(soft_stand[11+8]) +
                2.0/13.0*(soft_stand[11+9]) +
                8.0/13.0*(soft_stand[11+10]);
  }

  return;
}


void Informed::info(lbj::Info msg, int p1, int p2) {

  switch (msg) {

    case lbj::Info::Shuffle:
      std::cout << "Shuffle" << std::endl;
      // TODO: reset()
      
  remaining_cards = 52*decks;
  for (int rank = 1; rank < 9; rank++) {
    remaining[rank] = 4*decks;
  }
  // first  4 = 4 ranks (T,J,Q,K)
  // second 4 = 4 suits (H,S,C,D)
  remaining[10] = 4*4*decks;
      
    break;

    case lbj::Info::NewHand:
        std::cout << "NewHand" << std::endl;
    break;

    case lbj::Info::BetInvalid:
        std::cout << "BetInvalid" << std::endl;
    break;
    
    case lbj::Info::CardPlayer:
        std::cout << "CardPlayer" << std::endl;
        remaining[p1]--;
        remaining_cards--;
    break;

    case lbj::Info::CardDealer:
        std::cout << "CardDealer" << std::endl;
        if (p1 > 0) {
          remaining[p1]--;
          remaining_cards--;
        }
    break;

    case lbj::Info::CardDealerRevealsHole:
        std::cout << "CardDealerRevealsHole" << std::endl;
        remaining[p1]--;
        remaining_cards--;
    break;

    case lbj::Info::DealerBlackjack:
        std::cout << "DealerBlackjack" << std::endl;
    break;

    case lbj::Info::PlayerWinsInsurance:
        std::cout << "PlayerWinsInsurance" << std::endl;
    break;

    case lbj::Info::PlayerBlackjackAlso:
        std::cout << "PlayerBlackjackAlso" << std::endl;
    break;

    case lbj::Info::PlayerSplitInvalid:
        std::cout << "PlayerSplitInvalid" << std::endl;
    break;

    case lbj::Info::PlayerSplitOk:
        std::cout << "PlayerSplitOk" << std::endl;
    break;

    case lbj::Info::PlayerSplitIds:
        std::cout << "PlayerSplitIds" << std::endl;
    break;

    case lbj::Info::PlayerDoubleInvalid:
        std::cout << "PlayerDoubleInvalid" << std::endl;
    break;

    case lbj::Info::PlayerNextHand:
        std::cout << "PlayerNextHand" << std::endl;
    break;
    
    case lbj::Info::PlayerPushes:
        std::cout << "PlayerPushes" << std::endl;
    break;
    
    case lbj::Info::PlayerLosses:
        std::cout << "PlayerLosses" << std::endl;
    break;
    case lbj::Info::PlayerBlackjack:
        std::cout << "PlayerBlackjack" << std::endl;
    break;
    case lbj::Info::PlayerWins:
        std::cout << "PlayerWins" << std::endl;
    break;
    
    case lbj::Info::NoBlackjacks:
        std::cout << "NoBlackjacks" << std::endl;
    break;

    case lbj::Info::DealerBusts:
        std::cout << "DealerBusts" << std::endl;
    break;  
    
    case lbj::Info::Help:
        std::cout << "Help" << std::endl;
    break;

    case lbj::Info::Bankroll:
        std::cout << "Bankroll" << std::endl;
    break;
    
    case lbj::Info::CommandInvalid:
        std::cout << "CommandInvalid" << std::endl;
    break;
    
    case lbj::Info::Bye:
        std::cout << "Bye" << std::endl;
    break;

    case lbj::Info::None:
    break;

  }

  return;
}

}



