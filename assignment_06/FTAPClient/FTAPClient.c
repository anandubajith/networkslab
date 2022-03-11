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

        int bytes_transferred = (current_bytes-prev_bytes); // bytes transferred in 1e5 seconds
        int speed = bytes_transferred *10;
        int remaining_bytes = total_bytes-current_bytes;

        int remaining_time = remaining_bytes / speed;
        printf("\r\rTransmission rate = %d kbps %d s remaining", speed/1024, remaining_time);
        fflush(stdout);
        prev_bytes = current_bytes;
        usleep(1e5);
    }
}

void handle_get_file(int socket, char *filename) {

    Packet *p = malloc(sizeof(Packet));
    FILE* fp = fopen(filename, "wb");

    memset(p, 0, sizeof(*p));
    int recv_size = recv(socket, p, sizeof(*p), 0);
    if ( recv_size <= 0 ) {
        printf("Server closed connection\n");
        return;
    }

    /* printf("Received packet with code = %d\n", p->code); */
    if ( p->code != 601) {
        printf("INvalid FileInfoPacket\n");
        return;
    }
    // read file_info packet and
    int total_size = 0;
    sscanf(p->data, "%d", &total_size);

    while (1) {
        memset(p, 0, sizeof(*p));
        int recv_size = recv(socket, p, sizeof(*p), 0);
        printf("recv_size = %d\n" , recv_size);
        if ( recv_size <= 0 ) {
            printf("Server closed connection");
            return;
        }
        /* printf("%s", p->data); */
        fwrite(p->data, sizeof(char), p->size, fp);
        /* printf("Received packet with code = %d size = %d \n", p->code, p->size); */
        if ( p->code == 603) {
            break;
        }
    }

    fclose(fp);
    free(p);
    printf("Download %s complete\n", filename);
}

void handle_store_file(int socket, char *filename) {
    // todo: check if file exists

    Packet *p = malloc(sizeof(Packet));
    FILE* fp = fopen(filename, "rb");

    // send FileInfo packet99069
    memset(p, 0, sizeof(*p));
    fseek(fp, 0L, SEEK_END);
    p->code = 601;
    int file_size = ftell(fp);

    current_bytes = 0;
    total_bytes = file_size;

    sprintf(p->data, "%d", file_size);
    p->size = strlen(p->data);
    printf("Total File Size = %s\n", p->data);
    fseek(fp, 0L, SEEK_SET);
    send(socket, p, sizeof(*p), 0);

    pthread_t timer_t;
    pthread_create(&timer_t, NULL, timer_thread, NULL);

    memset(p, 0, sizeof(*p));


    int count = fread(p->data, sizeof(char), PACKET_SIZE, fp);
    while ( count ) {
        // to decide FileData , or FileEnd
        p->code = ftell(fp) == file_size ? 603 : 602;
        current_bytes = ftell(fp);
        p->size = count;
        int send_size = send(socket, p, sizeof(*p), 0);

        /*
         * it seems the issue is window size, and breaking up of packets
         */
        usleep(100);
        memset(p, 0, sizeof(*p));
        count = fread(p->data, sizeof(char), PACKET_SIZE, fp);
    }
    pthread_cancel(timer_t);
    fclose(fp);
    free(p);

    // todo: display avg speed + duration after overwriting
    printf("\rUpload %s complete\n", filename);
}


int main() {

    int sock = -1;
    Packet* p = malloc(sizeof(Packet));
    char* buffer = malloc(sizeof(char) * BUF_SIZE);
    int authenticated = 0;

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
            if ( authenticated == 1 && strncmp("StoreFile", buffer ,9) == 0) {
                if( access( buffer+10, F_OK ) == 0 ) {
                    send(sock, buffer, strlen(buffer), 0);
                    handle_store_file(sock, buffer+10);
                } else {
                    printf("Invalid filename \n");
                }
            } else if (authenticated == 1 &&  strncmp("GetFile", buffer, 7) == 0 ) {
                if( access( buffer+10, F_OK ) == 0 ) {
                    send(sock, buffer, strlen(buffer), 0);
                    handle_get_file(sock, buffer+10);
                } else {
                    printf("Invalid filename \n");
                }
            } else {
                send(sock, buffer, strlen(buffer), 0);
                recv(sock, p, sizeof(*p), 0);
                if ( p->code == 305)
                    authenticated = 1;
                else if (p->code == 495) {
                    shutdown(sock, 2);
                    return 0;
                }
                printf("Received packet %d %d %s\n", p->code, p->size, p->data);
            }
        } else {
            printf("Connect to server with START\n");
        }
    }

    return 0;
}
