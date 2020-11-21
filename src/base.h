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
#include <unordered_map>
#include <random>
#include <cmath>

namespace Libreblackjack {
  void shortversion(void);
  void help(const char *);
  void copyright(void);

  enum class DealerAction {
    None,
    StartNewHand,
    DealPlayerFirstCard,
    CheckforBlackjacks,
    AskForPlay,
    MoveOnToNextHand,
    HitDealerHand,
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
    BetInvalid,
    NewHand,
    Shuffle,
    CardPlayer,
    CardDealer,
    CardDealerRevealsHole,
    DealerBlackjack,
    PlayerWinsInsurance,
    PlayerBlackjackAlso,
    PlayerSplitInvalid,
    PlayerSplitOk,
    PlayerSplitIds,
    PlayerDoubleInvalid,
    PlayerNextHand,
    PlayerPushes,
    PlayerLosses,
    PlayerBlackjack,
    PlayerWins,
    NoBlackjacks,
    DealerBusts,
    Bankroll,
    Help,
    CommandInvalid,
    Bye,
  };
  
  // alphabetically-sorted
  enum class Suit {
    Clubs    = 0,
    Diamonds = 1,
    Hearts   = 2,
    Spades   = 3
  };
  
  enum class Color {
    Black,
    Red
  };
};

class Card {
  public:
    Card(unsigned int);
    ~Card() { };
    // TODO: delete copy & move
    
    // TODO: decidir si conviene usar getters o public members
    Libreblackjack::Suit getSuit()           { return suit; };
    unsigned int getNumber() { return number; };
    unsigned int getValue()  { return value; };

    std::string getNumberASCII() { return numberASCII; };
    std::string getSuitUTF8()    { return suitUTF8;    };
    
    Libreblackjack::Suit suit;
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

// TODO: class static? which class?
extern Card card[53];

class Hand {
  public:
    std::list<unsigned int> cards;

    // inline on purpose
    int value() {
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
      return (std::abs(value()) == 21 && cards.size() == 2);
    };
    
    // inline on purpose
    bool busted() {
      return (std::abs(value()) > 21);
    }
};    
    
class PlayerHand : public Hand {
  public:
    PlayerHand(std::size_t i = 0) : id(i) { };
    std::size_t id;
    unsigned int bet = 0;
    bool insured = false;
    bool doubled = false;
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

    virtual int play(void) = 0;
    virtual void info(Libreblackjack::Info = Libreblackjack::Info::None, int = 0, int = 0) = 0;
    
    Libreblackjack::PlayerActionRequired actionRequired = Libreblackjack::PlayerActionRequired::None;
    Libreblackjack::PlayerActionTaken    actionTaken    = Libreblackjack::PlayerActionTaken::None;

    int dealerValue = 0;
    int playerValue = 0;
    
    bool verbose = false;
    bool flat_bet = false;
    bool no_insurance = false;
    bool always_insure = false;
    
    unsigned int currentBet;
    
  protected:
    std::list<PlayerHand> hands;
    std::list<PlayerHand>::iterator currentHand;
    std::size_t currentHandId = 0;

    Hand dealerHand;
};

struct reportItem {
  reportItem(std::string k, std::string f, double v) : key(k), format(f), value(v) {};
  std::string key;
  std::string format;
  double value;
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
    virtual void shuffle(void) = 0;
    virtual void deal(void) = 0;
    virtual unsigned int drawCard(Hand * = nullptr) = 0;
    virtual int process(void) = 0;
    
    void setPlayer(Player *p) {
      player = p;
    }
    
    void info(Libreblackjack::Info msg, int p1 = 0, int p2 = 0) {
      if (player->verbose) {
        player->info(msg, p1, p2);
      }
      return;
    }

    bool finished(void) {
      return done;
    }
    
    bool finished(bool d) {
      return (done = d);
    }
    
    Libreblackjack::DealerAction nextAction = Libreblackjack::DealerAction::None;
    
    void reportPrepare(void);
    int writeReportYAML(void);
    
    
  protected:
    // TODO: multiple players
    Player *player;

    // TODO: most of the games will have a single element, but maybe
    // there are games where the dealer has more than one hand
//    std::list <Hand> hands;
    Hand hand;

    double error_standard_deviations = 1.0;
    int n_decks = -1;
    unsigned long int n_hands = 0;
    unsigned long int n_hand = 0;
    
    struct {
      std::list<PlayerHand> hands;
      std::list<PlayerHand>::iterator currentHand;
    
      unsigned int splits = 0;
        
    //  unsigned int currentBet = 0;
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
        
      
      double bankroll = 0;
      double worstBankroll = 0;
      double totalMoneyWaged = 0;
      double result = 0;
      double mean = 0;
      double M2 = 0;
      double variance = 0;
    } playerStats;

  private:
    bool done = false;
    std::list<reportItem> report;
    
};

#endif
