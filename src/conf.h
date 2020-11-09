/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - configuration handling
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

#include <string>
#include <list>
#include <map>

class Configuration {
  public:  
    Configuration(int, char **);  
    ~Configuration();

    int readConfigFile(std::string, bool = false);
    
    bool exists(std::string key) { return (data.count(key) != 0); }

    bool set(bool *, std::list<std::string>);
    bool set(int *, std::list<std::string>);
    bool set(unsigned int *, std::list<std::string>);
    bool set(long unsigned int *, std::list<std::string>);
    bool set(double *, std::list<std::string>);
    bool set(std::string &, std::list<std::string>);
    

    void show(void);
    bool getBool(std::string);
    int getInt(std::string);
    std::string getString(std::string);
    
    std::string getDealerName(void) { return dealer; };
    std::string getPlayerName(void) { return player; };

    unsigned int max_incorrect_commands = 10;
    std::string yaml_report_path;

    unsigned int hands_per_char = false;
    
    double error_standard_deviations;
    
  private:
    std::map<std::string, std::string> data;
    std::string configFilePath = "./blackjack.conf";
    bool explicitConfigFile = false;
    
    std::string dealer;
    std::string player;
    
    bool show_help = false;
    bool show_version = false;
//    bool show_bar = false;
//    bool bar_already_alloced = false;
    
};
#endif
