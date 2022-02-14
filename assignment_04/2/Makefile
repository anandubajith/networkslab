CC=gcc
BIN=./bin
SOURCE=./src/
CFLAGS=-g -lpthread

INC=

LIST=$(BIN)/server $(BIN)/client

all: $(LIST)

$(BIN)/%: $(SOURCE)%.c
	mkdir -p $(BIN)
	$(CC) $(INC) $< $(CFLAGS) -o $@ $(LIBS)

clean:
	rm -rf $(BIN)

