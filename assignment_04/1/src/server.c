#include "common.h"
extern Message* messageHead;

int main () {

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

    setup_terminal();

    int client_socket = accept(server_sock, NULL, NULL);
    if ( client_socket == -1 ) {
        printf("Failed to accept\n");
        return -1;
    }
    char* buffer = malloc(sizeof(char) * BUF_SIZE);
    while(1) {
        bzero(buffer, BUF_SIZE);
        recv(client_socket, buffer, BUF_SIZE, 0);
        add_message("server", buffer);
        print_message(messageHead);
        bzero(buffer, BUF_SIZE);
        strcpy(buffer, "Hello from server\n");
        send(client_socket, buffer, strlen(buffer), 0);
        /* sleep(10); */
    }

    return 0;
}
