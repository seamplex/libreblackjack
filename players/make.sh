#!/bin/bash

declare -A prev
declare -A next

current_prev=""
i=1
for dir in *; do
 if [ -e ${dir}/README.m4 ]; then
  cases[${i}]=$dir
  next[$((${i} - 1))]=$dir
  prev[${i}]=$current_prev
  current_prev=${dir}
  i=$((${i} + 1))
 fi
done

for i in $(seq 1 ${#cases[*]}); do
 echo $i ${cases[${i}]}
 cd ${cases[${i}]}
 difficulty=`echo ${cases[${i}]} | cut -c-2`
 m4 -Dcase_difficulty=${difficulty} \
    -Dcase_prev=${prev[${i}]} \
    -Dcase_next=${next[${i}]} \
    ../header.m4 README.m4 > README.md
 cd ..   
done
