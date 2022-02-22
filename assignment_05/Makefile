CC=gcc
BIN=./bin
SOURCE=./src/
CFLAGS=-Wall -Wextra -g

INC=src/graph.c

LIST=$(BIN)/lsr $(BIN)/dvr

all: $(LIST)

$(BIN)/%: $(SOURCE)%.c
	mkdir -p $(BIN)
	$(CC) $(INC) $< $(CFLAGS) -o $@ $(LIBS)

clean:
	rm -rf $(BIN)

test: all
	./bin/$(ALG) < ./test/$(FILE).txt
