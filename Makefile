BIN := sentiment
all:
	gcc -o $(BIN) sentiment.c
clean:
	$(RM) $(BIN)
