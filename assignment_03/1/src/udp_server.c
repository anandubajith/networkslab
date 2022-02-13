#include <stdio.h>

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

#include "fruits.h"

#define BUF_SIZE 1024
#define PORT 12345

int main(){

    setup_fruits();

	struct sockaddr_storage their_addr;
    int server_sock = socket(AF_INET, SOCK_DGRAM, 0);
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

    char* buffer = malloc(sizeof(char) * BUF_SIZE);
    while(1) {
        socklen_t size = sizeof(their_addr);
        memset(buffer, 0, BUF_SIZE);
        recvfrom(server_sock, buffer, BUF_SIZE-1 , 0, (struct sockaddr *)&their_addr, &size);

        // trim newline [may not be needed]
        int len = strlen(buffer);
        if ( buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        printf("Received Message: %d \"%s\"\n\n" , len, buffer);

        if ( strcmp("Hello", buffer) == 0) {
            memset(buffer, 0, BUF_SIZE);
            strcpy(buffer, "Welcome to store\n" );
        } else if (strcmp("SendInventory", buffer) == 0) {
            printf("Sending Inventory\n");
            memset(buffer, 0, BUF_SIZE);
            print_inventory(buffer);
        } else if ( strncmp("Buy", buffer, 3) == 0 ) {
            int length = strlen(buffer);
            if ( length == 3) {
                printf("Buy case1:\n");
                memset(buffer, 0, BUF_SIZE);
                strcpy(buffer, "Enter FruitName and count\n");
            } else {
                printf("Buy case2:\n");
                buffer[length -1] = '\0'; // remove the ]
                Fruit* f = parse_fruit_string(buffer + 4);
                memset(buffer, 0, BUF_SIZE);
                if ( f != NULL) {
                    // purchase the fruit + return with message
                    int r =purchase_fruit(f->name, f->count);
                    if ( r == 1) {
                        sprintf(buffer, "%s -> %d purchased successfully\n", f->name, f->count);
                    } else {
                        sprintf(buffer, "Purchase failed (Invalid fruit or count)\n");
                    }
                } else {
                    strcpy(buffer, "Invalid input\n");
                }
                free(f);
            }

        } else {
            printf("Invalid message\n");
            memset(buffer, 0, BUF_SIZE);
            strcpy(buffer, "Invalid message\n" );
        }

        sendto(server_sock,buffer, strlen(buffer), 0, (struct sockaddr *)&their_addr, size );
    }

    close(server_sock);
}
