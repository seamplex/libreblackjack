import numpy as np

def serial_correlation(card_list):
    rank_map = {'A':1, '2':2, '3':3, '4':4, '5':5, '6':6,
                '7':7, '8':8, '9':9, 'T':10, 'J':11, 'Q':12, 'K':13}
    ranks = [rank_map[card[0]] for card in card_list]
    return np.corrcoef(ranks[:-1], ranks[1:])[0, 1]
