/*------------ -------------- -------- --- ----- ---   --       -            -
 *  Libre Blackjack - base dealer
 *
 *  Copyright (C) 2025 jeremy theler
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
#include "dealer.h"

namespace lbj {
Dealer::Dealer(Configuration &conf) {
    
  conf.set(&error_standard_deviations, {"error_standard_deviations"});
  conf.set(report_file_path, {"report_file_path", "report_file", "report"});
  conf.set(&report_verbosity, {"report_verbosity", "report_level"});
    
}
}
