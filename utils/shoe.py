import random

def generate_card_list(num_decks=1):
    suits = ['H', 'D', 'C', 'S']  # Hearts, Diamonds, Clubs, Spades
    ranks = ['A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K']

    # Create one deck
    single_deck = [f"{rank}{suit}" for suit in suits for rank in ranks]

    # Multiply by number of decks and shuffle
    full_deck = single_deck * num_decks
    random.shuffle(full_deck)

    # Print each card on a new line
    for card in full_deck:
        print(card)

# Example usage: generate 2 decks
generate_card_list(num_decks=2)
