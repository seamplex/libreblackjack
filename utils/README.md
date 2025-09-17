# libreblackjack/utils

This directory contains utility scripts for analyzing card sequences and simulating draws, especially useful for testing randomness in blackjack games and shuffling algorithms.

## Contents

- `csm_draw.py`: Simulate draws from a Continuous Shuffling Machine (CSM).
- `card_frequency.py`: Perform a chi-squared test on card frequency.
- `runs_test.py`: Detect clustering or streaks using the Wald–Wolfowitz runs test.
- `serial_corr.py`: Measure serial correlation between consecutive card ranks.
- `entropy_check.py`: Compute Shannon entropy of a card sequence.
- `analyze_sequence.py`: CLI tool to run all statistical tests on a given card list.

## Usage

Each script can be run standalone or imported as a module. To analyze a card sequence:

```bash
python utils/analyze_sequence.py path/to/cards.txt
```
