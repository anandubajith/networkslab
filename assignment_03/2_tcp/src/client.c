#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include <pthread.h>

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



void recv_file(char *filename, int sock) {
    Message *m = malloc(sizeof(Message));
    Message *r = malloc(sizeof(Message));

    FILE *fptr = fopen(filename, "w");
    progress_ptr = fopen("stats.dat", "w");

    int packet_count = 0;

    current_seq_no = 0;
    int current_ack_no = -1;

    pthread_t timer_t;
    pthread_create(&timer_t, NULL, timer_thread, NULL);

    while (1) {
        memset(m, 0, sizeof(*m));

        int recv_size = recv(sock, m, sizeof(*m), 0);
        packet_count++;

        /* printf("received packet=%d seq_no=%d ack_no=%d size=%d\n", packet_count,m->seq_no,  m->ack_no, m->size); */

        if ( recv_size > 0 && m->seq_no == current_seq_no ) {
            fwrite(m->data, sizeof(char), m->size, fptr);
            current_ack_no = m->ack_no;
            m->ack_no = current_seq_no;
            current_seq_no++;
        } else {
            /* printf("ACK was not received on server\n"); */
            // we will atmost drop 1 packet
            fseek(fptr, -PACKET_SIZE, SEEK_CUR);
        }

        bzero(m->data,PACKET_SIZE);
        // START: Simulating packet loss
        //
        // END: Simulating packet loss

        /* printf("sending packet=%d seq_no=%d ack_no=%d size=%d\n", packet_count, m->seq_no,  m->ack_no, m->size); */
        send(sock, m, sizeof(*m), 0);
        /* getc(stdin); */

        // this would be the last packet
        if ( m->size < PACKET_SIZE ) {
            pthread_cancel(timer_t);
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

    char* buffer = malloc(sizeof(char) * BUF_SIZE);

    while(1) {
        printf(">");
        memset(buffer, 0, BUF_SIZE);
        scanf("%[^\n]", buffer);
        char t; scanf("%c", &t);
        if ( strlen(buffer) == 0){
            continue;
        }
        if ( strcmp(buffer, "GiveMeVideo") == 0 ) {
            send(sock, buffer, strlen(buffer), 0);
            recv_file("./output", sock);
        } else {
            send(sock, buffer, strlen(buffer), 0);
            if ( strcmp(buffer, "Bye") == 0 ) {
                close(sock);
                return 0;
            }
            memset(buffer, 0, BUF_SIZE);
            recv(sock, buffer,BUF_SIZE , 0);
            printf("%s", buffer);
        }
    }

    return 0;
}
