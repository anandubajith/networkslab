#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

#define PACKET_SIZE 500
#define TIMEOUT 123

typedef struct _message {
    int seq_no;
    int ack_no;
    int size;
    char data[PACKET_SIZE];
} Message;

void recv_file(char *, int );
void send_file(char *, int );
