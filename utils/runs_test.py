def runs_test(card_list):
    def is_red(card): return card[1] in ['H', 'D']
    binary_seq = [is_red(card) for card in card_list]

    runs = 1
    for i in range(1, len(binary_seq)):
        if binary_seq[i] != binary_seq[i-1]:
            runs += 1

    n1 = binary_seq.count(True)
    n2 = binary_seq.count(False)
    expected_runs = ((2 * n1 * n2) / (n1 + n2)) + 1
    std_dev = ((2 * n1 * n2 * (2 * n1 * n2 - n1 - n2)) /
               ((n1 + n2)**2 * (n1 + n2 - 1)))**0.5
    z = (runs - expected_runs) / std_dev
    return z
