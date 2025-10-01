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

kws=$(grep "^///${tag}+" ${src} | awk '{print $1}' | awk -F+ '{print $2}' | sort | uniq)
# no sorting, use the order in the file
# kws=$(grep "^///${tag}+" ${src} | awk '{print $1}' | awk -F+ '{print $2}' | uniq)

echo

for kw in ${kws}; do
  plain_kw=$(echo ${kw} | tr -d ?)
  echo "* \`${kw}\` (@sec:${plain_kw})"
done

echo
echo

for kw in ${kws}; do

  usage=$(grep "///${tag}+${kw}+usage" ${src} | cut -d" " -f2-)
  plain_kw=$(echo ${kw} | tr -d ?)

  echo "# ${usage} {#sec:${plain_kw}}"
  echo

  grep "^///${tag}+${kw}+detail" ${src} | cut -d" " -f2- | sed 's/@$//' 
  echo

  examples=$(grep "^///${tag}+${kw}+default" ${src} | wc -l)
  if [ $examples -ne 0 ]; then
#     echo "## Default {-}"
#     echo
    echo "**Default**"
    grep "^///${tag}+${kw}+default" ${src} | cut -d" " -f2- | sed 's/@$//' 
    echo
  fi

  examples=$(grep "^///${tag}+${kw}+example" ${src} | wc -l)
  if [ $examples -ne 0 ]; then
#     echo "## Examples {-}"
    echo "**Examples**"
    echo
    echo "~~~"
    grep "^///${tag}+${kw}+example" ${src} | cut -d" " -f2- | sed 's/@$//' 
    echo "~~~"
    echo
  fi

done

echo 
