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
	rm -rf $(BIN) stats.dat output file.bin

dummy:
	seq 10000000 > file.bin
	truncate --size=50M file.bin

demo:
	tmux new-session \; \
		send-keys './bin/server' C-m \; \
		split-window -h \; \
		send-keys './bin/client'

iptable:
	sudo iptables --append INPUT --protocol udp --src 127.0.0.1 --sport 12356 --dst 127.0.0.1 --dport 12357 -m statistic --mode random --probability 0.10 --jump DROP

remove-iptable:
	sudo iptables -D INPUT --protocol udp --src 127.0.0.1 --sport 12356 --dst 127.0.0.1 --dport 12357 -m statistic --mode random --probability 0.10 --jump DROP

graph:
	gnuplot plot.plt
