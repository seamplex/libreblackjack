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


echo "no bust"
ref=-0.079

rm -f fifo-nobust
mkfifo fifo-nobust
${extra}${blackjack} -n2e5 --report=no-bust.yaml < fifo-nobust | gawk -f ../players/05-no-bust/no-bust.awk > fifo-nobust

actual=$(yq .mean no-bust.yaml)
tol=$(yq .error no-bust.yaml)
echo $actual $ref $tol
awk -v a="$actual" -v r="$ref" -v t="$tol" 'BEGIN { exit !((a >= (r-t)) && (a <= (r+t))) }'
exitifwrong $?
echo "ok"
