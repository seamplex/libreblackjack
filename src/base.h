/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - base classes
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
#include <random>

// TODO: namespace

enum class DealerAction {
  None,
  StartNewHand,
  AskForBets,
  DealPlayerFirstCard,
  AskForInsurance,
  CheckforBlackjacks,
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

enum class Info {
  None,
  NewHand,
  Shuffle,
  CardPlayerFirst,
  CardDealerUp,
  CardPlayerSecond,
  CardDealerHoleDealt,
  CardDealerHoleRevealed,
  DealerBlackjack,
  PlayerWinsInsurance,
  PlayerBlackjackAlso,
  PlayerPushes,
  PlayerLosses,
  PlayerBlackjack,
  PlayerWins,
  NoBlackjacks,
  Bye,
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
    Suit getSuit()           { return suit; };
    unsigned int getNumber() { return number; };
    unsigned int getValue()  { return value; };

    std::string getNumberASCII() { return numberASCII; };
    std::string getSuitUTF8()    { return suitUTF8;    };
    
    Suit suit;
    unsigned int number;
    unsigned int value;
    
    std::string ascii() {
      return numberASCII + suitASCII;
    }
    std::string utf8(bool single = false) {
      return single ? singleUTF8 : numberASCII + suitUTF8;
    }
    std::string text();
    
  private:
    std::string numberASCII;
    std::string suitASCII;
    std::string suitUTF8;
    std::string suitName;
    std::string singleUTF8;
};

// TODO: static inside a class
extern Card card[52];

class Hand {
  public:
    std::list<int> cards;

    // inline on purpose
    int total() {
      unsigned int soft = 0;
      unsigned int n = 0;
      unsigned int value = 0;
      for (auto tag : cards) {
        value = card[tag].value;
        n += value;
        soft += (value == 11);
      }
     
      // this loop should be only executed once if everything works fine
      while (n > 21 && soft > 0){
        n -= 10;
        soft--;
      }
      
      return (soft)?(-n):(n);
    };
    
    // inline on purpose
    bool blackjack() {
      return (abs(total()) == 21 && cards.size() == 2);
    };
    
    // inline on purpose
    bool busted() {
      return (abs(total()) > 21);
    }
};    
    
class PlayerHand : public Hand {
  public:
    int bet = 0;
    int id = 0;
    bool insured = false;
    bool doubled = false;
};

class DealerHand : public Hand {
  public:
    bool holeCardShown = false;
};


class Player {
  public:
    Player() = default;
    virtual ~Player() = default;
    // delete copy and move constructors
    Player(Player&) = delete;
    Player(const Player&) = delete;
    Player(Player &&) = delete;
    Player(const Player &&) = delete;

    virtual int play() = 0;
    virtual void info(Info = Info::None, int = 0) = 0;
    
    PlayerActionRequired actionRequired = PlayerActionRequired::None;
    PlayerActionTaken    actionTaken    = PlayerActionTaken::None;
    
    bool bustedAllHands = false;

    unsigned int currentSplits = 0;
    
    unsigned int currentBet = 0;
    unsigned int n_hands = 0;  // this is different from the dealer's due to splitting

    unsigned int handsInsured = 0;
    unsigned int handsDoubled = 0;
    unsigned int blackjacksPlayer = 0;
    unsigned int blackjacksDealer = 0;

    unsigned int bustsPlayer = 0;
    unsigned int bustsDealer = 0;
    
    unsigned int wins = 0;
    unsigned int winsInsured = 0;
    unsigned int winsDoubled = 0;
    unsigned int winsBlackjack = 0;
    
    unsigned int pushes = 0;
    unsigned int losses = 0;
    // TODO: blackjack_pushes?
    
    unsigned int flat_bet = 1;
    bool no_insurance = false;
    bool always_insure = false;
  
    double bankroll = 0;
    double worst_bankroll = 0;
    double total_money_waged = 0;
    double current_result = 0;
    double mean = 0;
    double M2 = 0;
    double variance = 0;
    
    std::list<PlayerHand> hands;
    std::list<PlayerHand>::iterator currentHand;

    DealerHand dealerHand;
    
};

class Dealer {
  public:
    Dealer() = default;
    virtual ~Dealer() = default;
    // delete copy and move constructors
    Dealer(Dealer&) = delete;
    Dealer(const Dealer&) = delete;
    Dealer(Dealer &&) = delete;
    Dealer(const Dealer &&) = delete;

    // maybe this first one does not need to be deleted
    virtual void shuffle() = 0;
    virtual void deal(Player *) = 0;
    virtual unsigned int drawCard(Hand * = nullptr) = 0;
    virtual int process(Player *) = 0;
    
/*    
    void setNextAction(DealerAction a) {
      next_action = a;
    }

    void getNextAction(DealerAction a) {
      next_action = a;
    }
*/    
    bool finished(void) {
      return done;
    }
    
    bool finished(bool d) {
      return (done = d);
    }
    
    bool done = false;
    DealerAction nextAction = DealerAction::None;
    
    // TODO: most of the games will have a single element, but maybe
    // there are games where the dealer has more than one hand
//    std::list <Hand> hands;
    DealerHand hand;
    
};

#endif
