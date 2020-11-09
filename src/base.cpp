/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack -  base classes
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
