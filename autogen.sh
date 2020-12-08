#!/bin/sh
# 
# Execute this script to generate a configure script
#
# This file is free software: you are free to change and redistribute it.
# There is NO WARRANTY, to the extent permitted by law.
#

# check for needed tools
for i in git autoreconf make m4 fmt; do
  if test -z "$(which ${i})"; then
    echo "error: ${i} is not installed"
    exit 1
  fi
done

./autoclean.sh


if test -d ".git"; then
  vcs=git
else
  echo "this tree is not a repository (did you download instead of clone?)" 
  exit 1
fi

if test ${vcs} = "git"; then
 # major version is equal to the latest tag
 version=$(git describe --tags | sed 's/-/./')

 branch=$(git symbolic-ref HEAD | sed -e 's,.*/\(.*\),\1,')
 commitdate=$(git log -1 --pretty=format:"%ad")
 cat << EOF > src/version-vcs.h
#define LIBREBLACKJACK_VCS_BRANCH    "${branch}"
#define LIBREBLACKJACK_VCS_VERSION   "${version}"
#define LIBREBLACKJACK_VCS_DATE      "${commitdate}"
#define LIBREBLACKJACK_VCS_CLEAN     $(git status --porcelain | wc -l)
EOF

else
 version=0.1.0
fi

echo "define(libreblackjackversion, [${version}])dnl" > version.m4


if [ ! -z "$(which pandoc)" ]; then
  pandoc README.md -t plain -o README
#   cd doc
#   ./md2texi.sh
#   cd ..
else
  fmt README.md > README
  touch doc/libreblackjack.texi
fi

# touch ChangeLog
git log --oneline --decorate > ChangeLog

autoreconf --install
automake --add-missing
