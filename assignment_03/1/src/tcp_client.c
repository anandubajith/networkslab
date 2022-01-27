#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "fruits.h"

int main (int argc, char *argv[])
{
    int socket_fd;
    struct addrinfo hints, *servinfo, *p;
    int ret_val;


    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((ret_val = getaddrinfo("localhost", "4444", &hints, &servinfo)) != 0) {
        printf("ERROR:");
        return 1;
    }

    for ( p = servinfo; p != NULL; p = p->ai_next )
    {
        if ( (socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1 )
        {
            perror("Client: socket");
            continue;
        }
        if ( connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(socket_fd);
            perror("client: connect");
            continue;
        }
        break;
    }

    if ( p == NULL ) {
        printf("ERROR: failed to connect\n");
    }
    freeaddrinfo(servinfo);

    // get data from server info buffer



    close(socket_fd);
    return 0;
}
