#include "conf.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>
#include <getopt.h>

// source https://www.walletfox.com/course/parseconfigfile.php

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
///op+general+desc For example, passing `--no_insurance` is like setting `no_insurance = 1` in the configuration file. Command-line options override configuration options.
    
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

  int i, optc;
  int option_index = 0;
  int opterr = 0;
  while ((optc = getopt_long_only(argc, argv, "c:hvd:n:if:", longopts, &option_index)) != -1) {
    switch (optc) {
      case 'h':
        show_help = true;
      break;
      case 'v':
        show_version = true;
      break;
      case 'c':
        std::cout << "custom conf " << optarg << std::endl; 
        configFilePath = std::move(std::string(optarg));
        explicitConfigFile = true;
      break;
      case 'd':
        data["decks"] = std::move(std::string(optarg));
      break;
      case 'n':
        data["hands"] = std::move(std::string(optarg));
      break;
      case 'i':
        data["player"] = "internal";
      break;
      case 'f':
        if (optarg != NULL) {
          data["flat_bet"] = optarg;
        } else {
          data["flat_bet"] = "1";
        }
      break;
      case '?':
        {  
          std::string line(argv[optind - 1]);
          std::size_t delimiterPos = line.find("=");
          if (delimiterPos != std::string::npos) {
            auto name = line.substr(0, delimiterPos);
            auto value = line.substr(delimiterPos + 1);
            data[name] = value;
          } else {
            data[line] = "true";
          }
        }  
      break;
      default:
      break;
    }
  }
  
  
  std::ifstream default_file(configFilePath);
  if (default_file.good()) {
    readConfigFile(configFilePath, explicitConfigFile);
  }
    
}

int Configuration::readConfigFile(std::string filePath, bool mandatory) {
  
  // std::ifstream is RAII, i.e. no need to call close
  std::ifstream fileStream(filePath);
    
  std::size_t delimiterPos;
  std::string name;
  std::string value;
  
  if (fileStream.is_open()) {
    std::string line;
    while(getline(fileStream, line)) {
      line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
      if (line[0] == '#' || line[0] == ';' || line.empty()) {
        continue;
      }
                
      // TODO: comments on the same line
      delimiterPos = line.find("=");
      if (delimiterPos != std::string::npos) {
        name = line.substr(0, delimiterPos);
        value = line.substr(delimiterPos + 1);
        if (!exists(name)) {
          data[name] = value;
        }  
        // TODO: add another map of string to bools to mark wheter the option was used or not
      } else {
        // TODO: warning?
      }
    }
  } else {
    return -1;
  }
  
  return 0;
  
}

bool Configuration::set(bool *value, std::list<std::string> key) {
  for (auto it : key) {
    if (exists(*(&it))) {
      if (data[*(&it)] == "true") {
        *value = true;  
      } else if (data[*(&it)] == "false") {
        *value = false; 
      } else {
        *value = std::stoi(data[*(&it)]);
      }
      return true;
    }
  }
  return false;
}

bool Configuration::set(int *value, std::list<std::string> key) {
  for (auto it : key) {
    if (exists(*(&it))) {
      *value = std::stoi(data[*(&it)]);
      return true;
    }
  }
  return false;
}

bool Configuration::set(unsigned int *value, std::list<std::string> key) {
  for (auto it : key) {
    if (exists(*(&it))) {
      *value = std::stoi(data[*(&it)]);
      return true;
    }
  }
  return false;
}

bool Configuration::set(unsigned long int *value, std::list<std::string> key) {
  for (auto it : key) {
    if (exists(*(&it))) {
      *value = std::stoi(data[*(&it)]);
      return true;
    }
  }
  return false;
}

bool Configuration::set(double *value, std::list<std::string> key) {
  for (auto it : key) {
    if (exists(*(&it))) {
      *value = std::stof(data[*(&it)]);
      return true;
    }
  }
  return false;
}

void Configuration::show(void) {
    
  for (auto &it : data) {
    std::cout << it.first << " = " << it.second << std::endl;
  }
    
}

int Configuration::getInt(std::string key) {
  auto it = data.find(key);
  return (it != data.end()) ? std::stoi(it->second) : 0;
}

Configuration::~Configuration() {
  data.clear();
}
