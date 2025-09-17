#!/usr/bin/env python3
import sys
from card_frequency import frequency_test
from runs_test import runs_test
from serial_corr import serial_correlation
from entropy_check import shannon_entropy

def load_cards(path):
    with open(path) as f:
        return [line.strip() for line in f if line.strip()]

def main():
    if len(sys.argv) != 2:
        print("Usage: python analyze_sequence.py path/to/cards.txt")
        sys.exit(1)

    cards = load_cards(sys.argv[1])
    print(f"Analyzing {len(cards)} cards...\n")

    chi2, p = frequency_test(cards)
    print(f"Chi-squared test: χ² = {chi2:.2f}, p = {p:.4f}")

    z = runs_test(cards)
    print(f"Runs test (red/black): z = {z:.2f}")

    corr = serial_correlation(cards)
    print(f"Serial correlation (ranks): r = {corr:.4f}")

    entropy = shannon_entropy(cards)
    print(f"Shannon entropy: H = {entropy:.4f} bits")

if __name__ == "__main__":
    main()
