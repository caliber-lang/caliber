CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
SRC = src/main.c src/lexer.c src/utils.c
OBJ = $(SRC:.c=.o)
BIN = suic

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $

test: $(BIN)
	./$(BIN) test/test.sui

clean:
	rm -f $(OBJ) $(BIN)

.PHONY: all test clean
