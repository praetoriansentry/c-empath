BIN := sentiment
all:
	gcc -g -o $(BIN) trie/trie.c sentiment.c
clean:
	$(RM) $(BIN)
