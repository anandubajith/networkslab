#include "common.h"

void evaluate(char *s) {
    int l, r;
    char o;

    int r0 = sscanf(s, "%d %c %d", &l, &o, &r);

    if ( r0 != 3) {
        memset(s, 0, BUF_SIZE);
        sprintf(s, "Error: Invalid Expression\n");
        return;
    }


    memset(s, 0, BUF_SIZE);
    if ( o == '+' ) {
        sprintf(s, "%d\n", l+r);
        return;
    }
    if ( o == '-' ) {
        sprintf(s, "%d\n", l-r);
        return;
    }
    if ( o == '*' ) {
        sprintf(s, "%d\n", l*r);
        return;
    }
    if ( o == '/' ) {
        if ( r == 0) {
            sprintf(s, "Error: Division by zero\n");
            return;
        }
        sprintf(s, "%lf\n", (float)l/r);
        return;
    }
    sprintf(s, "Error: Invalid operator\n");
    printf("Scanned l=%d, operator=%c, r=%d, retVal = %d\n", l, o, r, r0);

}

void handle_client(int client_socket, int client_id) {
    printf("Connected to Client %d\n", client_id);
    char* buffer = malloc(sizeof(char) * BUF_SIZE);
    while(1) {
        memset(buffer, 0, BUF_SIZE);
        int r = recv(client_socket, buffer,BUF_SIZE , 0);
        if ( r == -1 ) {
            continue;
        } 
        if ( r == 0 ) {
            printf("Disconnected from Client %d\n", client_id);
            close(client_socket);
            free(buffer);
            return;
        }
        if ( strncmp("Bye", buffer, 3) == 0)  {
            printf("Disconnected from Client %d\n", client_id);
            close(client_socket);
            free(buffer);
            return;
        }
        printf("[Client%d] %s\n", client_id, buffer);
        evaluate(buffer);
        printf("[Server]: %s\n", buffer);
        send(client_socket, buffer, strlen(buffer), 0);
    }

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

    int client_count = 0;

    printf("Waiting for connections\n");
    while (1) {
        int client_socket = accept(server_sock, NULL, NULL);
        if ( client_socket == -1 ) {
            continue;
        }
        client_count++;
        if (!fork()) {
            close(server_sock);
            handle_client(client_socket, client_count);
        }
        close(client_socket);
    }

    close(server_sock);
    return 0;
}
