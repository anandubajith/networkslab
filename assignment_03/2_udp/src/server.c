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

void send_file(char *filename, int sock, void* their_addr, socklen_t size) {
    printf("Sending struct over UDP\n");
    Message *m = malloc(sizeof(Message));
    FILE *fptr = fopen(filename, "r");

    int packet_count =0;
    int current_seq_no = 0;

    memset(m, 0, sizeof(*m));

    int count = fread(m->data, sizeof(char), PACKET_SIZE, fptr);
    while ( count ) {
        printf("Sending packet \n");
        packet_count++;
        m->size = count;
        m->seq_no = current_seq_no;

        printf("sending packet=%d seq_no=%d ack_no=%d size=%d\n", packet_count,m->seq_no, m->ack_no, m->size);

        clock_t begin = clock();
        sendto(sock, m, sizeof(*m), 0, (struct sockaddr *)&their_addr, size );
        // wait for an ACK
        bzero(m->data, PACKET_SIZE);
        printf("Wating for ACK\n");

        int r = recvfrom(sock, m , sizeof(*m), 0, (struct sockaddr *)&their_addr, &size);
        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

        if ( r == -1 ) {
            // TODO
            printf("Waiting for ACK timed out\n");
            // move -PACKET_SIZE with fseek and resend packt
            fseek(fptr, -PACKET_SIZE, SEEK_CUR);
        } else {
            if ( m->ack_no == current_seq_no) {
                printf("\rRound Trip Time: %lf", time_spent);
                fflush(stdout);
                /* printf("\nGotCorrectACK\n"); */
                current_seq_no++;
            } else {
                printf("\nMissed ACK resend\n");
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
    sendto(sock, m, sizeof(*m), 0, (struct sockaddr *)&their_addr, size );
    free(m);

}

int main (int argc, char *argv[])
{

	struct sockaddr_storage their_addr;
    int server_sock = socket(AF_INET, SOCK_DGRAM, 0);
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

    char* buffer = malloc(sizeof(char) * BUF_SIZE);

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 10000;

    if (setsockopt (server_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0)
        printf("setsocketopt(SO_RCVTIMEO) failed");

    while (1) {
        socklen_t size = sizeof(their_addr);
        memset(buffer, 0, BUF_SIZE);
        int l = recvfrom(server_sock, buffer, BUF_SIZE-1 , 0, (struct sockaddr *)&their_addr, &size);
        int len = strlen(buffer);
        printf("Received Message: %d \"%s\"\n\n" , len, buffer);
        if ( strcmp("Bye", buffer) == 0) {
            printf("Client closed connection\n");
            break;
        } else if ( strcmp("GiveMeVideo", buffer) == 0) {
            printf("Sending video :)\n");
            send_file("./file.bin", server_sock, &their_addr, size) ;
        } else {
            printf("Invalid message\n");
            memset(buffer, 0, BUF_SIZE);
            strcpy(buffer, "Gibberish\n");
            sendto(server_sock,buffer, strlen(buffer), 0, (struct sockaddr *)&their_addr, size );
        }
    }

    close(server_sock);
    return 0;
}