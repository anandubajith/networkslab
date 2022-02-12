#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

#include "common.h"

#define BACKLOG 5
#define PORT 12356


void send_file(char *filename, int client_socket) {
    Message *m = malloc(sizeof(Message));
    FILE *fptr = fopen(filename, "r");

    int packet_count =0;
    int current_seq_no = 0;

    memset(m, 0, sizeof(*m));
    m->type = 2;// for filedata

    int count = fread(m->data, sizeof(char), PACKET_SIZE, fptr);
    while ( count ) {
        packet_count++;
        m->size = count;
        m->seq_no = current_seq_no;

        send(client_socket, m, sizeof(*m), 0);
        // wait for an ACK
        bzero(m->data, PACKET_SIZE);
        int r = recv(client_socket, m, sizeof(*m), 0);
        if ( r == -1 ) {
            // TODO
            printf("Waiting for ACK timed out\n");
            // move -PACKET_SIZE with fseek
            fseek(fptr, -PACKET_SIZE, SEEK_CUR);
        } else {
            if ( m->ack != current_seq_no ) {
                // received ACK of older packet [ go back to last recv packed ]
                printf("ACK mismatch");
                fseek(fptr, -(current_seq_no-m->ack + 1) * PACKET_SIZE, SEEK_CUR);
            } else {
                current_seq_no++;
            }
        }
        bzero(m->data, PACKET_SIZE);
        /* usleep(1e3); */
        count = fread(m->data, sizeof(char), PACKET_SIZE, fptr);
    }
    fclose(fptr);
    printf("Sent %d packets\n", packet_count);
    printf("Actual data packets: %d\n", current_seq_no);

    // send that file is done
    memset(m, 0, sizeof(*m));
    m->type=33;
    m->size = 0;
    send(client_socket, m, sizeof(*m), 0);
    free(m);
}

int main (int argc, char *argv[])
{

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        printf("setsockopt(SO_REUSEADDR) failed");

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 10000;

    /* if (setsockopt (server_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0) */
    /*     printf("setsocketopt(SO_RCVTIMEO) failed"); */


    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int success = bind(server_sock, (struct sockaddr*) &server_address, sizeof(server_address));
    if ( success != 0 ) {
        printf("Bind failed");
        exit(-1);
    }

    listen(server_sock, BACKLOG);

    while (1) {
        printf("Waiting for connection\n");
        int client_socket = accept(server_sock, NULL, NULL);
        printf("Accepted connection\n");
        send_file("./file.bin", client_socket);
        close(client_socket);
    }

    close(server_sock);
    return 0;
}
