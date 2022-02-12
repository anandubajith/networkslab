#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

#include "common.h"

#define BACKLOG 5
#define BUF_SIZE 1024
#define PORT 12356


int main (int argc, char *argv[])
{

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        printf("setsockopt(SO_REUSEADDR) failed");

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int success = bind(server_sock, (struct sockaddr*) &server_address, sizeof(server_address));
    if ( success != 0 ) {
        printf("Bind failed");
        exit(-1);
    }


    Message *m = malloc(sizeof(Message));
    memset(m, 0, sizeof(*m));
    m->seq_no = 1;
    m->type = 2;
    m->offset = 99;

    /* for( int i = 0; i < PACKET_SIZE;i++ ) { */
    /*     m->data[i] = 'A' + i%26; */
    /* } */


    listen(server_sock, BACKLOG);

    while ( 1) {
        printf("Waiting for connection\n");
        int client_socket = accept(server_sock, NULL, NULL);
        printf("Accepted connection\n");
        FILE *fptr = fopen("./file.bin", "r");
        int packet_count =0;

        int count = fread(m->data, sizeof(char), PACKET_SIZE, fptr);
        while ( count ) {
            packet_count++;
            m->offset = count;
            send(client_socket, m, sizeof(*m), 0);
            bzero(m->data, PACKET_SIZE);
            usleep(1e5);
            count = fread(m->data, sizeof(char), PACKET_SIZE, fptr);
        }
        printf("Nothing more to read\n");
        printf("Sent %d packets\n", packet_count);

        m->type=33;
        m->offset = 0;
        send(client_socket, m, sizeof(*m), 0);

        close(client_socket);
    }

    close(server_sock);
    return 0;
}
