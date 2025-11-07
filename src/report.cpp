#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <memory>
#include <string>

#include "dealer.h"


// TODO: make a separate report class and construct with the dealer & player
namespace lbj {

void Dealer::updateMeanAndVariance(void) {
  double delta = playerStats.currentOutcome - playerStats.mean;
  playerStats.mean += delta / (double)(n_hand);
  playerStats.M2 += delta * (playerStats.currentOutcome - playerStats.mean);
  playerStats.variance = playerStats.M2 / (double)(n_hand-1);
  return;
}


void Dealer::prepareReport(void) {

  // TODO: if n_hand is one the error is NaN
/*  
  if (n_hand <= 1) {
    return;
  }
*/

  // we need to update these statistics after the last played hand
  updateMeanAndVariance();  
    
  double total = static_cast<double>(n_hand);
  double error = error_standard_deviations * sqrt (playerStats.variance / total);

  int precision = 0;
  if (error > 0) {
    precision = (int) (std::ceil(-std::log10(error))) - 2;
    if (precision < 0) {
      precision = 0;
    }
  }
  
  std::ostringstream result;
  result << "(" 
      << std::showpos << std::fixed << std::setprecision(precision) << 100*playerStats.mean
      << " ± " 
      << std::noshowpos << std::fixed << std::setprecision(precision) << 100*error
      << ")";
  
  report.push_back(reportItem(1, "result", result.str()));
  
  std::string rules = this->rules();
  if (rules != "") {
    report.push_back(reportItem(1, "rules", rules));
  }
  
  report.push_back(reportItem(2, "mean",      playerStats.mean));
  report.push_back(reportItem(2, "error",     error));
  report.push_back(reportItem(2, "hands",     n_hand));
  report.push_back(reportItem(2, "bankroll",  playerStats.bankroll));


  report.push_back(reportItem(3, "busts_player_n",     playerStats.bustsPlayer));
  report.push_back(reportItem(3, "busts_dealer_n",     playerStats.bustsDealer));
  
  report.push_back(reportItem(3, "busts_player",       playerStats.bustsPlayer / total));
  report.push_back(reportItem(3, "busts_player_nobj",  playerStats.bustsPlayer / (total - playerStats.blackjacksPlayer)));

  double b1 = playerStats.bustsDealer / total;
  double b2 = playerStats.bustsDealer / (total - playerStats.bustsPlayerAllHands);
  double b3 = playerStats.bustsDealer / (total - playerStats.blackjacksDealer);
  double b4 = playerStats.bustsDealer / (total - playerStats.bustsPlayerAllHands - playerStats.blackjacksDealer);
  
//  double b2 = b1 * (1 + (double)playerStats.bustsPlayerAllHands/total);
//  double b3 = b2 * (1 + (double)playerStats.blackjacksDealer/total);
  report.push_back(reportItem(3, "busts_dealer1",          b1));
  // report.push_back(reportItem(3, "busts_dealer2old",       playerStats.bustsDealer / (double) (n_hand - playerStats.bustsPlayerAllHands)));
  report.push_back(reportItem(3, "busts_dealer2",       b2));
  // report.push_back(reportItem(3, "busts_dealer3old",       playerStats.bustsDealer / (double) (n_hand - playerStats.bustsPlayerAllHands - playerStats.blackjacksDealer)));
  report.push_back(reportItem(3, "busts_dealer3",       b3));
  report.push_back(reportItem(3, "busts_dealer3",       b4));
  
  report.push_back(reportItem(3, "wins",         playerStats.wins / total));
  report.push_back(reportItem(3, "pushes",       playerStats.pushes / total));
  report.push_back(reportItem(3, "losses",       playerStats.losses / total));

  report.push_back(reportItem(4, "total_money_waged",      playerStats.totalMoneyWaged));
  report.push_back(reportItem(4, "blackjacks_player",      playerStats.blackjacksPlayer / total));
  report.push_back(reportItem(4, "blackjacks_dealer",      playerStats.blackjacksDealer / total));
//  if (playerStats.bustsPlayerAllHands != 0) {
    report.push_back(reportItem(4, "blackjacks_dealer_real1", playerStats.blackjacksDealer / (double) (n_hand - playerStats.bustsPlayerAllHands)));
    report.push_back(reportItem(4, "blackjacks_dealer_real2", playerStats.blackjacksDealer / (double) (n_hand - playerStats.bustsPlayer)));
//  }

  report.push_back(reportItem(5, "variance",  playerStats.variance));
  report.push_back(reportItem(5, "deviation", sqrt(playerStats.variance)));
    

  return;
}

int Dealer::writeReportYAML(void) {
    
  // if (n_hand <= 1) {
  //   return 0;
  // }  


  std::ostream* out = &std::cerr;
  std::ofstream file_stream;

  if (report_file_path.empty() || report_file_path == "stderr") {
    out = &std::cerr;
  } else if (report_file_path == "stdout") {
    out = &std::cout;
  } else {
    file_stream.open(report_file_path);
    if (!file_stream.is_open()) {
        std::cerr << "Error: could not open file " << report_file_path << std::endl;
        return -1;  // or throw exception, or handle error as appropriate
    }
    out = &file_stream;
  }  

  // TODO: choose if comments with explanations are to be added
  *out << "---" << std::endl; 
  for (auto &item : report) {
    if (item.level <= report_verbosity) {
      *out << item.key << ": ";
        
      if (item.string != "") {
        *out << "\"" << item.string << "\"";  // This assumes item.value can be streamed directly
      } else{
        *out << item.value;  
      }
        
      *out << std::endl;
    }
  }
  *out << "..." << std::endl;
  
  return 0;
}
}
