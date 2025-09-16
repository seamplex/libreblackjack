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

namespace lbj {

class Blackjack : public Dealer {
  public:  
    Blackjack(Configuration &);
    ~Blackjack();
    
    void shuffle() override;
    unsigned int draw(Hand * = nullptr) override;
    void deal(void) override;
    int process(void) override;
    
  private:
    
    unsigned int rng_seed;
    std::random_device dev_random;
    std::mt19937 rng;
    std::uniform_int_distribution<unsigned int> fiftyTwoCards;
    
    std::vector<unsigned int> shoe;
    size_t pos = 0;
    size_t cut_card_position = 0;
    bool last_pass = false;
    
    unsigned int upCard;
    unsigned int holeCard;
    unsigned int playerFirstCard;
    unsigned int playerSecondCard;

    // TODO: check these!
    bool h17 = true;   // checked
    bool das = true;   // checked
    bool doa = true;   // checked
    bool enhc = false; // checked
    bool rsa = false;
    bool shuffle_every_hand = false;
    
    std::vector<unsigned int> arranged_cards;
    unsigned int n_arranged_cards = 0; // just to prevent calling size() each time we draw a card
    unsigned int i_arranged_cards = 0;

    unsigned int max_bet = 0;
    unsigned int number_of_burnt_cards = 0;
    
    
    
    double insurance = 0;
    double blackjack_pays = 1.5;
    
    double penetration = 0.75;
    double penetration_sigma = 0;
    
    void can_double_split(void);
      
};
};
#endif
