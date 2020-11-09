/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - stdio player
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

#ifndef STDINOUT_H
#define STDINOUT_H
#include "blackjack.h"

class StdInOut : public Player {
  public:  
    StdInOut();
    ~StdInOut() { };
    
    int play(void) override;
    void info(Info = Info::None, int = 0) override;
    
  private:
    std::string input_buffer;

    std::string black   = "\x1B[0m";
    std::string red     = "\x1B[31m";
    std::string green   = "\x1B[32m";
    std::string yellow  = "\x1B[33m";
    std::string blue    = "\x1B[34m";
    std::string magenta = "\x1B[35m";
    std::string cyan    = "\x1B[36m";
    std::string white   = "\x1B[37m";
    std::string reset   = "\033[0m";
    
      
};
#endif
