#!/bin/bash

gcc -Wall -Wextra -g mailclient.c -o mailclient
gcc -Wall -Wextra -g popserver.c -o popserver
gcc -Wall -Wextra -g smtpmail.c -o smtpmail


tmux new-session \; \
    send-keys './smtpmail 2525' C-m \; \
    split-window -h \; \
    send-keys './popserver 1110' C-m \; \
    split-window -h \; \
    send-keys './mailclient 2525 1110' C-m \; \
    select-layout even-horizontal


