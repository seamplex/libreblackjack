if [ ! -z "$(which awk)" ]; then
  ./blackjack -i 2>&1 | grep mean | awk '{e=-7e-3;s=5e-3} END {ok = ($2>(e-s)&&$2<(e+s)); print ok?"ok":"failed"; exit !ok }'
  result=$?
else
  result=77
fi
