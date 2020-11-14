/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - tty interactive player
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

#ifndef TTY_H
#define TTY_H
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

#include "blackjack.h"

extern std::vector<std::string> commands;

class Tty : public Player {
  public:  
    Tty(Configuration &);
    ~Tty() { };
    
    int play() override;
    void info(Info = Info::None, int = 0, int = 0) override;

    // for readline's autocompletion
    static char *rl_command_generator(const char *, int);
    static char **rl_completion(const char *, int, int);
    static int list_index;
    static int len;  
    
  private:

    void renderHand(Hand *);
    void renderTable(void);
      
#ifdef HAVE_LIBREADLINE
  char *input_buffer;
#else
  std::string input_buffer;
#endif

    std::string arrow;
    std::string prompt;
    
    int delay = 200;
    
    bool color = true;
    std::string black;
    std::string red;
    std::string green;
    std::string yellow;
    std::string blue;
    std::string magenta;
    std::string cyan;
    std::string white;
    std::string reset;
    

    inline void ltrim(std::string &s) {
      s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    };

    inline void rtrim(std::string &s) {
      s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
    };

    inline void trim(std::string &s) { ltrim(s); rtrim(s); };    
      
};

#endif
