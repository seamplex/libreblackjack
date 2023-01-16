#!/bin/bash

   n0=80000
n_max=9000000

RED="\033[0;31m"
GREEN="\033[0;32m"

BROWN="\033[0;33m"
MAGENTA="\e[0;35m"
CYAN="\e[0;36m"

NC="\033[0m" # No Color

for i in grep awk printf blackjack; do
 if [ -z "$(which $i)" ]; then
  echo "error: $i not installed"
  exit 1
 fi
done

debug=0

declare -A strategy
declare -A ev

declare -A min
min["hard"]=4   # from 20 to 4 in hards
min["soft"]=12  # from 20 to 12 in softs

rm -f table.md hard.html soft.html pair.html

# --------------------------------------------------------------
# start with standing
cp hard-stand.txt hard.txt
cp soft-stand.txt soft.txt

cat << EOF >> table.md
|  Hand  |  \$n\$  |  Stand [%]  |  Double [%]  |  Hit [%] |   Play    |
|:------:|:-----:|:-----------:|:------------:|:--------:|:---------:|
EOF


for type in hard soft; do
 for hand in $(seq 20 -1 ${min[${type}]}); do
 
  # choose two random cards that make up the player's assumed total
  if [ ${type} = "hard" ]; then
   t="h"
   card1=11
   card2=11
   while test $card1 -gt 10 -o $card2 -gt 10; do
    card1=$((${RANDOM} % (${hand}-3) + 2))
    card2=$((${hand} - ${card1}))
   done
  elif [ ${type} = "soft" ]; then
   t="s"
   # one card is an ace
   card1=1
   card2=$((${hand} - 10 - ${card1}))
  fi

  cat << EOF >> ${type}.html
 <tr>
  <td>${t}${hand}</td>
  <td>
   <div class="text-right">s<span class="d-none d-lg-inline">tand</span></div>
   <div class="text-right">h<span class="d-none d-lg-inline">it</span></div>
   <div class="text-right">d<span class="d-none d-lg-inline">ouble</span></div>
  </td>
EOF
  
  for upcard in $(seq 2 9) T A; do
  
   if [ "x$upcard" = "xT" ]; then
     upcard_n=10
   elif [ "x$upcard" = "xA" ]; then
     upcard_n=1
   else
     upcard_n=$(($upcard))
   fi
 
   n=${n0}   # start with n0 hands
   best="x"  # x means don't know what to so, so play
   
   while [ "${best}" = "x" ]; do
    # tell the user which combination we are trying and how many we will play
    echo -ne "${t}${hand}-${upcard} ($card1 $card2)\t"$(printf %.1e ${n})
    for play in s d h; do
     
     # start with options.conf as a template and add some custom stuff
     cp options.conf blackjack.conf
     cat << EOF >> blackjack.conf
