#include <iostream>
#include "base.h"

void Hand::render(bool holeCardShown) {
 
  for (auto it : cards) {
    std::cout << " _____   ";
  }
  std::cout << std::endl;
  
  unsigned int i = 0;
  for (auto it : cards) {
    if (holeCardShown || i != 1) {
      std::cout << "|" << card[it].getNumberASCII() << ((card[it].number != 10)?" ":"") << "   |  ";
    } else {
      std::cout << "|#####|  ";
    }
    i++;
  }
  std::cout << std::endl;

  i = 0;
  for (auto it : cards) {
    if (holeCardShown || i != 1) {
      std::cout << "|     |  ";
    } else {
      std::cout << "|#####|  ";
    }
    i++;
  }
  std::cout << std::endl;
  
  i = 0;
  for (auto it : cards) {
    if (holeCardShown || i != 1) {
      std::cout << "|  " << card[it].getSuitUTF8() << "  |  ";
    } else {
      std::cout << "|#####|  ";
    }
    i++;
  }
  std::cout << std::endl;
  
  i = 0;
  for (auto it : cards) {
    if (holeCardShown || i != 1) {
      std::cout << "|     |  ";
    } else {
      std::cout << "|#####|  ";
    }
    i++;
  }
  std::cout << std::endl;

  i = 0;
  for (auto it : cards) {
    if (holeCardShown || i != 1) {
      std::cout << "|___" << ((card[it].number != 10)?"_":"") << card[it].getNumberASCII() << "|  ";
    } else {
      std::cout << "|#####|  ";
    }
    i++;
  }
  std::cout << std::endl;
  
  return;
    
}
