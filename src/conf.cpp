#include "conf.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>

// source https://www.walletfox.com/course/parseconfigfile.php

Configuration::Configuration(std::string filePath, bool mandatory) {
    
  // std::ifstream is RAII, i.e. no need to call close
  std::ifstream fileStream((filePath != "") ? filePath : "./blackjack.conf");
    
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
                
      delimiterPos = line.find("=");
      if (delimiterPos != std::string::npos) {
        name = line.substr(0, delimiterPos);
        value = line.substr(delimiterPos + 1);
        data[name] = value;
      }
    }
  } else {
    std::cerr << "Couldn't open config file for reading.\n";
  }
}

void Configuration::show(void) {
    
  for (auto &it : data) {
    std::cout << it.first << " = " << it.second << std::endl;
  }
    
}


Configuration::~Configuration() {
  data.clear();
}