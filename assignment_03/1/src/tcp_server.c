#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

#include "fruits.h"

#define BACKLOG 5
#define BUF_SIZE 1024
#define PORT 12356

int recv_string(char *s, int socket) {
    // todo: there is a bug here if message is greater than BUF_SIZE
    memset(s, 0, BUF_SIZE);
    int num = 0;
    while( (num = recv(socket, s , BUF_SIZE, 0)) == 0 ) {
        printf("%s\n",s);
        memset(s, 0, BUF_SIZE);
    }
    if ( num != -1 ) {
        s[num-1] = '\0';
    }
    return num;
}

int main (int argc, char *argv[])
{

    setup_fruits();

    char* server_message = malloc(sizeof(char) * BUF_SIZE);
    strcpy(server_message, "Welcome to store\n" );
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

    while ( 1) {
        int client_socket = accept(server_sock, NULL, NULL);
        // send client welcome message
        send(client_socket, server_message, BUF_SIZE, 0);

        // wait for client to reply to message
        char client_message[BUF_SIZE];
        int read_size = 0;
        while(1) {

            send(client_socket, ">", 2, 0);
            int num = recv_string(client_message, client_socket);
            if ( num == -1) {
                break;
            }
            printf("Got Message: %d \"%s\"\n\n" , num, client_message);
            client_message[num-1] = '\0';

            if ( strcmp("Bye", client_message) == 0 ) {
                // close the connection
                printf("Closing the connection\n");
                close(client_socket);
            } else if ( strcmp(client_message, "SendInventory") == 0 ) {
                /* printf("Printing inventory\n"); */
                memset(server_message, 0, BUF_SIZE);
                print_inventory(server_message);
                send(client_socket, server_message, BUF_SIZE, 0);
            } else if ( strcmp(client_message, "Buy") == 0 ) {
                printf("Purchase\n");
                memset(server_message, 0, BUF_SIZE);
                strcpy(server_message, "Enter FruitName and count\n");
                send(client_socket, server_message, BUF_SIZE, 0);
                // recv another line with FruitName + count
                int num = recv_string(client_message, client_socket);
                if ( num == -1 ) {
                    strcpy(server_message, "Invalid input\n");
                } else {
                    Fruit* f = parse_fruit_string(client_message);
                    if ( f != NULL) {
                        // purchase the fruit + return with message
                        int r =purchase_fruit(f->name, f->count);
                        if ( r == 1) {
                            sprintf(server_message, "%s -> %d purchased successfully\n", f->name, f->count);
                        } else {
                            sprintf(server_message, "Purchase failed (Invalid fruit or count)\n");
                        }
                    } else {
                        strcpy(server_message, "Invalid input\n");
                    }
                    free(f);
                }
                send(client_socket, server_message, BUF_SIZE, 0);
            } else {
                printf("Invalid request\n");
            }

        }
    }

    close(server_sock);
    return 0;
}
