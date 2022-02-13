#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <pthread.h>

#define PACKET_SIZE 500
#define BUF_SIZE 1024
#define PORT 12356

typedef struct _message {
    int seq_no;
    int ack_no;
    int size;
    char data[PACKET_SIZE];
} Message;
