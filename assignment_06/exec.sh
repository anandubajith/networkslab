#!/bin/bash


gcc -Wall -Wextra -g -lpthread FTAPClient/FTAPClient.c -o FTAPClient/FTAPClient
gcc -Wall -Wextra -g -lpthread FTAPServer/FTAPServer.c -o FTAPServer/FTAPServer


tmux new-session \; \
    send-keys 'cd FTAPServer && ./FTAPServer' C-m \; \
    split-window -h \; \
    send-keys 'cd FTAPClient && ./FTAPClient' C-m \;

