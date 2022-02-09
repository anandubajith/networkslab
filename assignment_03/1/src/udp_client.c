#include <stdio.h>

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

#define BUF_SIZE 1024
#define PORT 12345

int main (int argc, char *argv[])
{

    int server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        printf("setsockopt(SO_REUSEADDR) failed");

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    socklen_t size = sizeof(server_address);

    char* buffer = malloc(sizeof(char) * BUF_SIZE);


    int buy_mode = 0;
    while(1) {
        printf(">");
        memset(buffer, 0, BUF_SIZE);
        scanf("%[^\n]", buffer);
        char t; scanf("%c", &t);

        if ( strlen(buffer) == 0){
            continue;
        }

        buffer[BUF_SIZE-1] = '\0';

        if ( strcmp("Bye", buffer) == 0) {
            printf("Connection closed\n");
            return 0;
        }
        if (buy_mode == 1) {
            /* printf("Reformattin buy message\n"); */
            char* copy = malloc(sizeof(char) * BUF_SIZE);
            sprintf(copy, "Buy[%s]", buffer);
            /* printf("message = %s\n", copy); */
            strcpy(buffer, copy);
            free(copy);
            buy_mode = 0;
        }
        // send it to server
        sendto(server_sock,buffer, strlen(buffer), 0, (struct sockaddr *)&server_address, size );
        if ( strcmp("Buy", buffer) == 0 ) {
            buy_mode = 1;
        }

        memset(buffer, 0, BUF_SIZE);
        recvfrom(server_sock, buffer, BUF_SIZE-1 , 0, (struct sockaddr *)&server_address, &size);

        printf("%s", buffer);


    }

    return 0;
}
