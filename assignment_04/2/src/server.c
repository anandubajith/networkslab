#include "common.h"

int evaluate(char *s) {
    int result = 0;

    return result;
}

void handle_client(int client_socket) {
    char* buffer = malloc(sizeof(char) * BUF_SIZE);
    while(1) {
        memset(buffer, 0, BUF_SIZE);
        recv(client_socket, buffer,BUF_SIZE , 0);
        if ( strncmp("Bye", buffer, 3) == 0)  {
            printf("Client closed connection\n");
            close(client_socket);
            free(buffer);
        }
        int result = evaluate(buffer);
        memset(buffer, 0, BUF_SIZE);
        sprintf(buffer, "%d\n", result);
        send(client_socket, buffer, strlen(buffer), 0);
    }

}

int main (int argc, char *argv[])
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

    int clients = 0;

    printf("Waiting for connections\n");
    while (1) {
        int client_socket = accept(server_sock, NULL, NULL);
        if ( client_socket == -1 ) {
            continue;
        }
        clients++;
        printf("Accepted connection\n");
        if (!fork()) {
            close(server_sock);
            handle_client(client_socket);
        }
        close(client_socket);
    }

    close(server_sock);
    return 0;
}
