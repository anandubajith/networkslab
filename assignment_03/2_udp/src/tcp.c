#include "common.h"

#include <pthread.h>


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

