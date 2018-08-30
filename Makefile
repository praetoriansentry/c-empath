BIN := sentiment
all:
	gcc -g -o $(BIN) -Wall -Wextra -Wpedantic -Wvla -std=c11 sentiment.c
clean:
	$(RM) $(BIN)

format:
	clang-format -i -style="{BasedOnStyle: llvm, IndentWidth: 4}" sentiment.c

test:
	cat data/test-text.txt | ./$(BIN) -c data/test-cats.tsv

