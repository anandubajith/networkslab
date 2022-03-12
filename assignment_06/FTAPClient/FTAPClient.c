#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define PORT 4035
#define BUF_SIZE 1024
#define PACKET_SIZE 1024

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

    int r =
        connect(sock, (struct sockaddr *)&server_address, sizeof(server_address));
    if (r == -1) {
        printf("Error: Failed to connect()");
        exit(1);
    }

    return sock;
}

int current_bytes = 0;
int total_bytes = 0;
int is_transferring = 0;

void *timer_thread() {
    int prev_bytes = 0;
    while (1) {
        // bytes transferred in 1e5 seconds
        int bytes_transferred = (current_bytes - prev_bytes);
        int speed = bytes_transferred * 10;
        if (speed != 0) {

            int remaining_bytes = total_bytes - current_bytes;

            int remaining_time = remaining_bytes / speed;
            printf("\r\r\x1b[2K\x1b[1;34mTransmission rate = %d kbps %d s remaining\x1b[m", speed / 1024, remaining_time);
            fflush(stdout);
            prev_bytes = current_bytes;
        }
        usleep(1e5);
    }
}

void print_packet(Packet *p) {
    printf("\x1b[1;34m\x1b[3m");
    printf("status_code = %d\n", p->code);
    printf("\x1b[1;35m\x1b[3m");
    printf("%s\n", p->data);
    printf("\x1b[m\x1b[23m");
}

void handle_get_file(int socket, char *filename) {

    Packet *p = malloc(sizeof(Packet));
    FILE *fp = fopen(filename, "wb");

    memset(p, 0, sizeof(*p));
    int recv_size = recv(socket, p, sizeof(*p), 0);
    if (recv_size <= 0) {
        printf("Server closed connection\n");
        return;
    }

    if (p->code != 601) {
        print_packet(p);
        return;
    }

    sscanf(p->data, "%d", &total_bytes);
    current_bytes = 0;

    if (total_bytes != 0) {
        pthread_t timer_t;
        is_transferring = 1;
        pthread_create(&timer_t, NULL, timer_thread, NULL);
        while (1) {
            memset(p, 0, sizeof(*p));
            int recv_size = recv(socket, p, sizeof(*p), 0);
            if (recv_size <= 0) {
                printf("Server closed connection");
                return;
            }
            current_bytes = ftell(fp);
            /* printf("%s", p->data); */
            fwrite(p->data, sizeof(char), p->size, fp);
            /* printf("Received packet with code = %d size = %d \n", p->code, p->size); */
            if (p->code == 603) {
                break;
            }
        }

        is_transferring = 0;
        pthread_cancel(timer_t);
    }

    fclose(fp);
    free(p);
    printf("\n\x1b[1;35mDownload %s complete\x1b[m\n", filename);
}

void handle_store_file(int socket, char *filename) {
    Packet *p = malloc(sizeof(Packet));

    // check if it's okay to send?
    memset(p, 0, sizeof(*p));
    recv(socket, p, sizeof(*p), 0);
    // check file error from server;
    if ( p->code != 600 ) {
        print_packet(p);
        return;
    }

    FILE *fp = fopen(filename, "rb");

    // send FileInfo packet
    memset(p, 0, sizeof(*p));
    fseek(fp, 0L, SEEK_END);
    p->code = 601;
    int file_size = ftell(fp);

    current_bytes = 0;
    total_bytes = file_size;

    sprintf(p->data, "%d", file_size);
    p->size = strlen(p->data);
    /* printf("Total File Size = %s\n", p->data); */
    fseek(fp, 0L, SEEK_SET);
    send(socket, p, sizeof(*p), 0);

    pthread_t timer_t;
    pthread_create(&timer_t, NULL, timer_thread, NULL);
    is_transferring = 1;

    memset(p, 0, sizeof(*p));

    int count = fread(p->data, sizeof(char), PACKET_SIZE, fp);
    while (count) {
        // to decide FileData , or FileEnd
        p->code = ftell(fp) == file_size ? 603 : 602;
        current_bytes = ftell(fp);
        p->size = count;
        send(socket, p, sizeof(*p), 0);

        /*
         * it seems the issue is window size, and breaking up of packets
         */
        usleep(100);
        memset(p, 0, sizeof(*p));
        count = fread(p->data, sizeof(char), PACKET_SIZE, fp);
    }
    pthread_cancel(timer_t);
    is_transferring = 0;
    fclose(fp);
    free(p);

    printf("\n\x1b[1;35mUpload %s complete\x1b[m\n", filename);
}


void close_handler() {
    if ( !is_transferring ) {
        exit(0);
    }
}

int main() {
    signal(SIGINT, close_handler);
    int sock = -1;
    Packet *p = malloc(sizeof(Packet));
    char *buffer = malloc(sizeof(char) * BUF_SIZE);
    int authenticated = 0;

    while (1) {
        fflush(stdout);
        memset(buffer, 0, BUF_SIZE);
        scanf("%[^\n]", buffer);
        char t;
        scanf("%c", &t);
        if (strlen(buffer) == 0) {
            continue;
        }
        if (strncmp("START", buffer, 5) == 0) {
            if (sock != -1) {
                printf("\x1b[1;31mAlready connected to server\x1b[m\n");
            } else {
                sock = setup_connection();
                send(sock, buffer, strlen(buffer), 0);
                recv(sock, p, sizeof(*p), 0);
                print_packet(p);
            }
        } else if (sock != -1) {
            if (authenticated == 1 && strncmp("StoreFile", buffer, 9) == 0) {
                if (access(buffer + 10, F_OK) == 0) {
                    send(sock, buffer, strlen(buffer), 0);
                    handle_store_file(sock, buffer + 10);
                } else {
                    printf("\x1b[1;31mInvalid filename\x1b[m\n");
                }
            } else if (authenticated == 1 && strncmp("GetFile", buffer, 7) == 0) {
                if (access(buffer + 8, F_OK) == 0) {
                    printf("\x1b[1;31mFile already exists on client\x1b[m\n");
                } else {
                    send(sock, buffer, strlen(buffer), 0);
                    handle_get_file(sock, buffer + 8);
                }
            } else {
                send(sock, buffer, strlen(buffer), 0);
                recv(sock, p, sizeof(*p), 0);
                if (p->code == 305)
                    authenticated = 1;
                else if (p->code == 495) {
                    shutdown(sock, 2);
                    return 0;
                }
                print_packet(p);
            }
        } else {
            printf("\x1b[1;31mConnect to server with 'START'\x1b[m\n");
        }
    }

    return 0;
}
