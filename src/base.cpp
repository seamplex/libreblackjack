#include <iostream>
#include "base.h"

void Hand::draw(void) {
 
  for (auto it : cards) {
    std::cout << card[it].utf8() << std::endl;
  }
  
  return;
    
}
