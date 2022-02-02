#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#include "fruits.h"

#define BUF_SIZE 1024
#define PORT 12356



int main (int argc, char *argv[])
{

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int r = connect(sock, (struct sockaddr *) &server_address, sizeof(server_address));
    if ( r == -1) {
        printf("Error: " );
        exit(1);
    }

    char* buffer = malloc(sizeof(char) * BUF_SIZE);

    while (1) {
        /* printf("going to recv_string"); */
        memset(buffer, 0, BUF_SIZE);
        int num = 0;
        //recv_string(buffer, sock);
        read(sock, buffer, BUF_SIZE);
        printf("%s", buffer);
        printf("\n>");
        memset(buffer, 0, BUF_SIZE);
        scanf("%[^\n]", buffer);
        printf("Scanned \"%s\"\n", buffer);
        buffer[BUF_SIZE-1] = '\0';
        // send it to server
        send(sock, buffer, BUF_SIZE, 0);
    }
    close(sock);
    return 0;
}
