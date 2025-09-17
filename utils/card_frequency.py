from collections import Counter
from scipy.stats import chisquare

def frequency_test(card_list):
    counts = Counter(card_list)
    observed = list(counts.values())
    expected = [len(card_list) / 52] * len(observed)
    chi2, p = chisquare(f_obs=observed, f_exp=expected)
    return chi2, p
