#!/bin/bash

set -e


files=("mailclient" "popserver" "smtpmail")

clean() {
    echo "Removing compiled binaries"
    for binary in ${files[@]}; do
        rm -f $binary
    done
}

remove_folders() {
    echo "Removing user folders"
    awk '{print $1}' "userlogincred.txt" | xargs /bin/rm -rf
}

build() {
    echo "Building binaries"
    for binary in ${files[@]}; do
        gcc -Wall -Wextra -g "$binary.c" -o "$binary"
    done
}

demo() {
    tmux new-session \; \
        send-keys './smtpmail 2525' C-m \; \
        split-window -h \; \
        send-keys './popserver 1110' C-m \; \
        split-window -h \; \
        send-keys './mailclient 2525 1110' C-m \; \
        select-layout even-horizontal
}


# build
# demo


if [[ "$1" == "clean" ]]; then
    clean
elif [[ "$1" == "cleanall" ]]; then
    remove_folders
    clean
else
    build
    demo
fi
