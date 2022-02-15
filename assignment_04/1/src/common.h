#include<stdio.h>
#include <termios.h>
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

typedef struct _packet {
    unsigned int time;
    char from[BUF_SIZE/4];
    char body[BUF_SIZE];
} Packet;

typedef struct _item {
    Packet *packet;
    struct _item *next;
} Item;

void add_message(char*, char*);
Message* new_message(char*, char*);
void print_message(Message *m);
void setup_terminal();
void reset_terminal();
