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
class Blackjack : public Dealer {
  public:  
    Blackjack();
    ~Blackjack();
    
    void deal(Player *) override;
    int process(Player *) override;
    
  private:
    bool lastPass = false;
    
    int n_hands = 0;
    int n_hand = 0;
    
    int max_bet = 0;
    int number_of_burnt_cards = 0;
    int infinite_decks_card_number_for_arranged_ones = 0;
    
    double insurance = 0;
      
};
#endif
