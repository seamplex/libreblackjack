/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - standard blackjack dealer
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

#ifndef BASE_H
#define BASE_H

#include <string>
#include <list>

// TODO: namespace

enum class DealerAction {
  None,
  StartNewHand,
  AskForBets,
  DealPlayerFirstCard,
  AskForInsurance,
  CheckforBlackjacks,
  PayOrTakeInsuranceBets,
  AskForPlay,
  MoveOnToNextHand,
  HitDealerHand,
  Payout
};

enum class PlayerActionRequired {
  None,
  Bet,
  Insurance,
  Play
};

enum class PlayerActionTaken {
  None,
// common  
  Quit,
  Help,
  Count,
  UpcardValue,
  Bankroll,
  Hands,
  Table,
// particular  
  Bet,
  Insure,
  DontInsure,
  Stand,
  Double,
  Split,
  Hit,
};

// alphabetically-sorted
enum class Suit {
  Clubs    = 0,
  Diamonds = 1,
  Hearts   = 2,
  Spades   = 3
};


class Card {
  public:
    Card(unsigned int);
    ~Card() { };
    // TODO: delete copy & move
    
    // TODO: decidir si conviene usar getters o public members
/*    
    Suit getSuit()           { return suit; };
    unsigned int getNumber() { return number; };
    unsigned int getValue()  { return value; };
 */
//    std::string get
    
    Suit suit;
    unsigned int number;
    unsigned int value;
    
    std::string ascii() {
      return valueASCII + suitASCII;
    }
    std::string utf8() {
      return valueASCII + suitUTF8;
    }
    std::string text();
    
  private:
    std::string valueASCII;
    std::string suitASCII;
    std::string suitUTF8;
    std::string suitName;
};


extern Card card[52];


// TODO: base + daugthers, para diferenciar entre dealer y player y otros juegos
class Hand {
  public:
    bool insured = false;
    bool holeCardShown = false;
    int bet = 0;
    std::list<unsigned int> cards;
    
    int total() {
      unsigned int soft = 0;
      unsigned int n = 0;
      unsigned int value = 0;
      for (auto tag : cards) {
        value = card[tag].value;
        n += value;
        soft += (value == 11);
      }
     
      while (n > 21 && soft > 0){
	      n -= 10;
	      soft--;
      }
      
      return (soft)?(-n):(n);
    };
    
    bool blackjack() {
      return (total() == 21 && cards.size() == 2);
    };
    
    bool busted() {
      return (total() > 21);
    }

  private:
  
};

class dealerHand {
  public:
    std::list<unsigned int> cards;
    
    unsigned int total() {
      return 0;
    }
  
  private:
  
};


class Player {
  public:
    Player() = default;
    ~Player() = default;
    // delete copy and move constructors
    Player(Player&) = delete;
    Player(const Player&) = delete;
    Player(Player &&) = delete;
    Player(const Player &&) = delete;

/*    
    PlayerAction getNextAction() {
      return nextAction;
    }
    
    void setNextAction(PlayerAction a) {
      nextAction = a;
    }
*/    
    virtual int play() = 0;
    
    PlayerActionRequired actionRequired = PlayerActionRequired::None;
    PlayerActionTaken    actionTaken    = PlayerActionTaken::None;
    
    bool hasSplit = false;
    bool hasDoubled = false;

    int flatBet = 0;
    int currentBet = 0;
    int n_hands = 0;  // this is different from the dealer's due to splitting
    int n_insured_hands = 0;

    bool no_insurance = false;
    bool always_insure = false;
  
    double bankroll = 0;
    double total_money_waged = 0;
    double current_result = 0;
    double mean = 0;
    double M2 = 0;
    double variance = 0;
    
    std::list<Hand> hands;
    std::list<Hand>::iterator currentHand;
};

class Dealer {
  public:
    Dealer() {};
    ~Dealer() {};
    // delete copy and move constructors
    Dealer(Dealer&) = delete;
    Dealer(const Dealer&) = delete;
    Dealer(Dealer &&) = delete;
    Dealer(const Dealer &&) = delete;

    // maybe this first one does not need to be deleted
    virtual void shuffle() = 0;
    virtual void deal(Player *) = 0;
    virtual int dealCard(Hand * = nullptr) = 0;
    virtual int process(Player *) = 0;
    
   
    void setNextAction(DealerAction a) {
      next_action = a;
    }

    void getNextAction(DealerAction a) {
      next_action = a;
    }
    
    bool getInputNeeded(void) {
      return input_needed;
    }
    
    void setInputNeeded(bool flag) {
      input_needed = flag;
    }

    bool finished(void) {
      return done;
    }
    
    bool finished(bool d) {
      return (done = d);
    }
    
    bool done = false;
    bool input_needed = false;
    DealerAction next_action = DealerAction::None;
    
    // TODO: most of the games will have a single element, but maybe
    // there are games where the dealer has more than one hand
//    std::list <Hand> hands;
    Hand hand;
    
    
};

#endif
