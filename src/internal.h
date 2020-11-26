/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - internal automatic player
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

#ifndef INTERNAL_H
#define INTERNAL_H
#include "blackjack.h"

class Internal : public Player {
  public:  
    Internal(Configuration &);
    ~Internal() { };
    
    int play(void) override;
    void info(Libreblackjack::Info = Libreblackjack::Info::None, int = 0, int = 0) override;

  private:
    
    std::string strategy_file_path{"bs.txt"};
    std::vector<std::vector<Libreblackjack::PlayerActionTaken>> pair;
    std::vector<std::vector<Libreblackjack::PlayerActionTaken>> soft;
    std::vector<std::vector<Libreblackjack::PlayerActionTaken>> hard;
      
};

#endif
