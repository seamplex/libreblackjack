if [ -z "$(which pandoc)" ]; then 
 echo "error: pandoc not installed"
 exit 1
fi


# rm -f players.md
# for i in $(cat players); do
#   echo $i
# #   grep 'define(case_title' ../players/${i}/README.m4 >> players.md
# #   awk '/> Difficulty.*/{t=1}t' ../players/${i}/README.m4 | grep -v Difficulty >> players.md
# 
# #   sed 's/title:/title_case:/' ../players/${i}/README.md |\
# #     grep -v ':::' |\
# #     grep -v '> Difficulty:' |\
# #     grep -v 'Index' |\
# #     grep -vw '\-\-\-\-\-\-\-' |\
# #     pandoc --shift-heading-level-by=1 --markdown-headings=atx -f markdown+pipe_tables -t markdown+pipe_tables >> players.md
# 
# #   sed 's/title:/title_case:/' ../players/${i}/README.md |\
#   grep -wv 'title:' ../players/${i}/README.md |\
#     grep -v '> Difficulty:' |\
#     grep -v 'Index' |\
#     grep -ve ':::' |\
#     grep -vw -E '^\-\-\-$' |\
#     grep -vw -E '^\-\-\-\-\-\-\-$' |\
#     grep -vw '\.\.\.' |\
#     sed 's/\$n\$/Hands needed/' |\
#     sed 's/#/##/' >> players.md
#     
#   echo >> players.md
#   echo >> players.md
# done

./help.sh > help.md
# ./reference.sh bjinit va > conf.md

# ./reference.sh dealer ig > input-general.md
# ./reference.sh dealer ip > input-particular.md

pandoc help.md -t plain > help.txt

pandoc --toc --template template.texi blackjack.md -o blackjack.texi

# m4 header.m4 blackjack.md > tmp.md
# pandoc tmp.md --toc --template template.texi -o blackjack.texi
