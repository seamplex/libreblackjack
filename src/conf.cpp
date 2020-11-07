#include "conf.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>

// source https://www.walletfox.com/course/parseconfigfile.php

Configuration::Configuration(int argc, char **argv) {
  
  int i, optc;
  int option_index = 0;
  
  // TODO: if blackjack.conf exists parse it
  parseConfigFile();
    
}

int Configuration::parseConfigFile(std::string filePath) {
  
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
        data[name] = value;
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
