/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - base classes
 *
 *  Copyright (C) 2020, 2023, 2025 jeremy theler
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

#include "conf.h"

namespace lbj {
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
    Rules,
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
    CardDealerUp,
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
    Rules,
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

class Card {
  public:
    Card(unsigned int);
    ~Card() { };
    
    Suit getSuit() { return suit; };
    unsigned int getNumber()       { return number; };
    unsigned int getValue()        { return value; };

    std::string getNumberASCII()   { return numberASCII; };
    std::string getSuitUTF8()      { return suitUTF8;    };
    
    Suit suit;
    unsigned int number;
    unsigned int value;
    
    std::string ascii()            { return numberASCII + suitASCII; };
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
    int value() const {
      unsigned int soft = 0;
      unsigned int n = 0;
      unsigned int value = 0;
      for (const auto &tag : cards) {
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
    bool blackjack() const {
      return (std::abs(value()) == 21 && cards.size() == 2);
    };

    // inline on purpose
    bool busted() const {
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
    Player(Configuration &conf);
    virtual ~Player() = default;
    // delete copy and move constructors
    Player(Player&) = delete;
    Player(const Player&) = delete;
    Player(Player &&) = delete;
    Player(const Player &&) = delete;

    virtual int play(void) = 0;
    virtual void info(lbj::Info = lbj::Info::None, int p1 = 0, int p2 = 0) { return; }
    
    lbj::PlayerActionRequired actionRequired = lbj::PlayerActionRequired::None;
    lbj::PlayerActionTaken    actionTaken    = lbj::PlayerActionTaken::None;

    // TODO: getters and setters
    std::string rules;
    
    bool verbose = false;
    bool flat_bet = false;
    bool no_insurance = false;
    bool always_insure = false;

    bool can_double = false;
    bool can_split = false;

    int value_dealer = 0;
    int value_player = 0;
    unsigned int current_bet = 0;
};

struct reportItem {
  reportItem(int l, std::string k, std::string s) : level(l), key(k), string(s) {};
  reportItem(int l, std::string k, double v) : level(l), key(k), value(v) {};
  int level;
  std::string key;
  double value;
  std::string string;
};

class Dealer {
  public:
    Dealer(Configuration &);
    ~Dealer();
    // delete copy and move constructors
    Dealer(Dealer&) = delete;
    Dealer(const Dealer&) = delete;
    Dealer(Dealer &&) = delete;
    Dealer(const Dealer &&) = delete;

    void shuffle();
    unsigned int count_shoe_cards();
    unsigned int draw(Hand * = nullptr);
    unsigned int draw_tag(void);
    void deal(void);
    int process(void);
    std::string rules(void);
    
    void setPlayer(Player *p) {
      player = p;
    }
    
    void info(lbj::Info msg, int p1 = 0, int p2 = 0) {
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
    
    void prepareReport(void);
    int writeReportYAML(void);
    
    lbj::DealerAction nextAction = lbj::DealerAction::None;

    // default one million hands
    size_t n_hands = 1000000;
    size_t n_hand = 0;
    
  private:
    // TODO: multiple players
    int n_players = 1;
    Player *player;

    // TODO: most of the games will have a single element, but maybe
    // there are games where the dealer has more than one hand
//    std::list <Hand> hands;
    Hand hand;

    // how many standard deviations does the reported error mean?
    double error_standard_deviations = 3.0;
    
    // default infinite number of decks (it's faster)
    unsigned int n_decks = 0;
    unsigned int shoe_cards[53];  // index 0 not used
    unsigned int n_shuffles = 0;
    
    struct {
      std::list<PlayerHand> hands;
      std::list<PlayerHand>::iterator currentHand;
    
      unsigned int splits = 0;

      // TODO: separate handsDealt from handsPlayed
      unsigned int n_hands = 0;  // this is different from the dealer's due to splitting
    
      unsigned int handsInsured = 0;
      unsigned int handsDoubled = 0;
      unsigned int blackjacksPlayer = 0;
      unsigned int blackjacksDealer = 0;
    
      unsigned int bustsPlayer = 0;
      unsigned int bustsPlayerAllHands = 0; // this is incremented only if all hands were busted (only for enhc)
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
      
      // these variables are used to compute the running mean and variance 
      double currentOutcome = 0;
      double mean = 0;
      double M2 = 0;
      double variance = 0;
    } playerStats;

    std::string report_file_path;
    int report_verbosity = 5;
    
    void updateMeanAndVariance(void);
    

    
  private:
    
    unsigned int rng_seed;
    std::random_device dev_random;
    std::mt19937 rng;
    std::uniform_int_distribution<unsigned int> fakeshoe;
    
    size_t cut_card_position = 0;
    bool last_pass = false;
    unsigned int pos;
    
    unsigned int dealer_up_card;
    unsigned int dealer_hole_card;
    unsigned int player_first_card;
    unsigned int player_second_card;

    bool h17 = true;
    bool das = true;
    bool doa = true;
    bool enhc = false;
//    bool rsa = false;  // TODO
    bool shuffle_every_hand = false;
    bool quit_when_arranged_cards_run_out = false;
    bool new_hand_reset_cards = true;
    bool dealer_draws_even_if_player_busted = false;
    
    std::vector<int> arranged_cards;
    size_t n_arranged_cards = 0; // just to prevent calling size() each time we draw a card
    size_t i_arranged_cards = 0;

    unsigned int resplits = 3;
    unsigned int max_bet = 0;
    unsigned int number_of_burnt_cards = 0;
    
    double insurance = 0;
    double blackjack_pays = 1.5;
    
    double penetration = 0.75;
    double penetration_sigma = 0;
    
    int read_arranged_cards(std::istringstream iss); // maybe this should go into the parent class?
    void can_double_split(void);
    
    bool done = false;
    std::list<reportItem> report;
    
};

template <typename ... Args> std::string string_format( const std::string& format, Args ... args);
template <typename ... Args> std::string string_format2( const std::string& format, Args ... args);
}

#endif
