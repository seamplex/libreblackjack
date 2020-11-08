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

#ifndef BLACKJACK_H
#define BLACKJACK_H

#include "base.h"
#include "conf.h"


class Blackjack : public Dealer {
  public:  
    Blackjack(Configuration &);
    ~Blackjack();
    
    void shuffle() override;
    unsigned int drawCard(Hand * = nullptr) override;
    void deal(Player *) override;
    int process(Player *) override;
    
  private:
    
    unsigned int rng_seed;
    std::random_device dev_random;
    std::mt19937 rng;
    std::uniform_int_distribution<unsigned int> fiftyTwoCards;
    
    bool lastPass = false;
    
    unsigned int upCard;
    unsigned int holeCard;
    unsigned int playerFirstCard;
    unsigned int playerSecondCard;
    
    int n_decks = -1;
    unsigned long int n_hands = 0;
    unsigned long int n_hand = 0;
    
    bool hit_soft_17 = true;
    bool double_after_split = true;
    bool shuffle_every_hand = false;
    
    std::vector<unsigned int> arranged_cards;

    unsigned int max_bet = 0;
    unsigned int number_of_burnt_cards = 0;
    unsigned int infinite_decks_card_number_for_arranged_ones = 0;
    
    
    
    double insurance = 0;
    double blackjack_pays = 1.5;
    
    double penetration = 0.75;
    double penetration_sigma = 0;
    
      
};
#endif
