/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - stdio player
 *
 *  Copyright (C) 2020, 2023 jeremy theler
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
#include "../blackjack.h"

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

namespace lbj {

class StdInOut : public Player {
  public:  
    StdInOut(Configuration &);
    ~StdInOut() { };
    
    int play(void) override;
    void info(lbj::Info = lbj::Info::None, int = 0, int = 0) override;
    
  private:
    std::string input_buffer;
    
    std::size_t hand_to_split;
    unsigned int card_to_split;
/*    
    std::list<PlayerHand> hands;
    std::list<PlayerHand>::iterator currentHand;
    std::size_t currentHandId = 0;
    Hand dealerHand;
*/
    
    inline void ltrim(std::string &s) {
      s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    };

    inline void rtrim(std::string &s) {
      s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
    };

    inline void trim(std::string &s) { ltrim(s); rtrim(s); };    
      
};
}
#endif
