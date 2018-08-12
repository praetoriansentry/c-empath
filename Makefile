BIN := sentiment
all:
	gcc -g -o $(BIN) -Wall -Wextra -Wpedantic -std=c11 trie/trie.c sentiment.c
clean:
	$(RM) $(BIN)
