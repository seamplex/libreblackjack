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
checkgawk   # this test does not work with mawk, it does not like the fifo

if [ ! -e functions.sh ]; then
  cd tests
  extra="../"
fi
if [ ! -e functions.sh ]; then
  exit 77
fi


echo "mimic the dealer"
ref=-0.055

rm -f fifo-mimic
mkfifo fifo-mimic
${extra}${blackjack} -n1e5 --report=mimic.yaml < fifo-mimic | gawk -f ../players/08-mimic-the-dealer/mimic-the-dealer.awk > fifo-mimic

actual=$(yq .mean mimic.yaml)
tol=$(yq .error mimic.yaml)
echo $actual $ref $tol
awk -v a="$actual" -v r="$ref" -v t="$tol" 'BEGIN { exit !((a >= (r-t)) && (a <= (r+t))) }'
exitifwrong $?
echo "ok"
