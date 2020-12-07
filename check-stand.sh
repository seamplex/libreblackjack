if [ ! -z "`which awk`" ]; then
  yes stand | ./blackjack -n5e5 --flat_bet=true --no_insurance=true 2>&1 | grep mean | awk '{e=-0.15;s=1e-2} END {ok = ($2>(e-s)&&$2<(e+s)); print ok?"ok":"failed"; exit !ok }'
  result=$?
else
  result=77
fi
