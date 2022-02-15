#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<time.h>

#define PORT 12129
#define BUF_SIZE 1024
#define BACKLOG 5

typedef struct _message {
    char* from;
    char* message;
    unsigned long time;
    struct _message *next;
} Message;

typedef struct _state {
    Message *messages;
    char* buffer;
} State;

void add_message(char*, char*);
void print_message(Message *m);
void draw();
void setup_terminal();
void reset_terminal();
