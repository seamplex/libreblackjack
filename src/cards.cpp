#include "base.h"

static std::string TJQK[4] = {"T", "J", "Q", "K"};
static std::string numbers[14] = {"", "ace", "deuce", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "jack", "queen", "king"};

Card::Card(unsigned int tag) {
  number = 1 + (tag % 13);
  suit = static_cast<Suit>(tag/13);
  value = (number == 1) ? 11 : ((number > 10) ? 10 : number);
  valueASCII = (number < 11) ? std::to_string(number) : TJQK[number-10];
      
  switch (suit){
    case Suit::Clubs:
      suitName = "clubs";
      suitASCII = "C";
      suitUTF8 = "♣";
    break;
    case Suit::Diamonds:
      suitName = "diamonds";
      suitASCII = "D";
      suitUTF8 = "♦";
    break;
    case Suit::Hearts:
      suitName = "hearts";
      suitASCII = "H";
      suitUTF8 = "♥";
    break;
    case Suit::Spades:
      suitName = "spades";
      suitASCII = "S";
      suitUTF8 = "♠";
    break;
  }
};

std::string Card::text() {
  return numbers[number] + " of " + suitName;
}

Card card[52] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
                 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
                 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
