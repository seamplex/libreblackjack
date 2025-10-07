#!/bin/sh
for i in . tests; do
  if [ -e ${i}/functions.sh ]; then
    . ${i}/functions.sh 
  fi
done
if [ -z "${functions_found}" ]; then
  echo "could not find functions.sh"
   exit 1
fi

checkyq

echo "known mean and variance"
if [ -d tests ]; then
  cd tests
fi
if [ -d variance ]; then
  cd variance
fi
if [ ! -e play.txt ]; then
  echo "error: cannot find play.txt"
  exit 1
fi
rm -rf report.yaml
../../blackjack < play.txt

mean=$(yq .mean report.yaml)
variance=$(yq .variance report.yaml)
echo $mean $variance

if [ "x$mean" != "x0.55" ]; then
  exit 1
fi

if [ "x$variance" != "x1.46944" ]; then
  exit 1
fi

exit 0
