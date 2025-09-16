/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - version reporting
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
#include <cstring>
#include "version-vcs.h"

#define ENGINE  "a free & open blackjack engine\n"

namespace lbj {

void shortversion(void) {

  std::cout << "LibreBlackjack ";  
#ifdef LIBREBLACKJACK_VCS_BRANCH
  std::cout << LIBREBLACKJACK_VCS_VERSION << ((LIBREBLACKJACK_VCS_CLEAN == 0) ? "" : "+Δ") << ((strcmp(LIBREBLACKJACK_VCS_BRANCH, "master")) ? LIBREBLACKJACK_VCS_BRANCH : "") << std::endl;
#else
  std::cout << PACKAGE_VERSION << std::endl;
#endif

//   std::cout << (_(ENGINE)) << std::endl;
  std::cout << ENGINE << std::endl;
  return;
}

void help(const char *program_name) {
  std::cout << "Usage: " << program_name <<  "[options] [path_to_conf_file]" << std::endl;
  std::cout << ENGINE << std::endl;

  std::cout << std::endl;
  std::cout << "If no configuration file is given, a file named blackjack.conf" << std::endl;
  std::cout << "in the current directory is used, provided it exists." << std::endl;
  std::cout << "See the full documentation for the available options and the default values." << std::endl;
  std::cout << std::endl;
  std::cout << "  -h, --hands=N    set the number of hands to play before quiting" << std::endl;
  std::cout << "  -d, --decks=N    set the number of decks to use" << std::endl;
  std::cout << "  -f, --flatbet    do not ask for the bet before each hand, use a unit flat bet" << std::endl;
  std::cout << "  -i, --internal   use the internal player to play against the dealer (not interactive)" << std::endl;
  std::cout << std::endl;
  std::cout << "  -h, --help       display this help and exit" << std::endl;
  std::cout << "  -v  --version    output version information and exit" << std::endl;

  return;
}


void copyright(void) {
  std::cout << "copyright (c) " << 2016 << "--" << 2020 << " jeremy theler." << std::endl;
  std::cout << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>" << std::endl;
  std::cout << "This is free software: you are free to change and redistribute it." << std::endl;
  std::cout << "There is NO WARRANTY, to the extent permitted by law." << std::endl << std::endl;
  return;
}

}
