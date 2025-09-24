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

ref=-0.158

echo "always stand"
yes stand | $blackjack -n5e5 --flat_bet=true --no_insurance=true --report=stand.yaml > /dev/null 
actual=$(yq .mean stand.yaml)
tol=$(yq .error stand.yaml)
echo $actual $ref $tol
awk -v a="$actual" -v r="$ref" -v t="$tol" 'BEGIN { exit !((a >= (r-t)) && (a <= (r+t))) }'
exitifwrong $?
echo "ok"
