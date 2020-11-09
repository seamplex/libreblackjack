#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>

#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "conf.h"
#include "blackjack.h"
#include "tty.h"

Tty::Tty(Configuration &conf) {
    
  conf.set(&flat_bet, {"flat_bet", "flatbet"});  
  conf.set(&no_insurance, {"no_insurance", "dont_insure"});  

#ifdef HAVE_LIBREADLINE
  prompt = cyan + " > " + reset;
#endif
}

void Tty::info(Info msg, int intData) {
  std::string s;
  
  switch (msg) {
    case Info::NewHand:
//      s = "new_hand";  
      s = "Starting new hand #" + std::to_string(intData);
    break;
    case Info::Shuffle:
//      s = "shuffle";  
      s = "Deck needs to be shuffled.";
    break;
    case Info::CardPlayerFirst:
//      s = "card_player_first";
      s = "Player's first card is " + card[intData].utf8();
    break;
    case Info::CardDealerUp:
//      s = "card_dealer_up";
      s = "Dealer's up card " + card[intData].utf8();
    break;
    case Info::CardPlayerSecond:
//      s = "card_player_second";
      s = "Player's second card is " + card[intData].utf8();
    break;
    case Info::CardDealerHoleDealt:
//      s = "card_dealer_hole";
      s = "Dealer's hole card is dealt";
    break;
    case Info::CardDealerHoleRevealed:
//      s = "card_dealer_hole";
      s = "Dealer's hole card was " + card[intData].utf8();
    break;
    case Info::DealerBlackjack:
//      s = "dealer_blackjack";
      s = "Dealer has Blackjack";
      // TODO: draw dealer's hand
    break;
    case Info::PlayerWinsInsurance:
//      s = "player_wins_insurance";
      s = "Player wins insurance";
    break;
    case Info::PlayerBlackjackAlso:
//      s = "player_blackjack_also";
      s = "Player also has Blackjack";
    break;
    case Info::PlayerPushes:
//      s = "player_pushes";
      s = "Player pushes";
      // TODO:
      //  print_hand_art (player->current_hand);
    break;
    case Info::PlayerLosses:
//      s = "player_losses";
      s = "Player losses";
      // TODO:
      //  print_hand_art (player->current_hand);
    break;
    case Info::PlayerBlackjack:
//      s = "blackjack_player";
      s = "Player has Blackjack";
      // TODO:
      //  print_hand_art (player->current_hand);
    break;
    case Info::PlayerWins:
//      s = "player_wins";
      s = "Player wins " + std::to_string(intData);
    break;
    
    case Info::NoBlackjacks:
//      s = "no_blackjacks";
      s = "No blackjacks";
    break;
    
    case Info::Bye:
//      s = "bye";  
      s = "Bye bye! We'll play Blackjack again next time.";
    break;
  }
  
  if (delay > 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
  }  
  std::cout << green << s << reset << std::endl;
  
  
  if (msg == Info::CardDealerHoleDealt) {
  //  hand.render(hand.holeCardShown);
    currentHand->render();      
  }
  
  return;
}

int Tty::play() {
  
#ifdef HAVE_LIBREADLINE
    
  if ((input_buffer = readline(prompt.c_str())) == nullptr) {
      
    // EOF means "quit"
    actionTaken = PlayerActionTaken::Quit;
    std::cout << std::endl;
    
  } else {

    add_history(input_buffer);
    actionTaken = PlayerActionTaken::None;

  // TODO: convertir a string y usar algo comun para non-readline
    // check common commands first
    if (strcmp(input_buffer, "quit") == 0 || strcmp(input_buffer, "q")== 0) {
      actionTaken = PlayerActionTaken::Quit;
    } else if (strcmp(input_buffer, "help") == 0) {
      actionTaken = PlayerActionTaken::Help;
    } else if (strcmp(input_buffer, "count") == 0 || strcmp(input_buffer, "c")== 0) {
      actionTaken = PlayerActionTaken::Count;
    } else if (strcmp(input_buffer, "upcard") == 0 || strcmp(input_buffer, "u")== 0) {
      actionTaken = PlayerActionTaken::UpcardValue;
    } else if (strcmp(input_buffer, "bankroll") == 0 || strcmp(input_buffer, "b")== 0) {
      actionTaken = PlayerActionTaken::Bankroll;
    } else if (strcmp(input_buffer, "hands") == 0) {
      actionTaken = PlayerActionTaken::Hands;
    } else if (strcmp(input_buffer, "table") == 0) {
      actionTaken = PlayerActionTaken::Table;
    }
    
    if (actionTaken == PlayerActionTaken::None) {
      switch (actionRequired) {

        case PlayerActionRequired::Bet:
          currentBet = std::stoi(input_buffer);
          actionTaken = PlayerActionTaken::Bet;
        break;

        case PlayerActionRequired::Insurance:
          if (strcmp(input_buffer, "y") == 0 || strcmp(input_buffer, "yes") == 0) {
            actionTaken = PlayerActionTaken::Insure;
          } else if (strcmp(input_buffer, "n") == 0 || strcmp(input_buffer, "no") == 0) {
            actionTaken = PlayerActionTaken::DontInsure;
          } else {
            // TODO: chosse if we allow not(yes) == no
            actionTaken = PlayerActionTaken::None;  
          }
        break;

        case PlayerActionRequired::Play:

          // TODO: sort by higher-expected response first
          if (strcmp(input_buffer, "h") == 0 || strcmp(input_buffer, "hit") == 0) {
            actionTaken = PlayerActionTaken::Hit;
          } else if (strcmp(input_buffer, "s") == 0 || strcmp(input_buffer, "stand") == 0) {
            actionTaken = PlayerActionTaken::Stand;
          } else if (strcmp(input_buffer, "d") == 0 || strcmp(input_buffer, "double") == 0) {
            actionTaken = PlayerActionTaken::Stand;
          } else if (strcmp(input_buffer, "p") == 0 || strcmp(input_buffer, "pair") == 0 || strcmp(input_buffer, "split") == 0) {
            actionTaken = PlayerActionTaken::Split;
          } else {
            actionTaken = PlayerActionTaken::None;
          }
        break;
      }
    }
      
    free(input_buffer);
  }
  
#else

  std::cout << prompt;
  std::cin >> input_buffer;

  // TODO: check EOF
  // TODO: esto puede ir en algo comun para tty y stdout
  if (input_buffer == "hit" || input_buffer == "h") {
    *command = Command::Hit;
  } else {
    *command = Command::None;
  }
  
#endif
  
  return 0;
}
