#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include<sys/types.h>
#include <termios.h>
#include <unistd.h>
#include<unistd.h>

typedef struct _message {
    char* from;
    char* message;
    unsigned long time;
    struct _message *next;
} Message;

Message* messageHead = NULL;

void add_message(char* from, char* message) {
    Message *m = malloc(sizeof(Message));
    m->from = malloc(sizeof(char) * 100);
    strcpy(m->from, from);
    m->message = malloc(sizeof(char) * 100);
    strcpy(m->message, message);
    m->time = (unsigned long)time(NULL);
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

struct termios orig_termios;
char nick[100];
char command[100];
int pos = 0;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
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

void draw() {
    printf("\x1b[2J");
    printf("\x1b[?25l");
    printf("\x1b[H");
    Message *t = messageHead;
    while ( t != NULL ) {
        print_message(t);
        t = t->next;
    }
    printf("\x1b[7m");
    printf("\n#> :\x1b[m %s\n", command);
    fflush(stdout);
}
void editorProcessKeypress() {
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
        if (iscntrl(c)) {
            if ( c == '\n' ) {
                printf("Got end\n");
                if ( strcmp("Bye", command) ==0) {
                    exit(0);
                } else {
                    add_message(nick, command);
                }

                bzero(command, sizeof(command));
                pos = 0;
                break;
            } else if(c == 127) {
                printf("Backspacing");
                command[--pos] = 0;
                /* pos--; */
                break;
            } else {
                printf("%d\n", c);
            }
            break;
        } else {
            command[pos++] = c;
            break;

        }
    }
}
int main (int argc, char *argv[])
{

    add_message("server", "anandu has joined chat");
    add_message("anandu", "example message 1");
    add_message("jane", "hi there anandu");

    printf("Enter NickName: ");
    scanf("%s", nick);
    enableRawMode();
    while (1) {
        draw();
        editorProcessKeypress();
    }
    disableRawMode();
    return 0;
}
