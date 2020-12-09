if [ -z "$(which pandoc)" ]; then 
 echo "error: pandoc not installed"
 exit 1
fi


rm -f players.md
for i in $(cat players); do
  echo $i
#   grep 'define(case_title' ../players/${i}/README.m4 >> players.md
#   awk '/> Difficulty.*/{t=1}t' ../players/${i}/README.m4 | grep -v Difficulty >> players.md
  sed 's/title:/title_case:/' ../players/${i}/README.md |\
    grep -v ':::' |\
    grep -v '> Difficulty:' |\
    grep -v 'Index' |\
    grep -vw '\-\-\-\-\-\-\-' |\
    pandoc --shift-heading-level-by=1 --markdown-headings=atx -t markdown >> players.md
  echo >> players.md
  echo >> players.md
done

./help.sh > help.md
# ./reference.sh bjinit va > conf.md

# ./reference.sh dealer ig > input-general.md
# ./reference.sh dealer ip > input-particular.md

pandoc help.md -t plain > help.txt

m4 ../version.m4 header.m4 blackjack.md | pandoc --toc --template template.texi -o blackjack.texi

# m4 header.m4 blackjack.md > tmp.md
# pandoc tmp.md --toc --template template.texi -o blackjack.texi
