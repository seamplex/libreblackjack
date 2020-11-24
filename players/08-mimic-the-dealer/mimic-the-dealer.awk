#!/usr/bin/gawk -f
function abs(x){return ( x >= 0 ) ? x : -x } 

/bet\?/ {
  print "1";
  fflush();
}

/insurance\?/ {
  print "no";
  fflush();
}

/play\?/ {
  # mimic the dealer: hit until 17 (hit soft 17)
  if (abs($2) < 17 || $2 == -17) {   # soft hands are negative
    print "hit";
  } else {
    print "stand";
  }
  fflush();  
}

/bye/ {
  exit;
}
