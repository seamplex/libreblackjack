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

# https://wizardofodds.com/games/blackjack/appendix/9/6dh17r4/
# ref=-0.006151
ref=-0.007

echo "6 decks h17 das nrsa 1e6"
$blackjack -i --report=ahc.yaml --decks=6
actual=$(yq .mean ahc.yaml)
tol=$(yq .error ahc.yaml)
echo $actual $ref $error
awk -v a="$actual" -v r="$ref" -v t="$tol" 'BEGIN { exit !((a >= (r-t)) && (a <= (r+t))) }'
exitifwrong $?
echo "ok"
