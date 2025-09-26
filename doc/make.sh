
./reference.sh ../src/players/stdinout.cpp inf > commands-inf.md




echo "help as markdown definition list"
./help-md.sh > help.md
pandoc help.md -t plain > help.txt

echo "help as a raw txt (which can be used in blackjack -h)"
./help-txt.sh usage   > help-usage.txt
./help-options-txt.sh > help-options-base.txt
./help-txt.sh extra   > help-extra.txt

./md2.sh --texi blackjack.md
mv blackjack.texi libreblackjack.texi

./md2.sh --pdf  blackjack
./md2.sh --html blackjack
