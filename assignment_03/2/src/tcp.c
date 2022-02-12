#include "common.h"

void recv_file(char *filename, int sock) {
    Message *m = malloc(sizeof(Message));
    FILE *fptr = fopen(filename, "w");

    int packet_count = 0;
    int current_seq_no = 0;

    while (1) {
        memset(m, 0, sizeof(*m));
        recv(sock, m, sizeof(*m), 0);
        packet_count++;

        printf("received packet=%d seq_no=%d ack_no=%d size=%d\n", packet_count,m->seq_no,  m->ack_no, m->size);

        if ( m->seq_no != current_seq_no) {
            // ack we sent was not received
            printf("ACK was not received on server\n");
            fseek(fptr, -(current_seq_no-m->seq_no+1)*PACKET_SIZE,SEEK_CUR);
            current_seq_no = m->seq_no;
        }

        fwrite(m->data, sizeof(char), m->size, fptr);

        bzero(m->data,PACKET_SIZE);
        // START: Simulating packet loss
        /* int jitter = (rand()%1000); */
        /* m->ack_no = current_seq_no; */
        /* if ( jitter > 0 ) { */
        /*     m->ack_no -= 1; */
        /* } */
        // END: Simulating packet loss

        printf("sending packet=%d seq_no=%d ack_no=%d size=%d\n", packet_count, m->seq_no,  m->ack_no, m->size);
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

void send_file(char *filename, int client_socket) {
    Message *m = malloc(sizeof(Message));
    FILE *fptr = fopen(filename, "r");

    int packet_count =0;
    int current_seq_no = 0;

    memset(m, 0, sizeof(*m));

    int count = fread(m->data, sizeof(char), PACKET_SIZE, fptr);
    while ( count ) {
        packet_count++;
        m->size = count;
        m->seq_no = current_seq_no;

        printf("sending packet=%d seq_no=%d ack_no=%d size=%d\n", packet_count,m->seq_no, m->ack_no, m->size);
        send(client_socket, m, sizeof(*m), 0);
        // wait for an ACK
        bzero(m->data, PACKET_SIZE);
        int r = recv(client_socket, m, sizeof(*m), 0);
        if ( r == -1 ) {
            // TODO
            printf("Waiting for ACK timed out\n");
            // move -PACKET_SIZE with fseek and resend packt
            fseek(fptr, -PACKET_SIZE, SEEK_CUR);
        } else {
            if ( m->ack_no != current_seq_no ) {
                // received ACK of older packet [ go back to last recv packed ]
                printf("ACK mismatch\n");
                fseek(fptr, -(current_seq_no-m->ack_no + 1) * PACKET_SIZE, SEEK_CUR);
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
    m->size = 0;
    send(client_socket, m, sizeof(*m), 0);
    free(m);
}

