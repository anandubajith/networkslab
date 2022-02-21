#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<time.h>

#define PORT 2021
#define BUF_SIZE 1024
#define BACKLOG 5

/*
 * write a send_file, receive_file
 * which would be commmon
 */

void handle_client(int client_socket) {
    printf("Connected to Client\n");
    char* buffer = malloc(sizeof(char) * BUF_SIZE);
    memset(buffer, 0, BUF_SIZE);
    /*
     * START
     * USERN
     * PASSWD
     * CreateFile
     * ListDir
     * StoreFile
     * GetFile
     * Quit
     */

    send(client_socket, buffer, strlen(buffer), 0);
    free(buffer);
    close(client_socket);
}

int main ()
{

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        printf("setsockopt(SO_REUSEADDR) failed");

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

    printf("Waiting for connections on %d\n", PORT);
    while (1) {
        int client_socket = accept(server_sock, NULL, NULL);
        if ( client_socket == -1 ) {
            continue;
        }
        if (!fork()) {
            close(server_sock);
            handle_client(client_socket);
        }
        close(client_socket);
    }

    close(server_sock);
    return 0;
}
