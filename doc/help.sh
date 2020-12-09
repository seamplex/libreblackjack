#!/bin/bash

src="main"
tag="op"
dir="../src"

kws=`grep "///${tag}+" ${dir}/${src}.cpp | awk '{print $1}' | awk -F+ '{print $2}' | uniq` 

for kw in ${kws}; do

  # option como definition list
  grep "///${tag}+${kw}+option" ${dir}/${src}.cpp | cut -d" " -f2-
  echo  

  # detailed description
  # el cut saca los tags especiales
  # el segundo se es para poder poner links como https:/\/ (sin la barra del medio gcc piensa que es un comentario)
  echo -n ':    '
  grep "///${tag}+${kw}+desc" ${dir}/${src}.cpp | cut -d" " -f2- | sed 's_/\\/_//_'
  echo  
  echo  
  
done

echo 
