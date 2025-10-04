/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - internal fully-informed automatic player
 *
 *  Copyright (C) 2020,2023 jeremy theler
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

#ifndef INFORMED_H
#define INFORMED_H
#include "../blackjack.h"


// all arrays are fixed in size so the compiler can do its magic
#define SIZE 1+32

namespace lbj {

class Informed : public Player {
  public:  
    Informed(Configuration &);
    ~Informed() { };

    int play(void) override;
    void info(lbj::Info = lbj::Info::None, int = 0, int = 0) override;

  private:

    // number of decks (0 = infinite)
    int decks = 0;
    bool enhc = false;

    // ditto
    int remaining_cards;

    // how many remaining cards of each rank are left
    // index = 0  -> invalid
    // index = 1  -> ace
    // index = 2  -> deuce
    // ...
    // index = 10 -> faces
    // index = 11 -> again ace
    int remaining[12];

    // dealer's probability of getting a total equal to the first index starting from a total equal to the second
    double dealer_hard[SIZE][SIZE];   // european hard hand
    double dealer_soft[SIZE][SIZE];   // european soft hand
    double dealer_american[SIZE][SIZE];   // american hard hand

    // expected values of the player having a total equal to the first index
    // for a given dealer's upcard
    double hard_stand[SIZE];
    double soft_stand[SIZE];
    double hard_hit[SIZE];
    double soft_hit[SIZE];
    double hard_double[SIZE];
    double soft_double[SIZE];
    double split[SIZE];

    void init(void);
    void reset(void);
    void dealer_bust_european_iteration(void);
    void dealer_european_to_american(void);
    void stand(int upcard);
    void hit_iteration();
    void double_down();
    void pairs();
};
}
#endif
