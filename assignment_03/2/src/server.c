#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

#include "common.h"

#define BACKLOG 5
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

    while (1) {
        printf("Waiting for connection\n");
        int client_socket = accept(server_sock, NULL, NULL);
        printf("Accepted connection\n");
        send_file("./file.bin", client_socket);
        close(client_socket);
    }

    close(server_sock);
    return 0;
}
