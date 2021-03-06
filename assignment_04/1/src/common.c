#include "common.h"

Message* messageHead = NULL;
struct termios orig_termios;

Message *new_message(char*from, char*message) {
    Message *m = malloc(sizeof(Message));
    m->from = malloc(sizeof(char) * strlen(from)+1);
    strcpy(m->from, from);
    m->message = malloc(sizeof(char) * strlen(message)+1);
    strcpy(m->message, message);
    m->time = (unsigned long)time(NULL);
    return m;

}
void add_message(char* from, char* message) {
    Message* m = new_message(from, message);
    if ( messageHead == NULL ) {
        messageHead = m;
        return;
    }
    Message *t = messageHead;
    while ( t != NULL && t->next != NULL) {
        t = t->next;
    }
    t->next = m;
}

void reset_terminal() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void setup_terminal() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(reset_terminal);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void print_message(Message* m) {
    if ( strcmp("server", m->from) == 0) {
        printf("[\x1b[1;33m%lu\x1b[m]",m->time);
        printf("[\x1b[1;35m%s\x1b[m]",m->from);
        printf("\x1b[3m %s\x1b[m\n", m->message);
        return;
    }
    printf("[\x1b[1;31m%lu\x1b[m]",m->time);
    printf("[\x1b[1;34m%s\x1b[m]",m->from);
    printf(" %s\n", m->message);
}


