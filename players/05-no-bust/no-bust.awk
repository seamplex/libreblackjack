#!/usr/bin/gawk -f
# mawk does not work, it hangs due to the one-sided FIFO
{
  if (n++ > 1234567) {
    print "quit";
  }
}
/bet\?/ {
  print "1"
  fflush()
}
/insurance\?/ {
  print "no"
  fflush()
}
/play\?/ {
  if ($2 < 12) {
    print "hit";
  } else {
    print "stand";
  }
  fflush()
}
/bye/ {
  exit;
}
