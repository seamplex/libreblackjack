define(case_title, Internal player)
---
title: case_title
...

# case_title

> Difficulty: case_difficulty/100

If `blackjack` is called with the `-i` option, it uses an _internal_ player to play against itself. By default it plays basic strategy, although it can read a text file with the strategy. Run 

```terminal
include(run.sh)dnl
```

and you will get the following report with the results of playing one million hands with basic strategy.

```yaml
include(report.yaml)
```

 * rules can be changed in the conf
 * the strategy can be read from a txt file
 * the yaml goes to stderr by default but it can be written to a file or post-proccesed with `yq`

case_nav