hands = ${n}
player = basic
arranged_cards = ${card1}, $((${upcard_n} + 13)), $((${card2} + 26))
report = ${t}${hand}-${upcard}-${play}.yaml
#log = ${t}${hand}-${upcard}-${play}.log
EOF
 
     # read the current strategy
     while read w p2 p3 p4 p5 p6 p7 p8 p9 pT pA; do
      # w already has the "h" or the "s"
      strategy[${w},2]=$p2   
      strategy[${w},3]=$p3
      strategy[${w},4]=$p4    
      strategy[${w},5]=$p5    
      strategy[${w},6]=$p6    
      strategy[${w},7]=$p7    
      strategy[${w},8]=$p8    
      strategy[${w},9]=$p9    
      strategy[${w},T]=$pT    
      strategy[${w},A]=$pA    
     done < ${type}.txt
     
     # override the read strategy with the explicit play: s, d or h
     strategy[${t}${hand},${upcard}]=${play}
     
     # save the new (temporary) strategy
     rm -f ${type}.txt
     for h in $(seq 20 -1 ${min[${type}]}); do
      echo -n "${t}${h}  " >> ${type}.txt
      
      # extra space if h < 10
      if [ ${h} -lt 10 ]; then
       echo -n " " >> ${type}.txt
      fi 
      
      for u in $(seq 2 9) T A; do
       echo -n "${strategy[${t}${h},${u}]}  " >> ${type}.txt
      done
      echo >> ${type}.txt
     done

     # debug, comment for production
     if [ "${debug}" != "0" ]; then
      cp ${type}.txt ${t}${hand}-${upcard}-${play}.str
     fi
    
     # ensamble the full bs.txt with no pairing
     cat hard.txt soft.txt pair-no.txt > bs.txt
     
     # play!
     blackjack
    
     # evaluate the results
     ev[${t}${hand},${upcard},${play}]=$(grep mean ${t}${hand}-${upcard}-${play}.yaml | awk '{printf("%g", $2)}')
     error[${t}${hand},${upcard},${play}]=$(grep error ${t}${hand}-${upcard}-${play}.yaml | awk '{printf("%g", $2)}')
     
    done
   
    # choose the best one
    ev_s=$(echo ${ev[${t}${hand},${upcard},s]} | awk '{printf("%+.2f", 100*$1)}')
    ev_d=$(echo ${ev[${t}${hand},${upcard},d]} | awk '{printf("%+.2f", 100*$1)}')
    ev_h=$(echo ${ev[${t}${hand},${upcard},h]} | awk '{printf("%+.2f", 100*$1)}')
   
    
    if [ ${n} -le ${n_max} ]; then 
     # if we still have room, take into account errors
     error_s=$(echo ${error[${t}${hand},${upcard},s]} | awk '{printf("%.1f", 100*$1)}')
     error_d=$(echo ${error[${t}${hand},${upcard},d]} | awk '{printf("%.1f", 100*$1)}')
     error_h=$(echo ${error[${t}${hand},${upcard},h]} | awk '{printf("%.1f", 100*$1)}')
    else
     # instead of running infinite hands, above a threshold asume errors are zero
     error_s=0
     error_d=0
     error_h=0
    fi  
 
    echo -ne "\t${ev_s}\t(${error_s})"
    echo -ne "\t${ev_d}\t(${error_d})"
    echo -ne "\t${ev_h}\t(${error_h})"
   
    if   (( $(echo ${ev_s} ${error_s} ${ev_d} ${error_d} | awk '{print (($1-$2) > ($3+$4))}') )) &&
         (( $(echo ${ev_s} ${error_s} ${ev_h} ${error_h} | awk '{print (($1-$2) > ($3+$4))}') )); then
         
     best="s"
     color=${BROWN}
     best_string="stand"
     
    elif (( $(echo ${ev_d} ${error_d} ${ev_s} ${error_s} | awk '{print (($1-$2) > ($3+$4))}') )) &&
         (( $(echo ${ev_d} ${error_d} ${ev_h} ${error_h} | awk '{print (($1-$2) > ($3+$4))}') )); then
         
     best="d"
     color=${CYAN}
     best_string="double"
     
    elif (( $(echo ${ev_h}-${error_h} ${ev_s} ${error_s} | awk '{print (($1-$2) > ($3+$4))}') )) &&
         (( $(echo ${ev_h}-${error_h} ${ev_d} ${error_d} | awk '{print (($1-$2) > ($3+$4))}') )); then
         
     best="h"
     color=${MAGENTA}
     best_string="hit"
         
    else
    
     best="x"
     color=${NC}
     best_string="uncertain"
     
     n=$((${n} * 4))
     
    fi
    
    echo -e ${color}"\t"${best_string}${NC}
    
   done

   strategy[${t}${hand},${upcard}]=${best}
   
   
   
   echo "| ${t}${hand}-${upcard} | $(printf %.1e ${n}) | ${ev_s} (${error_s}) | ${ev_h} (${error_h}) | ${ev_d} (${error_d}) | ${best_string} | " >> table.md
    
   echo " <!-- ${upcard} -->" >> ${type}.html
   echo " <td>" >> ${type}.html
   echo ${ev_s} ${error_s} | awk -f html_cell.awk >> ${type}.html
   echo ${ev_h} ${error_h} | awk -f html_cell.awk >> ${type}.html
   echo ${ev_d} ${error_d} | awk -f html_cell.awk >> ${type}.html
   echo " </td>" >> ${type}.html
   
   
   # save the strategy again with the best strategy
   rm -f ${type}.txt
   for h in $(seq 20 -1 ${min[${type}]}); do
    echo -n "${t}${h}  " >> ${type}.txt
    
    # extra space if h < 10
    if [ ${h} -lt 10 ]; then
     echo -n " " >> ${type}.txt
    fi 
    
    for u in $(seq 2 9) T A; do
     echo -n "${strategy[${t}${h},${u}]}  " >> ${type}.txt
    done
    
    echo >> ${type}.txt
    
   done
  done
  
#   echo "</tr>" >> ${type}.html
  
 done
done


cat << EOF >> table.md


|  Hand  |  \$n\$  |   Yes [%]  |   No [%]   |
|:------:|:-------:|:----------:|:----------:|
EOF

# --------------------------------------------------------------------
# pairs
type="pair"
t="p"
cp pair-no.txt pair.txt

for hand in A T $(seq 9 -1 2); do
 if [ "${hand}" = "A" ]; then
  pair=1
 elif [ "${hand}" = "T" ]; then
  pair=10
 else
  pair=$((${hand}))
 fi
  
#  cat << EOF >> ${type}.html
#  <tr>
#   <td>${t}${hand}</td>
#   <td>
#    <div class="text-right">y<span class="d-none d-lg-inline">es</span></div>
#    <div class="text-right">n<span class="d-none d-lg-inline">o</span></div>
#   </td>
# EOF
  
 for upcard in $(seq 2 9) T A; do
  if [ "$upcard" = "T" ]; then
    upcard_n=10
  elif [ "$upcard" = "A" ]; then
    upcard_n=1
  else
    upcard_n=$(($upcard))
  fi
 
  n=${n0}   # start with n0 hands
  best="x"  # x means don't know what to so, so play
   
  while [ "${best}" = "x" ]; do
   # tell the user which combination we are trying and how many we will play
   echo -ne "${t}${hand}-${upcard}\t\t$(printf %.0e ${n})"
   
   for play in y n; do
    
    # start with options.conf as a template and add some custom stuff
    cp options.conf blackjack.conf
    cat << EOF >> blackjack.conf
