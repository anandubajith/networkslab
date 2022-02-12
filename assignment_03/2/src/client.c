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

    Message *m = malloc(sizeof(Message));

    FILE *fptr = fopen("./output", "w");
    int packet_count = 0;
    while (1) {
        memset(m, 0, sizeof(*m));
        recv(sock, m, sizeof(*m), 0);

        printf("received packet %d with %d bytes\n", packet_count, m->offset);
        packet_count++;
        if ( m->type == 33) {
            printf("Received %d packets\n", packet_count);
            fclose(fptr);
            return 0;
        }
        fprintf(fptr,"%s", m->data);
    }
    fclose(fptr);
    /* printf("Seq: %d\nType: %d\nOffset: %d\n", m->seq_no, m->type, m->offset); */
    /* printf("bufdata:%s\n", m->data); */

    return 0;
}
