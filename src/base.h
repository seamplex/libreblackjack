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
  DealDealerHoleCard,
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


#define CARD_ART_LINES 6
#define CARD_TYPES     5
#define CARD_SIZE      16

class Card {
  public:
    int tag;
    int value;
  
  private:
    std::string token[CARD_TYPES];
    std::string text;
    std::string art[CARD_ART_LINES];
    
};

class Hand {
  public:
    bool insured;
    bool soft;
    bool blackjack;
    bool busted;
    bool holeCardShown; // maybe we need a separate class for dealer and for player?
    int bet;
    int count;
    
    std::list<Card> cards;
  
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

    virtual void deal(Player *) = 0;
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
    std::list <Hand> hands;
    
};

#endif
