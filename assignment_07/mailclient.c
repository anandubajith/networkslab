/*
 * THIS IS THE MAIL CLIENT
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
        printf("./mailclient PORT\n");
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[1]));
    server_address.sin_addr.s_addr = INADDR_ANY;

    /* int r = connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)); */
    /* if (r == -1) { */
    /*     printf("Error: Failed to connect()"); */
    /*     return 0; */
    /* } */

    // ask for username
    printf("username: ");
    fflush(stdout);
    printf("\n");
    // ask for password
    printf("password: ");
    fflush(stdout);
    printf("\n");

    printf("Welcome %s\n", "asdf");
    printf("1. Send Mail\n2. Quit\n");


    return 0;
}
