BIN := sentiment

all:
	gcc -g -o $(BIN) -Wall -Wextra -Wpedantic -Wvla -std=c11 sentiment.c
clean:
	$(RM) $(BIN)

format:
	clang-format -i -style="{BasedOnStyle: llvm, IndentWidth: 4}" sentiment.c

test:
	cat data/test-text.txt | ./$(BIN) -c data/test-cats.tsv

books:
	mkdir books_temp
	cd books_temp && wget 'https://www.gutenberg.org/browse/scores/top'
	cd books_temp && cat top | grep '<li><a href="/ebooks' | sed 's#^.*href="\(.*\)".*$$#https://www.gutenberg.org\1#g' | sort | uniq | xargs wget
	mkdir books
	cd books && cat ../books_temp/* | grep 'Plain Text UTF-8' | sed 's#^.*a href="\([^"]*\)".*$$#https:\1#g' | sort | uniq | xargs wget
	$(RM) -rf books_temp

install: all
	mkdir -p $(HOME)/.local/bin
	cp $(BIN) $(HOME)/.local/bin 
