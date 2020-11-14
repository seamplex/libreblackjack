/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - stdio player
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

#include <iostream>

#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif


#include "blackjack.h"
#include "stdinout.h"

StdInOut::StdInOut(void) {
}

void StdInOut::info(Info msg, int p1, int p2) {
  return;
}

int StdInOut::play() {
  
  std::cin >> input_buffer;

  // TODO: check EOF
/*  
  if (input_buffer == "hit" || input_buffer == "h") {
    *command = Command::Hit;
  } else {
    *command = Command::None;
  }
*/
  return 0;
}