hands = ${n}
player = basic
arranged_cards = ${pair}, $((${upcard_n} + 13)), $((${pair} + 26))
report = ${t}${hand}-${upcard}-${play}.yaml
# log = ${t}${hand}-${upcard}-${play}.log
EOF
 
    # read the current strategy
    while read w p2 p3 p4 p5 p6 p7 p8 p9 pT pA; do
     # w already has the "p"
     strategy[${w},2]=$p2   
     strategy[${w},3]=$p3
     strategy[${w},4]=$p4    
     strategy[${w},5]=$p5    
     strategy[${w},6]=$p6    
     strategy[${w},7]=$p7    
     strategy[${w},8]=$p8    
     strategy[${w},9]=$p9    
     strategy[${w},T]=$pT    
     strategy[${w},A]=$pA    
    done < ${type}.txt
     
    # override the read strategy with the explicit play: y or n
    strategy[${t}${hand},${upcard}]=${play}
     
    # save the new (temporary) strategy
    rm -f ${type}.txt
    for h in A T $(seq 9 -1 2); do
     echo -n "${t}${h}   " >> ${type}.txt
     for u in $(seq 2 9) T A; do
      echo -n "${strategy[${t}${h},${u}]}  " >> ${type}.txt
     done
     echo >> ${type}.txt
    done
     
    if [ "${debug}" != "0" ]; then
     cp ${type}.txt ${t}${hand}-${upcard}-${play}.str
    fi  
    
    # ensamble the full bs.txt
    cat hard.txt soft.txt pair.txt > bs.txt

    # play!
    blackjack

    # evaluate the results
    ev[${t}${hand},${upcard},${play}]=$(grep mean ${t}${hand}-${upcard}-${play}.yaml | awk '{printf("%g", $2)}')
    error[${t}${hand},${upcard},${play}]=$(grep error ${t}${hand}-${upcard}-${play}.yaml | awk '{printf("%g", $2)}')
    
   done
   
   # choose the best one
   ev_y=$(echo ${ev[${t}${hand},${upcard},y]} | awk '{printf("%+.2f", 100*$1)}')
   ev_n=$(echo ${ev[${t}${hand},${upcard},n]} | awk '{printf("%+.2f", 100*$1)}')
   
   if [ $n -le ${n_max} ]; then 
    # if we still have room, take into account errors
    error_y=$(echo ${error[${t}${hand},${upcard},y]} | awk '{printf("%.1f", 100*$1)}')
    error_n=$(echo ${error[${t}${hand},${upcard},n]} | awk '{printf("%.1f", 100*$1)}')
   else
    # instead of running infinite hands, above a threshold asume errors are zero
    error_y=0
    error_n=0
   fi  
 
   echo -ne "\t${ev_y}\t(${error_y})"
   echo -ne "\t${ev_n}\t(${error_n})"
   
   if   (( $(echo ${ev_y} ${error_y} ${ev_n} ${error_n} | awk '{print (($1-$2) > ($3+$4))}') )); then
   
    best="y"
    color=${GREEN}
    best_string="yes"
    
   elif (( $(echo ${ev_n} ${error_n} ${ev_y} ${error_y} | awk '{print (($1-$2) > ($3+$4))}') )); then
   
    best="n"
    color=${RED}
    best_string="no"
   
   else
   
    best="x"
    color=${NC}
    best_string="uncertain"
    
    n=$((${n} * 4))
    
   fi
   
   echo -e ${color}"\t"${best_string}${NC}
  done

  echo "| ${t}${hand}-${upcard} | $(printf %.1e ${n}) | ${ev_y} (${error_y}) | ${ev_n} (${error_n}) | ${best_string} | " >> table.md
  
  echo " <!-- ${upcard} -->" >> ${type}.html
  echo " <td>" >> ${type}.html
  echo ${ev_y} ${error_y} | awk -f html_cell.awk >> ${type}.html
  echo ${ev_n} ${error_n} | awk -f html_cell.awk >> ${type}.html
  echo " </td>" >> ${type}.html
  
  
  strategy[${t}${hand},${upcard}]=${best}
   
  # save the strategy again with the best strategy
  rm -f ${type}.txt
  for h in A T $(seq 9 -1 2); do
   echo -n "${t}${h}   " >> ${type}.txt
   for u in $(seq 2 9) T A; do
    echo -n "${strategy[${t}${h},${u}]}  " >> ${type}.txt
   done
   echo >> ${type}.txt
  done
 done
done

 
cat header.txt hard.txt header.txt soft.txt header.txt pair.txt > bs.txt
rm -f hard.txt soft.txt pair.txt blackjack.conf
if [ "${debug}" == "0" ]; then
 rm -f *.yaml
 rm -f *.str
 rm -f *.log
fi
 
