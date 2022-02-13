#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#include "common.h"

int current_seq_no = 0;
FILE *progress_ptr = NULL;

void* timer_thread() {
    int prev_seq_no = 0;
    // current_seq_no - prev_seq_no * 500 bytes
    // was transferred in last 0.1 sec
    // => Transmission speed = (current -prev) * 500 * 10 bytes per second
    // write this to the file
    while (1) {
        int bps = (current_seq_no - prev_seq_no) * PACKET_SIZE * 10;
        printf("\rTransmission rate = %d kbps ", bps/1024);
        fflush(stdout);
        prev_seq_no = current_seq_no;
        fprintf(progress_ptr, "%d\t%d\n",prev_seq_no, bps/1024);
        usleep(1e5);
    }
    // TODO better to use mutex, but since only reading int int
    // it won't cause issue here
}


void recv_file(char *filename, int sock, void* server_address, socklen_t size) {
    printf("Reached recv-file\n");
    Message *m = malloc(sizeof(Message));

    FILE *fptr = fopen(filename, "w");
    progress_ptr = fopen("stats.dat", "w");

    int packet_count = 0;

    current_seq_no = 0;

    pthread_t timer_t;
    /* pthread_create(&timer_t, NULL, timer_thread, NULL); */

    while (1) {
        memset(m, 0, sizeof(*m));

        printf("Waiting for packet\n");
        int recv_size = recvfrom(sock,m , sizeof(*m), 0, (struct sockaddr *)server_address, &size);
        packet_count++;

        printf("received packet=%d seq_no=%d ack_no=%d size=%d\n", packet_count,m->seq_no,  m->ack_no, m->size);

        if ( recv_size > 0 && m->seq_no == current_seq_no ) {
            fwrite(m->data, sizeof(char), m->size, fptr);
            m->ack_no = current_seq_no;
            current_seq_no++;
        }
        bzero(m->data,PACKET_SIZE);

        printf("sending packet=%d seq_no=%d ack_no=%d size=%d\n", packet_count, m->seq_no,  m->ack_no, m->size);

        sendto(sock, m, sizeof(*m), 0, (struct sockaddr *)server_address, size );

        // this would be the last packet
        if ( m->size < PACKET_SIZE ) {
            /* pthread_cancel(timer_t); */
            break;
        }
    }
    printf("\nReceived %d packets\n", packet_count);
    fclose(fptr);
    fclose(progress_ptr);
    free(m);

}

int main (int argc, char *argv[])
{

    int client_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        printf("setsockopt(SO_REUSEADDR) failed");

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;
    socklen_t server_size = sizeof(server_address);

    struct sockaddr_in client_address;
    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(PORT+1);
    client_address.sin_addr.s_addr = INADDR_ANY;
    socklen_t client_size = sizeof(client_address);

    int success = bind(client_sock, (struct sockaddr*) &client_address, sizeof(client_address));

    char* buffer = malloc(sizeof(char) * BUF_SIZE);

    while(1) {
        printf(">");
        memset(buffer, 0, BUF_SIZE);
        scanf("%[^\n]", buffer);
        char t; scanf("%c", &t);

        if ( strlen(buffer) == 0){
            continue;
        }

        buffer[BUF_SIZE-1] = '\0';

        if ( strcmp("Bye", buffer) == 0) {
            printf("Connection closed\n");
            return 0;
        } else if ( strcmp("GiveMeVideo", buffer) == 0 ) {
            sendto(client_sock,buffer, strlen(buffer), 0, (struct sockaddr *)&server_address, server_size );
            recv_file("./output", client_sock, &server_address, server_size) ;
        } else {
            sendto(client_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&server_address, server_size );
            recvfrom(client_sock, buffer, BUF_SIZE-1 , 0, (struct sockaddr *)&server_address, &server_size );
            printf("%s", buffer);

        }
    }

    return 0;
}
