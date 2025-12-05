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

n=2e6

# https://wizardofodds.com/games/blackjack/appendix/9/6dh17r4/
ref=-0.00654793    # from running 1e9 hands
d=6
echo "ahc ${d}decks h17 das nrsa ${n}"
$blackjack -i -p --report=ahc.yaml -n${n} --rules=h17 --shuffle_every_hand --decks=${d}
actual=$(yq .mean ahc.yaml)
tol=$(yq .error ahc.yaml)
echo $actual
echo $ref
echo " $tol"
awk -v a="$actual" -v r="$ref" -v t="$tol" 'BEGIN { exit !((a >= (r-t)) && (a <= (r+t))) }'
exitifwrong $?
echo "ok"


#https://wizardofodds.com/games/blackjack/appendix/9/1ds17r4/
# ref=0.001839
ref=0.0005

d=1
echo "ahc ${d}decks s17 das nrsa ${n}"
$blackjack -i -p --report=ahc.yaml -n${n} --rules=s17 --shuffle_every_hand --decks=${d}
actual=$(yq .mean ahc.yaml)
tol=$(yq .error ahc.yaml)
echo $actual
echo $ref
echo " $tol"
awk -v a="$actual" -v r="$ref" -v t="$tol" 'BEGIN { exit !((a >= (r-t)) && (a <= (r+t))) }'
exitifwrong $?
echo "ok"

# https://wizardofodds.com/games/blackjack/appendix/9/euro-6ds17r4/
# ref=-0.0051417
# TODO: update the strategy to match ENHC
ref=-0.0075
d=6
echo "enhc ${d}decks s17 das nrsa ${n}"
$blackjack -i -p --report=enhc.yaml -n${n} --rules="enhc s17" --shuffle_every_hand --decks=${d}
actual=$(yq .mean enhc.yaml)
tol=$(yq .error enhc.yaml)
echo $actual
echo $ref
echo " $tol"
awk -v a="$actual" -v r="$ref" -v t="$tol" 'BEGIN { exit !((a >= (r-t)) && (a <= (r+t))) }'
exitifwrong $?
echo "ok"
