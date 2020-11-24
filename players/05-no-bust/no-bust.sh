#!/bin/sh

i=0
while read command
do
  i=$((i+1))
  if test ${i} -ge 12345; then
    echo "quit"
  elif test "${command}" = 'bye'; then
    exit
  elif test "${command}" = 'bet?'; then
    echo 1  
  elif test "${command}" = 'insurance?'; then
    echo "no"
  elif test "$(echo ${command} | cut -c-5)" = 'play?'; then
    count=$(echo ${command} | cut -f2 -d" ")
    if test ${count} -lt 12; then
      echo "hit"
    else
      echo "stand"
    fi
  fi
done
