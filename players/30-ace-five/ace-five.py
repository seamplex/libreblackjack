#!/usr/bin/python3

# plays the wizard's ace-five count
# <http://wizardofodds.com/games/blackjack/appendix/17/>
# with the simple strategy at
# <http://wizardofodds.com/games/blackjack/appendix/21/>
#
#
#  Player's hand     Dealer's upcard
# 
#  -- hard --------------------------------
#                    2 to 6     7 to A
#  4 to 8              H          H   
#  9                   D          H   
#  10 or 11        D with more than dealer
#  12 to 16            S          H
#  17 to 21            S          S
#
#  -- soft --------------------------------
#                    2 to 6     7 to A
#  13 to 15            H          H   
#  16 to 18            D          H   
#  19 to 21            S          S
#
#  -- split -------------------------------
#                    2 to 6     7 to A
#  22,33,66,77,99      Y          N   
#  88,AA               Y          Y   
#  44,55,TT            N          N
#
#  Plus:
#    1. surrender 16 vs 10
#    2. never take insurance
#    3. if not allowed to double, stand with soft 18
#

import sys 

max_bet = 32

n_player_cards = 0
count = 0
bet = 1

import fileinput

for linenl in fileinput.input():
  line = linenl.rstrip()
  #print("<- %s" % line, file = sys.stderr) 
  
  if line == "bye":
    sys.exit(0)
    
  elif line == "shuffling":
    count = 0
    bet = 1
        
  elif line[:8] == "new_hand": 
    n_player_cards = 0

  elif line == "insurance?": 
    print("no", flush = True)
        
  elif line == "bet?":
    #if count <= 1:
      #bet = 1
    #elif bet < max_bet:
      #bet *= 2
    #print(bet, flush = True)
    print("1", flush = True)
    
  elif line[:15] == "player_split_ok":    
    n_player_cards = 1

  elif line[:5] == "card_":
    tokens = line.split()
    if (len(tokens) > 1):
      card = tokens[1][0]
    else:
      card = ""
    # count aces and fives
    if card == "A":
      count -= 1
    elif card == "5":
      count += 1

    if line[:11] == "card_player":
      n_player_cards += 1
      if n_player_cards == 1:
        card_player_first = card
      elif n_player_cards == 2:
        card_player_second = card
                      
  elif line[:5] == "play?":
    tokens = line.split()
    player = int(tokens[1])
    dealer = abs(int(tokens[2]))
    action = "quit"
    
    #print("player_cards %d" % n_player_cards, file = sys.stderr) 
    #print("card_player_first %s" % card_player_first, file = sys.stderr) 
    #print("card_player_second %s" % card_player_second, file = sys.stderr) 

    if n_player_cards == 2 and card_player_first == card_player_second and \
        ((card_player_first == "8" or card_player_first == "A") or \
         (dealer < 7 and \
            (card_player_first == "2" or \
             card_player_first == "3" or \
             card_player_first == "6" or \
             card_player_first == "7" or \
             card_player_first == "9"))):  # --- split------------------------------------
      action = "split"                     # always split aces and 8s but 2,3,6,7 & 9 only against 6 or less
      
    else:
      if player > 0:                       # --- hard ------------------------------------
        if player < 9:         
          action = "hit"                   # hit 4 to 8 against anything
        elif player == 9:
          if dealer < 7: 
            if n_player_cards == 2:
              action = "double"            # double 9 against 2 to 6
            else:
              action = "hit"               # else hit
          else:
            action = "hit"                 # hit 9 against 7 to A
        elif player < 12:
          if player > dealer:
            if n_player_cards == 2:
              action = "double"            # double with 10 or 11 and more than dealer
            else:
              action = "hit"
          else:
            action = "hit"                 # otherwise hit
        elif player < 17:
          if dealer < 7:
            action = "stand"               # stand with 12 to 16 against 2 to 6
          else:
            action = "hit"                 # hit with 12 to 16 against 7 to A
        else:
          action = "stand"                 # stand with hard 17 or more
      else:
        # soft
        player = abs(player)
        if player < 16:        
          action = "hit"                   # hit every soft hand less than 16
        elif player < 19:
          if dealer < 7:
            if n_player_cards == 2:
              action = "double"            # double soft 16 to 18 againt 2 to 6
            elif player == 18:
              action = "stand"             # stand with soft 18
            else:
              action = "hit"               # hit soft 17
          else:
            action = "hit"                 # hit soft 16 to 18 against 7 to A
        else:
          action = "stand"                 # stand with soft 19 or more
    print(action, flush = True)
    #print("-> %s" % action, file = sys.stderr) 
    
  elif line == "invalid_command":
    print("I sent an invalid command!", file = sys.stderr) 
  

