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

enum class PlayerAction {
  None,
  Bet,
  Insurance,
  Play
};

enum class Command {
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
  Yes,
  No,
  Stand,
  Double,
  Split,
  Hit,
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

    virtual void deal() = 0;
//    virtual void ask() = 0;
    virtual int process(Command) = 0;
    
   
    void setNextAction(DealerAction a) {
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
    
  private:
    
    bool done = false;
    bool input_needed = false;
    DealerAction next_action = DealerAction::None;
    PlayerAction player_action = PlayerAction::None;
    
    Command player_command = Command::None;
    
    double insurance = 0;
    int bet = 0;
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
    
    virtual int play(Command *, int *) = 0;
};

#endif