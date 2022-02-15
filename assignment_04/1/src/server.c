#include "common.h"
extern Message* messageHead;


void print_all() {
    Message * t = messageHead;
    while ( t!= NULL ){
        print_message(t);
        t = t->next;
    }
}
int main () {

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        printf("setsockopt(SO_REUSEADDR) failed");


    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    setsockopt(server_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);


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

    while(1) {
        int client_socket = accept(server_sock, NULL, NULL);
        if ( client_socket == -1 ) {
            continue;
        }
        char* buffer = malloc(sizeof(char) * BUF_SIZE);
        while(1) {
            bzero(buffer, BUF_SIZE);
            int r = recv(client_socket, buffer, BUF_SIZE, 0);
            if (r != -1 ) {
                Message *m = new_message("server", buffer);
                print_message(m);
                bzero(buffer, BUF_SIZE);
                strcpy(buffer, "Hello from server");
                send(client_socket, buffer, strlen(buffer), 0);
            }
        }
    }

    return 0;
}
