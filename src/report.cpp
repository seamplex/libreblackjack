#include <iostream>
#include <cmath>
#include <memory>
#include <string>

#include "base.h"

namespace lbj {

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


void Dealer::prepareReport(void) {
  
//  if (n_hand != n_hands) {
//    n_hand--;
//  }

  if (n_hand <= 1) {
    return;
  }

  // we need to update these statistics after the last played hand
  updateMeanAndVariance();  
    
  double error = error_standard_deviations * sqrt (playerStats.variance / (double) n_hand);

  int precision = 0;
  if (error > 0) {
    precision = (int) (std::ceil(-std::log10(error))) - 2;
    if (precision < 0) {
      precision = 0;
    }
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
    
  if (n_hand <= 1) {
    return 0;
  }  

  FILE *out = stderr;  
  if (report_file_path != "") {
    out = fopen(report_file_path.c_str(), "w");
  }
    
  // TODO: choose if comments with explanations are to be added
  fprintf(out, "---\n");
  for (auto item : report) {
    if (item.level <= report_verbosity) {  
      fprintf(out, "%s: ", item.key.c_str());
      fprintf(out, item.format.c_str(), item.value);
      fprintf(out, "\n");
    }  
  }
  fprintf(out, "...\n");
  
  if (report_file_path != "") {
    fclose(out);
  }

  return 0;
}
}