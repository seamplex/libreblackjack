#!/bin/sh
# 
# Execute this script to clean up the working directory
#
# This file is free software: you are free to change and redistribute it.
# There is NO WARRANTY, to the extent permitted by law.
#

if [ -e Makefile ]; then
  make clean
fi
# from https://stackoverflow.com/questions/13541615/how-to-remove-files-that-are-listed-in-the-gitignore-but-still-on-the-repositor
cat .gitignore | grep -v nbproject | sed '/^#.*/ d' | sed '/^\s*$/ d' | sed 's/^/rm -rf /' | bash

cd doc
cat .gitignore | grep -v nbproject | sed '/^#.*/ d' | sed '/^\s*$/ d' | sed 's/^/rm -rf /' | bash
cd ..

rm -f players/05-no-bust/fifo players/08-mimic-the-dealer/d2p players/08-mimic-the-dealer/p2d players/30-ace-five/f

# cd po
# cat .gitignore | grep -v nbproject | sed '/^#.*/ d' | sed '/^\s*$/ d' | sed 's/^/rm -rf /' | bash
# cd ..
