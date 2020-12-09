#!/bin/bash

if [ -z "$(which gawk)" ]; then
  echo "error: gawk is not installed"
  exit 1
fi      

# get into the correct PWD
if [ ! -f ./header.m4 ]; then
  cd players
fi
if [ ! -f ./header.m4 ]; then
  echo "error: call me from the players directory"
  exit 1
fi

# ordered cases
cases="00-internal 02-always-stand 05-no-bust 08-mimic-the-dealer"

# fancy names
declare -A names=(\
["00-internal"]="Internal player" \
["02-always-stand"]="Always stand" \
["05-no-bust"]="No-bust strategy" \
["08-mimic-the-dealer"]="Mimic the dealer")

# expected results
declare -A expected=(\
["00-internal"]="-0.0065" \
["02-always-stand"]="-0.150" \
["05-no-bust"]="-0.075" \
["08-mimic-the-dealer"]="-0.055")

# allowed deviation
sigma=0.02

# if there is something on the commandline, use that one, otherwise the expected keys
if test ! -z "$1"; then
  player=$(basename $1)     # remove trailing '/'
else
  player=${cases}
fi

echo 'Case                    | Expected      | Result         | Error         |  Status ' | tee check.md
echo '------------------------|---------------|--------------- |---------------|---------' | tee -a check.md
for i in ${player}; do
  if test -d ${i} -a -x ${i}/run.sh; then
    cd ${i}
    if test $(echo "${names[${i}]}" | wc -c) -gt 16 ; then
      sep="\\t"
    else
      sep="\\t\\t"
    fi  
    echo -ne ${names[${i}]}${sep}'| '${expected["${i}"]}\\t'| '
    ./run.sh 1>/dev/null 2> ../${i}.yaml
    
    return=$(grep mean ../${i}.yaml | awk '{print $2}')
    error=$(grep error ../${i}.yaml | awk '{print $2}')
    
    echo -ne ${return}\\t' | '${error}\\t' |   '
    status=$(echo ${return} | awk -v e=${expected["${i}"]} -v s=${sigma} 'END {ok = ($1>(e-s)&&$1<(e+s)); print ok?"ok":"failed"; exit !ok }')
    result=$?
    cd ..
    echo ${status}
    echo -e "${names[${i}]}${sep}| ${expected["${i}"]}\\t| ${return}\\t | ${error}\\t | ${status}" >> check.md
  else
#     echo "skipped ${i}"
    result=77
  fi
done
