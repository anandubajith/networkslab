#!/bin/bash


gcc -Wall -Wextra -g FTAPClient/FTAPClient.c -o FTAPClient/FTAPClient
gcc -Wall -Wextra -g FTAPServer/FTAPServer.c -o FTAPServer/FTAPServer


tmux new-session \; \
    send-keys 'cd FTAPServer && ./FTAPServer' C-m \; \
    split-window -h \; \
    send-keys './FTAPClient/FTAPClient' C-m \;

