import random

def draw_from_csm(n):
    suits = ['H', 'D', 'C', 'S']
    ranks = ['A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K']
    all_cards = [f"{rank}{suit}" for suit in suits for rank in ranks]

    # Simulate n draws with replacement
    draws = [random.choice(all_cards) for _ in range(n)]
    return draws

# Example usage: draw 20 cards from a continuous shuffling machine
for card in draw_from_csm(20):
    print(card)
