/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - version reporting
 *
 *  Copyright (C) 2020,2025 jeremy theler
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
  std::cout << "usage: " << program_name <<  " [options] [-c path_to_conf_file]" << std::endl;
  std::cout << ENGINE << std::endl;

  std::cout << std::endl;
  
  std::cout << "-c<path> or --conf=path    Specify the path to the configuration file." << std::endl;
  std::cout << "                           Default is ./blackjack.conf." << std::endl;
  std::cout << std::endl;
  std::cout << "-n<n> or --hands=n         Specify the number of hands to play." << std::endl;
  std::cout << "                           Corresponds to the hands variable in the configuration file." << std::endl;
  std::cout << std::endl;
  std::cout << "-d<n> or --decks=n         Specify the number of decks to use in the shoe." << std::endl;
  std::cout << "                           Corresponds to the decks variable in the configuration file." << std::endl;
  std::cout << std::endl;
  std::cout << "-f or --flatbet            Do not ask for the amount to bet before starting" << std::endl;
  std::cout << "                           a new hand and use a flat unit bet." << std::endl;
  std::cout << "                           Corresponds to the flat_bet variable in the configuration file." << std::endl;
  std::cout << std::endl;
  std::cout << "-i or --internal           Use the internal player to play against the dealer." << std::endl;
  std::cout << "                           See the manual for details and instructions to define the rules" << std::endl;
  std::cout << "                           and optionally, the playing strategy and/or arranged shoes." << std::endl;
  std::cout << std::endl;
  std::cout << "-p or --progress           Show a progress bar when playing a fixed number of hands." << std::endl;
  std::cout << std::endl;
  std::cout << "-h or --help               Print this informative help message into standard output" << std::endl;
  std::cout << "                           and exit successfully." << std::endl;
  std::cout << std::endl;
  std::cout << "-v or --version            Print the version number and licensing information into" << std::endl;
  std::cout << "                           standard output and then exit successfully." << std::endl;
  std::cout << std::endl;
  std::cout << "--configuration_variable[=value]" << std::endl;
  std::cout << "                           Any configuration variable from the configuration file can be" << std::endl;
  std::cout << "                           set from the command line. For example, passing --no_insurance" << std::endl;
  std::cout << "                           is like setting no_insurance = 1 in the configuration file." << std::endl;
  std::cout << "                           Command-line options override setting in the configuration file." << std::endl;
  std::cout << std::endl;
  std::cout << "If no configuration file is given, a file named blackjack.conf" << std::endl;
  std::cout << "in the current directory is used, provided it exists." << std::endl;
  std::cout << "See the full documentation for the available options and the default values." << std::endl;

  return;
}


void copyright(void) {
  std::cout << "copyright (c) " << 2016 << "--" << 2025 << " jeremy theler." << std::endl;
  std::cout << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>" << std::endl;
  std::cout << "This is free software: you are free to change and redistribute it." << std::endl;
  std::cout << "There is NO WARRANTY, to the extent permitted by law." << std::endl << std::endl;
  return;
}

}
