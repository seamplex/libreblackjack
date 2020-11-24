#include <iostream>
#include <cmath>
#include <memory>
#include <string>

#include "base.h"


// https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
template<typename ... Args>
std::string string_format( const std::string& format, Args ... args)
{
  size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
  if (size <= 0) {
    return std::string("");
  }
  std::unique_ptr<char[]> buf(new char[size]); 
  snprintf(buf.get(), size, format.c_str(), args ...);
  return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

void Dealer::updateMeanAndVariance(void) {
  double delta = playerStats.currentOutcome - playerStats.mean;
  playerStats.mean += delta / (double)(n_hand);
  playerStats.M2 += delta * (playerStats.currentOutcome - playerStats.mean);
  playerStats.variance = playerStats.M2 / (double)(n_hand);
  return;
}


void Dealer::reportPrepare(void) {

  // we need to update these statistics after the last played hand
  updateMeanAndVariance();  
    
  double error = error_standard_deviations * sqrt (playerStats.variance / (double) n_hand);

  int precision = (int) (std::ceil(-std::log10(error))) - 2;
  if (precision < 0) {
    precision = 0;
  }
  std::string format = "\"(%+." + std::to_string(precision) + "f ± %." + std::to_string(precision) + "f) %%%%\"";
  report.push_back(reportItem(1, "result", string_format(format, 100*playerStats.mean, 100*error), 0.0));
  
  report.push_back(reportItem(2, "mean",      "%g", playerStats.mean));
  report.push_back(reportItem(2, "error",     "%g", error));
  report.push_back(reportItem(2, "hands",     "%g", n_hand));
  report.push_back(reportItem(2, "bankroll",  "%g", playerStats.bankroll));

  report.push_back(reportItem(3, "bustsPlayer", "%g", playerStats.bustsPlayer / (double) n_hand));
  report.push_back(reportItem(3, "bustsDealer", "%g", playerStats.bustsDealer / (double) n_hand));
  report.push_back(reportItem(3, "wins",        "%g", playerStats.wins / (double) n_hand));
  report.push_back(reportItem(3, "pushes",      "%g", playerStats.pushes / (double) n_hand));
  report.push_back(reportItem(3, "losses",      "%g", playerStats.losses / (double) n_hand));

  report.push_back(reportItem(4, "total_money_waged", "%g", playerStats.totalMoneyWaged));

  report.push_back(reportItem(5, "variance",  "%g", playerStats.variance));
  report.push_back(reportItem(5, "deviation", "%g", sqrt(playerStats.variance)));
    

  return;
}

int Dealer::writeReportYAML(void) {
//   FILE *file;
//   struct rusage usage;
//   double wall;
//   double ev, error;
//   int precision;
//   char format[32];

//  if ((file = blackjack_conf.yaml_report) == NULL) {
//    file = stderr;
//  }

  // TODO: choose if comments with explanations are to be added
  // TODO: choose verbosity level
  std::cerr <<  "---" << std::endl;
  for (auto item : report) {
    if (item.level <= reportVerbosity) {  
      std::cerr << item.key << ": ";
      fprintf(stderr, item.format.c_str(), item.value);
      std::cerr << std::endl;
    }  
  }
  
//   std::cerr <<  "rules:" << std::endl;
//   std::cerr <<  "  decks:                  " << decks << std::endl;
//   std::cerr <<  "  hands:                  " << n_hands  << std::endl;
//   std::cerr <<  "  hit_soft_17:            %d\n",
// 	   blackjack_conf.hit_soft_17);
//   std::cerr <<  "  double_after_split:     %d\n",
// 	   blackjack_conf.double_after_split);
//   std::cerr <<  "  blackjack_pays:         %g\n",
// 	   blackjack_conf.blackjack_pays);
//   std::cerr <<  "  rng_seed:               %d\n", blackjack_conf.rng_seed);
//   std::cerr <<  "  number_of_burnt_cards:  %d\n",
// 	   blackjack_conf.number_of_burnt_cards);
//   std::cerr <<  "  no_negative_bankroll:   %d\n",
// 	   blackjack_conf.no_negative_bankroll);
//   std::cerr <<  "  max_bet:                %d\n", blackjack_conf.max_bet);

//   if (blackjack_conf.decks > 0)
//     {
//       std::cerr <<  "  penetration:            %g\n",
// 	       blackjack_conf.penetration);
//       std::cerr <<  "  penetration_sigma:      %g\n",
// 	       blackjack_conf.penetration_sigma);
//     }

  // TODO
//   if (getrusage (RUSAGE_SELF, &usage) == 0) {
//     std::cerr <<  "cpu:" << std::endl;
//     std::cerr <<  "  user:             " + (usage.ru_utime.tv_sec + 1e-6 * usage.ru_utime.tv_usec) << std::endl;
//     std::cerr <<  "  system:           " + (usage.ru_stime.tv_sec + 1e-6 * usage.ru_stime.tv_usec) << std::endl;

    // measue final wall time
    // TODO: chrono
//     gettimeofday (&wall_time_final, NULL);
//     wall = (blackjack.wall_time_final.tv_sec -
// 	 blackjack.wall_time_initial.tv_sec) +
// 	1e-6 * (blackjack.wall_time_final.tv_usec -
// 		blackjack.wall_time_initial.tv_usec);
//       std::cerr <<  "  wall:             %g\n", wall);

      // speed
//       std::cerr <<  "  second_per_hand:  %.1e\n", wall / blackjack.hand);
//       std::cerr <<  "  hands_per_second: %.1e\n", blackjack.hand / wall);
//   }

//   std::cerr <<  "player: " << std::endl;
//   std::cerr <<  "  wins:               " ((double) player->wins / (double) player->number_of_hands)) << std::endl;
//   std::cerr <<  "  pushes:             %g\n",
// 	   (double) player->pushes / (double) player->number_of_hands);
//   std::cerr <<  "  losses:             %g\n",
// 	   (double) player->losses / (double) player->number_of_hands);
//   std::cerr <<  "  dealer_blackjacks:  %g\n",
// 	   (double) player->dealer_blackjacks /
// 	   (double) player->number_of_hands);
//   std::cerr <<  "  player_blackjacks:  %g\n",
// 	   (double) player->player_blackjacks /
// 	   (double) player->number_of_hands);
//   std::cerr <<  "  dealer_busts:       %g\n",
// 	   (double) player->dealer_busts / (double) player->number_of_hands);
//   std::cerr <<  "  player_busts:       %g\n",
// 	   (double) player->player_busts / (double) player->number_of_hands);
//   std::cerr <<  "  doubled_hands:      %g\n",
// 	   (double) player->doubled_hands / (double) player->number_of_hands);
//   std::cerr <<  "  doubled_wins:       %g\n",
// 	   (double) player->doubled_wins / (double) player->number_of_hands);
//   std::cerr <<  "  insured_hands:      %g\n",
// 	   (double) player->insured_hands / (double) player->number_of_hands);
//   std::cerr <<  "  insured_wins:       %g\n",
// 	   (double) player->insured_wins / (double) player->number_of_hands);

//   std::cerr <<  "  number_of_hands:    %g\n",
// 	   (double) player->number_of_hands);
//   std::cerr <<  "  number_of_shuffles: %g\n", (double) blackjack.shuffles);
//   std::cerr <<  "  total_money_waged:  %g\n",
// 	   (double) player->total_money_waged);
//   std::cerr <<  "  worst_bankroll:     %g\n",
// 	   (double) player->worst_bankroll);
//   std::cerr <<  "  final_bankroll:     %g\n", (double) player->bankroll);

  // return is a keyword!

//   precision = (int) (ceil (-log10 (error))) - 2;
//   if (precision >= 0) {
//     snprintf (format, 32, ("%%+.%df ± %%.%df"), precision, precision);
//   } else {
//     snprintf (format, 32, ("%%+.0f ± %%.0f"));
//   }


//   std::cerr <<  "  variance:           % g\n", player->variance);
//   std::cerr <<  "  deviation:          % g\n", sqrt (player->variance));
//   std::cerr <<  "  error:              % g\n",
// 	   sqrt (player->variance / (double) (blackjack.hand)));
//   std::cerr <<  "  result:             \"("), std::cerr <<  format,
// 							100.0 * ev,
// 							100 * error);
//   std::cerr <<  ") %%\"" << std::endl;
  std::cerr <<  "..." << std::endl;

  return 0;
}
