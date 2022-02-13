#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#include "common.h"

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

    recv_file("./output", sock);

    return 0;
}
