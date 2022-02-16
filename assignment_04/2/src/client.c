#include "common.h"

int sock;
char *buffer;

void close_handler(int sig) {
    signal(sig, SIG_IGN);
    printf("\nClosing connection\n");
    if ( sock != -1 && buffer != NULL) {
        strcpy(buffer, "Bye");
        send(sock, buffer, strlen(buffer), 0);
        shutdown(sock,2);
        close(sock);
        free(buffer);
    }
    exit(0);
}

int main() {

    signal(SIGINT, close_handler);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int r = connect(sock, (struct sockaddr *) &server_address, sizeof(server_address));
    if ( r == -1) {
        printf("Error: Failed to connect()" );
        exit(1);
    }

    printf("Connected to server\n");
    buffer = malloc(sizeof(char) * BUF_SIZE);

    while(1) {
        printf("\nEnter the query: ");
        fflush(stdout);
        memset(buffer, 0, BUF_SIZE);
        scanf("%[^\n]", buffer);
        char t; scanf("%c", &t);
        if ( strlen(buffer) == 0){
            continue;
        }

        send(sock, buffer, strlen(buffer), 0);
        memset(buffer, 0, BUF_SIZE);
        int r = recv(sock, buffer,BUF_SIZE , 0);
        if ( r == 0 ) {
            close_handler(0);
            return 0;
        }
        printf("[Server] %s", buffer);
    }

    return 0;
}
