/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - fifty-two-deck cards
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

#include "dealer.h"

namespace lbj {

static std::string TJQK[4] = {"T", "J", "Q", "K"};
static std::string numbers[14] = {"", "ace", "deuce", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "jack", "queen", "king"};

Card::Card(unsigned int tag) {
  
  if (tag <= 0 || tag > 52) {
    return;
  }  
    
  number = 1 + ((tag-1) % 13);
  suit = static_cast<lbj::Suit>((tag-1)/13);
  value = (number == 1) ? 11 : ((number > 10) ? 10 : number);
  numberASCII = (number == 1) ? "A" : ((number < 10) ? std::to_string(number) : TJQK[number-10]);
      
  switch (suit){
    case lbj::Suit::Clubs:
      suitName = "clubs";
      suitASCII = "C";
      suitUTF8 = "♣";
    break;
    case lbj::Suit::Diamonds:
      suitName = "diamonds";
      suitASCII = "D";
      suitUTF8 = "♦";
    break;
    case lbj::Suit::Hearts:
      suitName = "hearts";
      suitASCII = "H";
      suitUTF8 = "♥";
    break;
    case lbj::Suit::Spades:
      suitName = "spades";
      suitASCII = "S";
      suitUTF8 = "♠";
    break;
  }
  
  
  // https://en.wikipedia.org/wiki/Playing_cards_in_Unicode
  // yes, I could loop and compute the code for each card and
  // it would be hacky but hard to understand for humans
  // UNIX rule of representation!
  switch(tag) {
    case 1: singleUTF8 = "🃑"; break;
    case 2: singleUTF8 = "🃒"; break;
    case 3: singleUTF8 = "🃓"; break;
    case 4: singleUTF8 = "🃔"; break;
    case 5: singleUTF8 = "🃕"; break;
    case 6: singleUTF8 = "🃖"; break;
    case 7: singleUTF8 = "🃗"; break;
    case 8: singleUTF8 = "🃘"; break;
    case 9: singleUTF8 = "🃙"; break;
    case 10: singleUTF8 = "🃚"; break;
    case 11: singleUTF8 = "🃛"; break;
    case 12: singleUTF8 = "🃝"; break;
    case 13: singleUTF8 = "🃞"; break;

    case 14: singleUTF8 = "🃁"; break;
    case 15: singleUTF8 = "🃂"; break;
    case 16: singleUTF8 = "🃃"; break;
    case 17: singleUTF8 = "🃄"; break;
    case 18: singleUTF8 = "🃅"; break;
    case 19: singleUTF8 = "🃆"; break;
    case 20: singleUTF8 = "🃇"; break;
    case 21: singleUTF8 = "🃈"; break;
    case 22: singleUTF8 = "🃉"; break;
    case 23: singleUTF8 = "🃊"; break;
    case 24: singleUTF8 = "🃋"; break;
    case 25: singleUTF8 = "🃍"; break;
    case 26: singleUTF8 = "🃍"; break;
    
    case 27: singleUTF8 = "🂱"; break;
    case 28: singleUTF8 = "🂲"; break;
    case 29: singleUTF8 = "🂳"; break;
    case 30: singleUTF8 = "🂴"; break;
    case 31: singleUTF8 = "🂵"; break;
    case 32: singleUTF8 = "🂶"; break;
    case 33: singleUTF8 = "🂷"; break;
    case 34: singleUTF8 = "🂸"; break;
    case 35: singleUTF8 = "🂹"; break;
    case 36: singleUTF8 = "🂺"; break;
    case 37: singleUTF8 = "🂻"; break;
    case 38: singleUTF8 = "🂽"; break;
    case 39: singleUTF8 = "🂾"; break;
    
    case 40: singleUTF8 = "🂡"; break;
    case 41: singleUTF8 = "🂢"; break;
    case 42: singleUTF8 = "🂣"; break;
    case 43: singleUTF8 = "🂤"; break;
    case 44: singleUTF8 = "🂥"; break;
    case 45: singleUTF8 = "🂦"; break;
    case 46: singleUTF8 = "🂧"; break;
    case 47: singleUTF8 = "🂨"; break;
    case 48: singleUTF8 = "🂩"; break;
    case 49: singleUTF8 = "🂪"; break;
    case 50: singleUTF8 = "🂫"; break;
    case 51: singleUTF8 = "🂭"; break;
    case 52: singleUTF8 = "🂮"; break;
  }    
};

std::string Card::text() {
  return numbers[number] + " of " + suitName;
}

Card card[53] = { 0,
                  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13,
                 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
                 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
                 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52};
}