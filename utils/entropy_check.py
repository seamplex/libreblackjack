from collections import Counter
from math import log2

def shannon_entropy(card_list):
    counts = Counter(card_list)
    total = len(card_list)
    return -sum((count/total) * log2(count/total) for count in counts.values())
