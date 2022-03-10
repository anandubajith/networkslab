#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<time.h>

#define PORT 4035
#define BUF_SIZE 1024
#define PACKET_SIZE 504

typedef struct _packet {
    int code;
    int size;
    char data[PACKET_SIZE];
} Packet;

int setup_connection() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int r = connect(sock, (struct sockaddr *) &server_address, sizeof(server_address));
    if ( r == -1) {
        printf("Error: Failed to connect()" );
        exit(1);
    }

    return sock;
}

int current_bytes = 0;
int total_bytes = 0;

void* timer_thread() {
    int prev_bytes = 0;
    while (1) {
        int bps = (current_bytes-prev_bytes) * PACKET_SIZE * 10;
        printf("\rTransmission rate = %d kbps ", bps/1024);
        fflush(stdout);
        prev_bytes = current_bytes;
        usleep(1e5);
    }
}


void handle_store_file(int socket, char *filename) {

    Packet *p = malloc(sizeof(Packet));

    pthread_t timer_t;
    pthread_create(&timer_t, NULL, timer_thread, NULL);


    FILE* fp = fopen(filename, "rb");

    // send FileInfo packet
    memset(p, 0, sizeof(*p));
    fseek(fp, 0L, SEEK_END);
    p->code = 601;
    int file_size = ftell(fp);
    sprintf(p->data, "%d", file_size);
    p->size = strlen(p->data);
    printf("Total File Size = %s\n", p->data);
    fseek(fp, 0L, SEEK_SET);
    send(socket, p, sizeof(*p), 0);

    memset(p, 0, sizeof(*p));
    int count = fread(p->data, sizeof(char), PACKET_SIZE, fp);
    while ( count ) {
        // to decide FileData , or FileEnd
        p->code = ftell(fp) == file_size ? 603 : 602;
        p->size = count;
        int send_size = send(socket, p, sizeof(*p), 0);

        /*
         * it seems the issue is window size, and breaking up of packets
         */
        usleep(100);

        /* printf("Sending packet with size ; size = %d; send_size = %d,  code = %d %d\n", count ,send_size, p->code,  count); */

        // print progress + remaining time
        //
        //
        printf("\r %d done - %d s remaining\n", (int)ftell(fp)*100/file_size, 0);

        memset(p, 0, sizeof(*p));
        count = fread(p->data, sizeof(char), PACKET_SIZE, fp);
    }
    pthread_cancel(timer_t);

    fclose(fp);
    free(p);
}


int main() {

    int sock = -1;
    Packet* p = malloc(sizeof(Packet));
    char* buffer = malloc(sizeof(char) * BUF_SIZE);

    while(1) {
        fflush(stdout);
        memset(buffer, 0, BUF_SIZE);
        scanf("%[^\n]", buffer);
        char t; scanf("%c", &t);
        if ( strlen(buffer) == 0){
            continue;
        }
        if ( strncmp("START", buffer, 5) == 0 ) {
            if ( sock != -1 ) {
                printf("Already connected\n");
            } else {
                sock = setup_connection();
            }
        } else if ( sock != -1) {
            send(sock, buffer, strlen(buffer), 0);
            // we already have active connection
            // send the message
            // do recv of the packet
            // everything would be text exccept for store/get File
            if ( strncmp("StoreFile", buffer ,9) == 0) {
                handle_store_file(sock, buffer+10);
            } else if ( strncmp("GetFile", buffer, 7) == 0 ) {
                /* handle_get_file(socket, buffer+10); */
            } else {
                recv(sock, p, sizeof(*p), 0);
                printf("Received packet %d %d %s\n", p->code, p->size, p->data);
            }
        } else {
            printf("Connect to server with START\n");
        }
    }

    return 0;
}
