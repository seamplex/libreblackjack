#!/bin/bash

for i in grep awk sort uniq; do
 if [ -z "$(which $i)" ]; then
  echo "error: ${i} not installed"
  exit 1
 fi
done


src=${1}
tag=${2}

if [ -z "${tag}" ]; then
  echo "usage: $0 src tag";
  echo "example: $0 ../src/players/stdinout.cpp inf"
  exit 0
fi

# kws=$(grep "^///${tag}+" ${src} | awk '{print $1}' | awk -F+ '{print $2}' | sort | uniq)
# no sorting, use the order in the file
kws=$(grep "^///${tag}+" ${src} | awk '{print $1}' | awk -F+ '{print $2}' | uniq)

echo

for kw in ${kws}; do
  echo "* \`${kw}\` (@sec:${kw})"
done

echo
echo

for kw in ${kws}; do

#  * `new_hand` (@sec:new_hand)
# 
# #### `new_hand` $n$ $b$ {#sec:new_hand}
# 
# The dealer states that a new hand is starting. The integer $n$ states the number of the hand (starting from 1).
# The decimal number $b$ states the player's bankroll before placing the bet in the starting hand.
# 
#     
# **Examples**
#     
# ```
# new_hand 1 0.000000
# new_hand 22 -8.000000
# new_hand 24998 -7609.500000
# ```

  usage=$(grep "///${tag}+${kw}+usage" ${src} | cut -d" " -f2-)


  #### 
  echo "#### ${usage} {#sec:${kw}}"
  echo
  
  grep "^///${tag}+${kw}+detail" ${src} | cut -d" " -f2- | sed 's/@$//' 
  echo
  
  echo "**Examples**"
  echo
  echo "~~~"
  grep "^///${tag}+${kw}+example" ${src} | cut -d" " -f2- | sed 's/@$//' 
  echo "~~~"
  echo
  
  
done

echo 
