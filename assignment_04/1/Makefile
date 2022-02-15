CC=gcc
BIN=./bin
SOURCE=./src/
CFLAGS=-Wall -Wextra -g

INC=src/common.c

LIST=$(BIN)/server $(BIN)/client

all: $(LIST)

$(BIN)/%: $(SOURCE)%.c
	mkdir -p $(BIN)
	$(CC) $(INC) $< $(CFLAGS) -o $@ $(LIBS)

clean:
	rm -rf $(BIN)

demo: clean all
	tmux new-session \; \
		send-keys './bin/server' C-m \; \
		split-window -h \; \
		send-keys './bin/client' C-m \; \
		split-window -v \; \
		send-keys './bin/client' C-m \; \
		split-window -v \; \
		send-keys './bin/client'
