/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - internal basic strategy automatic player
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

#ifndef INTERNAL_H
#define INTERNAL_H
#include "../blackjack.h"

class Basic : public Player {
  public:  
    Basic(Configuration &);
    ~Basic() { };
    
    int play(void) override;

  private:
    
    std::string strategy_file_path{"bs.txt"};
    Libreblackjack::PlayerActionTaken pair[21][12];
    Libreblackjack::PlayerActionTaken soft[21][12];
    Libreblackjack::PlayerActionTaken hard[21][12];
      
};

#endif
