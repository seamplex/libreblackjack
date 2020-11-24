/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - tty interactive player
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

#include "conf.h"
#include "blackjack.h"
#include "internal.h"


Internal::Internal(Configuration &conf) {
  return;
}

void Internal::info(Libreblackjack::Info msg, int p1, int p2) {
  return;
}

int Internal::play() {

  switch (actionRequired) {
    case Libreblackjack::PlayerActionRequired::Bet:
      currentBet = 1;
      actionTaken = Libreblackjack::PlayerActionTaken::Bet;
    break;

    case Libreblackjack::PlayerActionRequired::Insurance:
      actionTaken = Libreblackjack::PlayerActionTaken::DontInsure;
    break;
    
    case Libreblackjack::PlayerActionRequired::Play:
      actionTaken = (playerValue < 12) ? Libreblackjack::PlayerActionTaken::Hit : Libreblackjack::PlayerActionTaken::Stand;
    break;  
    
    case Libreblackjack::PlayerActionRequired::None:
    break;  
    
  }
  
  return 0;
}
