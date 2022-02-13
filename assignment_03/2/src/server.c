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

int main (int argc, char *argv[])
{

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        printf("setsockopt(SO_REUSEADDR) failed");

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 10000;

    /* if (setsockopt (server_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0) */
    /*     printf("setsocketopt(SO_RCVTIMEO) failed"); */


    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int success = bind(server_sock, (struct sockaddr*) &server_address, sizeof(server_address));
    if ( success != 0 ) {
        printf("Bind failed");
        exit(-1);
    }

    listen(server_sock, BACKLOG);

    char* buffer = malloc(sizeof(char) * BUF_SIZE);

    while (1) {
        printf("Waiting for connection\n");
        int client_socket = accept(server_sock, NULL, NULL);
        printf("Accepted connection\n");
        while (1) {
            memset(buffer, 0, BUF_SIZE);
            recv(client_socket, buffer,BUF_SIZE , 0);
            printf("Received message %s\n", buffer);
            if ( strcmp("Bye", buffer) == 0) {
                close(client_socket);
                break;
            } else if ( strcmp("GiveMeVideo", buffer) == 0) {
                printf("Sending video :)\n");
                send_file("./file.bin", client_socket);
            } else {
                printf("Invalid message\n");
                strcpy(buffer, "Gibberish\n");
                send(client_socket, buffer, strlen(buffer), 0);
            }
        }
    }

    close(server_sock);
    return 0;
}
