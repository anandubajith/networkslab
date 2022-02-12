CC=gcc
BIN=./bin
SOURCE=./src/
CFLAGS=-g

INC=src/tcp.c

LIST=$(BIN)/server $(BIN)/client

all: $(LIST)

$(BIN)/%: $(SOURCE)%.c
	mkdir -p $(BIN)
	$(CC) $(INC) $< $(CFLAGS) -o $@ $(LIBS)

clean:
	rm -rf $(BIN)

dummy:
	seq 10000000 > file.bin
	truncate --size=50M file.bin
