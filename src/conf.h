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

#ifndef CONF_H
#define CONF_H

#include <map>

class Configuration {
  public:  
    Configuration(int, char **);  
    ~Configuration();

    int readConfigFile(std::string);
    
    bool exists(std::string key) { return !(data.find(key) == data.end()); }
    

    void show(void);
    bool getBool(std::string);
    int getInt(std::string);
    std::string getString(std::string);
    
  private:
    std::map<std::string, std::string> data;
    bool show_help = false;
    bool show_version = false;
    bool show_bar = false;
    bool bar_already_alloced = false;
    unsigned int hands_per_char = false;
    
};
#endif
