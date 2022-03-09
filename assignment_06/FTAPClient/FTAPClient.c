#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<time.h>

#define PORT 4035
#define BUF_SIZE 1024

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

typedef struct _packet {
    int code;
    int size;
    char data[504];
} Packet;

void send_file(int socket, char *filename) {

    Packet *p = malloc(sizeof(Packet));

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
    int count = fread(p->data, sizeof(char), 504, fp);
    while ( count ) {
        // to decide FileData , or FileEnd
        p->code = ftell(fp) == file_size ? 603 : 602;
        p->size = count;
        int send_size = send(socket, p, sizeof(*p), 0);

        /*
         * it seems the issue is window size, and breaking up of packets
         */
        usleep(100);

        printf("Sending packet with size ; size = %d; send_size = %d,  code = %d %d\n", count ,send_size, p->code,  count);

        // print progress + remaining time

        memset(p, 0, sizeof(*p));
        count = fread(p->data, sizeof(char), 504, fp);
    }

    fclose(fp);

    free(p);

}


int main() {
    int sock = setup_connection();
    /* send_file(sock, "sintel.mkv"); */
    send_file(sock, "m.txt");
}


int xmain() {

    int sock = -1;

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
            // we already have active connection
            if ( strncmp("USERN", buffer, 5) == 0) {
                // send to server and get reply
            } else if ( strncmp("PASSWD", buffer, 6) == 0) {
                // send to server and get reply
            } else if ( strncmp("StoreFile", buffer ,9) == 0) {
                // check for error -> File Already exists
                // send 600 FileStart
                // send 601
                // send 602
            } else if ( strncmp("GetFile", buffer, 7) == 0 ) {
                // receive 600, 601, 602
            } else if ( strncmp("CreateFile", buffer, 10) == 0) {
                // check for error -> File Already exists
                // send 600 FileStart
                // send 601 with 0
                // send 602
            } else if ( strncmp("ListDir", buffer, 7) == 0) {
                // send message and get output back
            } else if ( strncmp("QUIT", buffer, 4) == 0) {
                // shutdown the connection
            } else {
                // invalid command
                // todo: decide if we have to send to server first?
            }
        } else {
            printf("Connect to server with START\n");
        }
    }

    return 0;
}
