#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<time.h>

#include "common.h"

void send_file(char *filename, int client_socket) {
    Message *m = malloc(sizeof(Message));
    FILE *fptr = fopen(filename, "r");

    int packet_count =0;
    int current_seq_no = 0;
    int current_ack_no = 0;

    memset(m, 0, sizeof(*m));

    int count = fread(m->data, sizeof(char), PACKET_SIZE, fptr);
    while ( count ) {
        packet_count++;
        m->size = count;
        m->seq_no = current_seq_no;
        m->ack_no = current_ack_no;

        /* printf("sending packet=%d seq_no=%d ack_no=%d size=%d\n", packet_count,m->seq_no, m->ack_no, m->size); */


        clock_t begin = clock();
        send(client_socket, m, sizeof(*m), 0);
        // wait for an ACK
        bzero(m->data, PACKET_SIZE);
        int r = recv(client_socket, m, sizeof(*m), 0);
        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

        if ( r == -1 ) {
            // TODO
            printf("Waiting for ACK timed out\n");
            // move -PACKET_SIZE with fseek and resend packt
            fseek(fptr, -PACKET_SIZE, SEEK_CUR);
        } else {
            printf("\rRound Trip Time: %lf", time_spent);
            fflush(stdout);
            if ( m->ack_no == current_seq_no) {
                current_ack_no = m->ack_no;
                current_seq_no++;
            } else {
                printf("ACK mismatch\n");
                fseek(fptr, -PACKET_SIZE, SEEK_CUR);

            }
        }
        bzero(m->data, PACKET_SIZE);
        /* usleep(100); */
        count = fread(m->data, sizeof(char), PACKET_SIZE, fptr);
    }
    fclose(fptr);
    printf("\n");
    printf("Sent %d packets\n", packet_count);
    printf("Actual data packets: %d\n", current_seq_no);

    // send that file is done
    memset(m, 0, sizeof(*m));
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

    char* buffer = malloc(sizeof(char) * BUF_SIZE);

    while (1) {
        printf("Waiting for connection\n");
        int client_socket = accept(server_sock, NULL, NULL);
        printf("Accepted connection\n");
        while (1) {
            memset(buffer, 0, BUF_SIZE);
            recv(client_socket, buffer,BUF_SIZE , 0);
            printf("Received message %s\n", buffer);
            if ( strcmp("Bye", buffer) == 0) {
                close(client_socket);
                break;
            } else if ( strcmp("GiveMeVideo", buffer) == 0) {
                printf("Sending video :)\n");
                send_file("./file.bin", client_socket);
            } else {
                printf("Invalid message\n");
                strcpy(buffer, "Gibberish\n");
                send(client_socket, buffer, strlen(buffer), 0);
            }
        }
    }

    close(server_sock);
    return 0;
}
