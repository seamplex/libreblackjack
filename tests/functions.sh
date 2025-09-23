#!/bin/false

# marker
functions_found=yes

# see how to invoke blackjack
blackjack=""
for i in  ../blackjack ../src/blackjack ./blackjack ./src/blackjack ../../../bin/blackjack /usr/local/bin/blackjack; do
  if [ -z "${blackjack}" ]; then
    if [ -f ${i} ]; then
      blackjack=${i}
    fi
  fi 
done
if [ -z "${blackjack}" ]; then
  echo "error: could not find blackjack executable"
  exit 1
fi

# see where the inputs are
for i in  . tests; do
  if [ -f ${i}/functions.sh ]; then
    dir=${i}
  fi
done


#######

exitifwrong() {
  if [ $1 != 0 ]; then
    exit $1
  fi
}

# checks if yq executable is available in the path
checkyq() {
 if [ -z "$(which yq)" ]; then
  echo "yq not found, skipping test"
  exit 77
 fi
}

checkgawk() {
 if [ -z "$(which gawk)" ]; then
  echo "gawk not found, skipping test"
  exit 77
 fi
}

