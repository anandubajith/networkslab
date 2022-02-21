CC=gcc
BIN=./bin
SOURCE=./src/
CFLAGS=-Wall -Wextra -g

INC=src/graph.c

LIST=$(BIN)/lsr

all: $(LIST)

$(BIN)/%: $(SOURCE)%.c
	mkdir -p $(BIN)
	$(CC) $(INC) $< $(CFLAGS) -o $@ $(LIBS)

clean:
	rm -rf $(BIN)

