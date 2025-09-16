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
  
  const struct option longopts[] = {
///op+conf+option `-c`path  or `--conf=`path
///op+conf+desc Specify the path to the [configuration file]. Default is `./blackjack.conf`
    {"conf", required_argument, NULL, 'c'},

    ///op+hands+option `-n`$n$  or `--hands=`$n$
///op+hands+desc Specify the number of hands to play. Corresponds to the `hands` variable in the [configuration file].
    {"hands", required_argument, NULL, 'n'},
    
///op+decks+option `-d`$n$ or `--decks=`$n$
///op+decks+desc Specify the number of decks to use in the shoe. Corresponds to the `decks` variable in the [configuration file].
    {"decks", required_argument, NULL, 'd'},
    
///op+flatbet+option `-f` or `--flatbet`
///op+flatbet+desc Do not ask for the amount to bet before starting a new hand and use a flat unit bet. Corresponds to the `flat_bet` variable in the [configuration file].
    {"flatbet", optional_argument, NULL, 'f'},

///op+general+option `--`configuration_variable`[=`*value*`]`
///op+general+desc Any configuration variable from the [configuration file] can be set from the command line.
///op+general+desc For example, passing `--no_insurance` is like setting `no_insurance = 1` in the configuration file.
///op+general+desc Command-line options override setting in the configuration file.
    
///op+internal+option `-i` or `--internal`
///op+internal+desc Use the internal player to play against itself. See [internal player] for details.
    {"internal", no_argument, NULL, 'i'},
    
///op+help+option `-h` or `--help`
///op+help+desc Print this informative help message on standard output and exit successfully.
    {"help", no_argument, NULL, 'h'},
    
///op+version+option `-v` or `--version`
///op+version+desc Print the version number and licensing information of Hello on standard output and then exit successfully.
    {"version", no_argument, NULL, 'v'},
    {NULL, 0, NULL, 0}
  };

  int optc = 0;
  int option_index = 0;
  opterr = 0;
  while ((optc = getopt_long_only(argc, argv, "c:hvd:n:if", longopts, &option_index)) != -1) {
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
      case '?':
        {  
          std::string line(argv[optind - 1]);
          std::size_t delimiterPos = line.find("=");
          if (delimiterPos != std::string::npos) {
            std::size_t offset = 0;  
            if (line.substr(0, 2) == "--") {
              offset = 2;
            } else if (line.substr(0, 1) == "-") {
              offset = 1;
            }
            auto name = line.substr(offset, delimiterPos-offset);
            auto value = line.substr(delimiterPos + 1);
            conf[name] = value;
          } else {
            conf[line] = "true";
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
  
  
  if (set(dealer, {"dealer", "game"}) == false) {
    // we play old-school blackjack by default      
    dealer = "blackjack";
  }
  
  if (set(player, {"player"}) == false) {
    // if we are on an interactive terminal we play through tty otherwise stdinout
    if (isatty(fileno(stdin)) && isatty(fileno(stdout)))  {
      player = "tty";
    } else {
      player = "stdio";
    }
  }
  
  // common settings to all dealers and players
  set(&max_incorrect_commands, {"max_incorrect_commands"});
  
  return;
    
}

// source https://www.walletfox.com/course/parseconfigfile.php

int Configuration::readConfigFile(std::string file_path, bool mandatory) {
  
  // std::ifstream is RAII, i.e. no need to call close
  std::ifstream fileStream(file_path);
  
  if (fileStream.is_open()) {
    int line_num = 0; 
    std::string line;
    while(getline(fileStream, line)) {
      line_num++;
      line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
      if (line[0] == '#' || line[0] == ';' || line.empty()) {
        continue;
      }
                
      // TODO: comments on the same line
      std::size_t delimiter_pos = line.find("=");
      if (delimiter_pos != std::string::npos) {
        std::string name = line.substr(0, delimiter_pos);
        std::string value = line.substr(delimiter_pos + 1);
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
//      std::cout << "set bool " << it << "=" << *value << std::endl;
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
//      std::cout << "set int " << it << "=" << *value << std::endl;
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
//      std::cout << "set uint " << it << "=" << *value << std::endl;
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
//      std::cout << "set size_t " << it << "=" << *value << std::endl;
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
//      std::cout << "set string " << it << "=" << value << std::endl;
      
      return true;
    }
  }
  return false;
}

bool Configuration::set(double *value, std::list<std::string> key) {
  for (auto it : key) {
    if (exists(*(&it))) {
      *value = std::stof(conf[*(&it)]);
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