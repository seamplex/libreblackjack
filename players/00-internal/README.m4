define(case_title, Internal player)
---
title: case_title
...

# case_title

> Difficulty: case_difficulty/100

If `blackjack` is called with the `-i` option, it uses an _internal_ player to play against itself. By default it plays basic strategy. Run 

```terminal
include(run.sh)dnl
```

and you will get the following report with the results of playing one million hands with basic strategy.

```yaml
include(report.yaml)
```

case_nav
