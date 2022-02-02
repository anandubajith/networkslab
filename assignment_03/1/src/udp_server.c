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
        int num = 0;
        socklen_t size = sizeof(their_addr);
        recvfrom(server_sock, buffer, BUF_SIZE-1 , 0, (struct sockaddr *)&their_addr, &size);
        printf("Got \"%s\"", buffer);

        // parse this command
        /*
         * Let the command be like
         * Hello         -> Welcome to server message
         * SendInventory -> Send the inventory object
         * Buy           -> ????
         * Buy[Apple][4] -> Buy apple 4 quantity
         *
         */

        memset(buffer, 0, BUF_SIZE);
        strcpy(buffer, "Hi there");
        sendto(server_sock,buffer, strlen(buffer), 0, (struct sockaddr *)&their_addr, size );
        memset(buffer, 0, BUF_SIZE);
    }



    close(server_sock);
}
