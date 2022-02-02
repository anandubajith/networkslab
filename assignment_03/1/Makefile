CC=gcc
BIN=./bin
SOURCE=./src/
CFLAGS=-g

INC=src/fruits.c

LIST=$(BIN)/tcp_server $(BIN)/tcp_client $(BIN)/udp_server $(BIN)/udp_client



all: $(LIST)

$(BIN)/%: $(SOURCE)%.c
	mkdir -p $(BIN)
	$(CC) $(INC) $< $(CFLAGS) -o $@ $(LIBS)

clean:
	rm -rf $(BIN)

