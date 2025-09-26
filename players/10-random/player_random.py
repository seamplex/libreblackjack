import sys
import random

def main():
    cards_file = open("cards.txt", "w")
    log_file = open("log.txt", "w")
    while True:
        line = sys.stdin.readline()
        log_file.write(line)
        
        if not line:
            break
        line = line.strip()
        
        if line.startswith("card"):
            parts = line.split()
            if len(parts) >= 2:
                cards_file.write(parts[1] + "\n")
        if line.startswith("play?"):
            move = random.choice(["hit", "stand"])
            print(move, flush=True)
        elif line.endswith("bet?"):
            print("1", flush=True)
        elif line.endswith("?"):
            print("no", flush=True)
        
    cards_file.close()

if __name__ == "__main__":
    main()
