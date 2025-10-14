/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - configuration handling
 *
 *  Copyright (C) 2020, 2025 jeremy theler
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

#include "conf.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>

#include <getopt.h>
#include <unistd.h>

namespace lbj {
Configuration::Configuration(int argc, char **argv) {

///help+usage+desc [-c path_to_conf_file] [options] 

///help+extra+desc If no configuration file is given, a file named `blackjack.conf`
///help+extra+desc in the current directory is used, provided it exists.
///help+extra+desc See the full documentation for the available options and the default values.
  
  const struct option longopts[] = {
///op+conf+option `-c<`*path*`>`  or `--conf=`*path*
///op+conf+desc Specify the path to the configuration file. Default is `./blackjack.conf`.
    {"conf", required_argument, NULL, 'c'},

///op+hands+option `-n<`$n$`>`  or `--hands=`$n$
///op+hands+desc Specify the number of hands to play. Corresponds to the `hands` variable in the configuration file.
    {"hands", required_argument, NULL, 'n'},
    
///op+decks+option `-d<`$n$`>` or `--decks=`$n$
///op+decks+desc Specify the number of decks to use in the shoe. Corresponds to the `decks` variable in the configuration file.
    {"decks", required_argument, NULL, 'd'},
    
///op+flatbet+option `-f` or `--flatbet`
///op+flatbet+desc Do not ask for the amount to bet before starting a new hand and use a flat unit bet.
///op+flatbet+desc Corresponds to the `flat_bet` variable in the configuration file.
    {"flatbet", optional_argument, NULL, 'f'},

///op+internal+option `-i` or `--internal`
///op+internal+desc Use the internal player to play against the dealer. See the manual for details
///op+internal+desc and instructions to define the rules and optionally, the playing strategy and/or arranged shoes.
    {"internal", no_argument, NULL, 'i'},

///op+progress+option `-p` or `--progress`
///op+progress+desc Show a progress bar when playing a fixed number of hands.
    {"progress", no_argument, NULL, 'p'},
    
///op+help+option `-h` or `--help`
///op+help+desc Print this informative help message into standard output and exit successfully.
    {"help", no_argument, NULL, 'h'},

///op+version+option `-v` or `--version`
///op+version+desc Print the version number and licensing information into standard output and then exit successfully.
    {"version", no_argument, NULL, 'v'},
    {NULL, 0, NULL, 0}
  };
///op+general+option `--`*configuration_variable*`[=`*value*`]`
///op+general+desc Any configuration variable from the configuration file can be set from the command line.
///op+general+desc For example, passing `--no_insurance` is like setting `no_insurance = 1` in the configuration file.
///op+general+desc Command-line options override setting in the configuration file.


  int optc = 0;
  int option_index = 0;
  opterr = 0;
  while ((optc = getopt_long_only(argc, argv, "c:hvd:n:ifp", longopts, &option_index)) != -1) {
    switch (optc) {
      case 'h':
        show_help = true;
      break;
      case 'v':
        show_version = true;
      break;
      case 'c':
        config_file_path = optarg;
        explicit_config_file = true;
      break;
      case 'd':
        conf["decks"] = optarg;
      break;
      case 'n':
        conf["hands"] = optarg;
      break;
      case 'i':
        conf["player"] = "internal";
      break;
      case 'f':
        conf["flat_bet"] = (optarg != NULL) ? optarg : "yes";
      break;
      case 'p':
        progress = 1;
      break;
      case '?':
        {
          std::string line(argv[optind - 1]);
          
          std::size_t offset = 0;  
          if (line.substr(0, 2) == "--") {
            offset = 2;
          } else if (line.substr(0, 1) == "-") {
            offset = 1;
          }
          
          std::size_t delimiterPos = line.find("=");
          if (delimiterPos != std::string::npos) {
            auto name = line.substr(offset, delimiterPos-offset);
            auto value = line.substr(delimiterPos + 1);
            conf[name] = value;
          } else {
            auto name = line.substr(offset);
            conf[name] = "true";
          }
        }
      break;
      default:
      break;
    }
  }

  std::ifstream default_file(config_file_path);
  if (default_file.good()) {
    if (readConfigFile(config_file_path, explicit_config_file) != 0) {
      exit(1);
    }
  }

///conf+dealer+usage `dealer = ` *game*
///conf+dealer+details Defines the game the dealer will deal.
///conf+dealer+details Currently, the only valid choice is `blackjack`.
///conf+dealer+default `blackjack`
///conf+dealer+example dealer = blackjack
  if (set(dealer, {"dealer", "game"}) == false) {
    // we play old-school blackjack by default      
    dealer = "blackjack";
  }

///conf+player+usage `player =  ` *player*
///conf+player+details Defines which player will be playing against the dealer.
///conf+player+details Currently, the only valid choices are
///conf+player+details @
///conf+player+details  * `tty`: the game starts in an interactive mode where the dealer's messages and dealt cards
///conf+player+details are printed in the terminal, and the user is asked to issue her commands through the keyboard.
///conf+player+details This player is usually used to test if the configuration settings (i.e. `enhc` or `cards_file`)
///conf+player+details work as expected, although it can be used to just play ASCII blackjack.
///conf+player+details  * `stdio`: the dealer writes messages into the standard output and reads
///conf+player+details back commands from the standard input. With proper redirection (and possibly FIFO devices), this
///conf+player+details option can be used to have an ad-hoc player to programatically play blackjack.
///conf+player+details See @sec-players for examples.
///conf+player+details  * `internal`: the dealer plays against an internal player already programmed in
///conf+player+details Libre Blackjack that bets flat, never takes insurance and follows the basic strategy. 
///conf+player+details The strategy can be changed by setting the configuration variable `strategy_file`.
///conf+player+details This player is chosen if `-i` is passed in the command line.
///conf+player+default If neither the standard input nor output of the executable `blackjack` is re-directed, the default is `tty`.
///conf+player+default If at least one of them is re-directed or piped, the default is `stdio`.
///conf+player+example player = tty
///conf+player+example player = stdio
///conf+player+example player = internal
  if (set(player, {"player"}) == false) {
    // if we are on an interactive terminal we play through tty otherwise stdinout
    if (isatty(fileno(stdin)) && isatty(fileno(stdout)))  {
      player = "tty";
    } else {
      player = "stdio";
    }
  }

  // common settings to all dealers and players
///conf+max_incorrect_commands+usage `max_incorrect_commands = ` $n$
///conf+max_incorrect_commands+details Tells the dealer how many consecutive incorrect or invalid commands to accept before quitting.
///conf+max_incorrect_commands+details A finite value of $n$ avoids infinite loops where the player sends commands
///conf+max_incorrect_commands+details that do not make sense (such as garbage) or that are not valid (such as doubling when not allowed).
///conf+max_incorrect_commands+default 10
///conf+max_incorrect_commands+example max_incorrect_commands = 20
  set(&max_incorrect_commands, {"max_incorrect_commands"});

  return;

}

std::string trim(const std::string& str) {
  size_t first = str.find_first_not_of(" \t\n\r\f\v");
  if (first == std::string::npos) {
    return ""; // String is all whitespace
  }

  size_t last = str.find_last_not_of(" \t\n\r\f\v");
  return str.substr(first, (last - first + 1));
}

std::string strip_inline_comment(const std::string& line) {
    size_t hash_pos = line.find('#');
    size_t semicolon_pos = line.find(';');
    size_t comment_pos = std::min(hash_pos, semicolon_pos);

    // If both # and ; are present, pick the first one
    if (hash_pos != std::string::npos && semicolon_pos != std::string::npos) {
        comment_pos = std::min(hash_pos, semicolon_pos);
    } else if (hash_pos != std::string::npos) {
        comment_pos = hash_pos;
    } else if (semicolon_pos != std::string::npos) {
        comment_pos = semicolon_pos;
    } else {
        return line;
    }

    return line.substr(0, comment_pos);
}

int Configuration::readConfigFile(std::string file_path, bool mandatory) {

  std::ifstream fileStream(file_path);

  if (fileStream.is_open()) {
    int line_num = 0; 
    std::string line;
    while(getline(fileStream, line)) {
      line_num++;
      line = trim(strip_inline_comment(line));    
      if (line[0] == '#' || line[0] == ';' || line.empty()) {
        continue;
      }

      std::size_t delimiter_pos = line.find("=");
      if (delimiter_pos != std::string::npos) {
        std::string name = trim(line.substr(0, delimiter_pos));
        std::string value = trim(line.substr(delimiter_pos + 1));
        if (!exists(name)) {
          conf[name] = value;
          used[name] = false;
        }
      } else {
        std::cerr << "error: cannot find '=' in " << file_path << ":" << line_num << std::endl;
        return -1;
      }
    }
  } else {
    return -1;
  }

  return 0;

}

bool Configuration::set(bool *value, std::list<std::string> key) {
  for (auto &it : key) {
    if (exists(it)) {
      auto s = conf[it];
      if (s == "true" || s == "yes" || s == "y" || s == "") {
        *value = true;  
      } else if (s == "false" || s == "no" || s == "n") {
        *value = false; 
      } else {
        *value = std::stoi(s);
      }
      used[it] = true;
      return true;
    }
  }
  return false;
}

bool Configuration::set(int *value, std::list<std::string> key) {
  for (auto &it : key) {
    if (exists(it)) {
      *value = std::stoi(conf[it]);
      used[it] = true;
      return true;
    }
  }
  return false;
}

bool Configuration::set(unsigned int *value, std::list<std::string> key) {
  // check for negative values
  for (auto &it : key) {
    if (exists(it)) {
      
      int tmp = std::stoi(conf[it]);
      if (tmp < 0) {
        std::cerr << "error: key " << it << " cannot be negative" << std::endl;
        exit(-1);
      }
      
      *value = std::stoi(conf[it]);
      used[it] = true;      
      return true;
    }
  }
  return false;
}

bool Configuration::set(size_t *value, std::list<std::string> key) {
  for (auto &it : key) {
    if (exists(it)) {
      *value = (size_t)std::stod(conf[it]);
      used[it] = true;      
      return true;
    }
  }
  return false;
}

bool Configuration::set(std::string &value, std::list<std::string> key) {
  for (auto &it : key) {
    if (exists(it)) {
      value = conf[it];
      used[it] = true;      
      return true;
    }
  }
  return false;
}

bool Configuration::set(double *value, std::list<std::string> key) {
  for (auto it : key) {
    if (exists(*(&it))) {
      *value = std::stof(conf[*(&it)]);
      used[it] = true;      
      return true;
    }
  }
  return false;
}

void Configuration::markUsed(std::string key) {
  used[key] = true;
  return;
}

int Configuration::checkUsed(void) {
    
  for (auto &it : used) {
    if (it.second == false) {
      std::cerr << "error: unknown setting " << it.first << std::endl;
      return 1;
    }
  }
  return 0;
}

void Configuration::show(void) {
    
  for (auto &it : conf) {
    std::cout << it.first << " = " << it.second << std::endl;
  }
    
}

int Configuration::getInt(std::string key) {
  auto it = conf.find(key);
  return (it != conf.end()) ? std::stoi(it->second) : 0;
}

std::string Configuration::getString(std::string key) {
  auto it = conf.find(key);
  return (it != conf.end()) ? it->second : "";
}

Configuration::~Configuration() {
  conf.clear();
}
}
