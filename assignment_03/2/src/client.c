#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#include "common.h"

#define BUF_SIZE 1024
#define PORT 12356


void recv_file(char *filename, int sock) {
    Message *m = malloc(sizeof(Message));
    FILE *fptr = fopen(filename, "w");

    int packet_count = 0;
    int current_seq_no = 0;

    while (1) {
        memset(m, 0, sizeof(*m));
        recv(sock, m, sizeof(*m), 0);
        printf("received packet=%d seq_no=%d ack_no=%d size=%d\n", packet_count,m->seq_no,  m->ack_no, m->size);
        packet_count++;

        if ( m->seq_no != current_seq_no) {
            // ack we sent was not received
            printf("ACK was not received on server\n");
            fseek(fptr, -(current_seq_no-m->seq_no+1)*PACKET_SIZE,SEEK_CUR);
            current_seq_no = m->seq_no;
        }

        fwrite(m->data, sizeof(char), m->size, fptr);

        bzero(m->data,PACKET_SIZE);
        m->ack_no = current_seq_no;
        send(sock, m, sizeof(*m), 0);

        current_seq_no++;
        // this would be the last packet
        if ( m->size < PACKET_SIZE ) {
            break;
        }
    }
    printf("Received %d packets\n", packet_count);
    fclose(fptr);
    free(m);
}

int main (int argc, char *argv[])
{

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int r = connect(sock, (struct sockaddr *) &server_address, sizeof(server_address));
    if ( r == -1) {
        printf("Error: " );
        exit(1);
    }

    recv_file("./output", sock);

    return 0;
}
