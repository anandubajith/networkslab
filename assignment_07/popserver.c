/*
 * THIS IS THE POPSERVER to read email
 */

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
    if ( argc < 2 ) {
        printf("./popserver PORT\n");
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[1]));
    server_address.sin_addr.s_addr = INADDR_ANY;

    printf("Pop serer");


    return 0;
}
